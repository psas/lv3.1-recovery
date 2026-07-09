/*
 * parachute_can.ino
 *
 * Arduino (ESP32-S3, arduino-esp32 core) side of the parachute CAN interface.
 *
 * The arduino-esp32 core is built on ESP-IDF and exposes the native TWAI
 * driver directly via driver/twai.h, so no extra CAN library is needed.
 *
 * Mirrors the STM32 parachute board's CAN ID scheme:
 *   0x1XX - Drogue parachute commands/acks
 *   0x2XX - Main parachute commands/acks
 *   0x7XX - System status / heartbeat
 *
 * Bitrate: 1 Mbit/s (must match CAN_BITRATE on the STM32 side exactly)
 *
 * Adjust TWAI_TX_PIN / TWAI_RX_PIN to whatever pins your TCAN330 is wired to.
 */

#include "driver/twai.h"

// ---- Pin config: CHANGE THESE to match your board ----
#define TWAI_TX_PIN GPIO_NUM_5
#define TWAI_RX_PIN GPIO_NUM_4

// ---- CAN ID scheme, mirrored from the STM32 firmware's can.rs ----
#define DROGUE_DEPLOY_ID      0x100
#define DROGUE_HEARTBEAT_ID   0x710
#define DROGUE_ACKNOWLEDGE_ID 0x101

#define MAIN_DEPLOY_ID        0x200
#define MAIN_HEARTBEAT_ID     0x720
#define MAIN_ACKNOWLEDGE_ID   0x201

#define SENDER_HEARTBEAT_ID   0x700

#define HEARTBEAT_INTERVAL_MS 500UL  // well under the STM32's 2000ms staleness window
#define ACK_WAIT_MS           200UL

enum chute_t {
    CHUTE_DROGUE,
    CHUTE_MAIN,
};

// Tracks a deploy command that's waiting on an ack.
struct PendingDeploy {
    bool waiting;
    uint32_t ack_id;
    unsigned long sent_at;
    int attempts_left;
};

static PendingDeploy pending = {false, 0, 0, 0};
static unsigned long last_heartbeat_ms = 0;

// ---------------------------------------------------------------------------
// Transmit helpers
// ---------------------------------------------------------------------------

static bool send_frame(uint32_t id, const uint8_t *data, uint8_t len)
{
    twai_message_t msg = {};
    msg.identifier = id;
    msg.data_length_code = len;
    msg.extd = 0;   // standard (11-bit) ID
    msg.rtr = 0;
    if (len > 0 && data != nullptr) {
        memcpy(msg.data, data, len);
    }

    esp_err_t err = twai_transmit(&msg, pdMS_TO_TICKS(100));
    if (err != ESP_OK) {
        Serial.printf("[CAN] transmit failed for id 0x%03lX: %s\n", id, esp_err_to_name(err));
        return false;
    }
    return true;
}

// Payload is ignored by the STM32 firmware, but a single byte (1) is sent
// as a convention, matching the ack frames it sends back.
static bool send_deploy(chute_t chute)
{
    uint32_t id = (chute == CHUTE_DROGUE) ? DROGUE_DEPLOY_ID : MAIN_DEPLOY_ID;
    uint8_t payload[1] = {1};
    Serial.printf("[CAN] sending deploy command 0x%03lX\n", id);
    return send_frame(id, payload, 1);
}

static bool send_sender_heartbeat(void)
{
    return send_frame(SENDER_HEARTBEAT_ID, nullptr, 0);
}

// Call this to kick off a deploy. Non-blocking: arms a pending-ack state
// that gets checked/retried from the main loop instead of blocking here.
//
// NOTE: the STM32 side has no debounce on repeated deploy frames -- each one
// re-drives the motor. Keep retries low and deliberate.
static void start_deploy(chute_t chute)
{
    if (pending.waiting) {
        Serial.println("[CAN] deploy already in progress, ignoring");
        return;
    }
    uint32_t ack_id = (chute == CHUTE_DROGUE) ? DROGUE_ACKNOWLEDGE_ID : MAIN_ACKNOWLEDGE_ID;
    if (send_deploy(chute)) {
        pending.waiting = true;
        pending.ack_id = ack_id;
        pending.sent_at = millis();
        pending.attempts_left = 1;   // one retry allowed
    }
}

// ---------------------------------------------------------------------------
// Receive handling - called every loop() iteration, non-blocking
// ---------------------------------------------------------------------------

static void handle_rx_frame(const twai_message_t &rx)
{
    if (rx.extd) {
        return;  // we only use standard IDs
    }

    switch (rx.identifier) {
        case DROGUE_HEARTBEAT_ID:
            Serial.printf("[CAN] drogue status: ring=%u batt=%u ok=%u shore=%u sender_ok=%u ready=%u\n",
                          rx.data[0], rx.data[1], rx.data[2], rx.data[3], rx.data[4], rx.data[5]);
            break;
        case MAIN_HEARTBEAT_ID:
            Serial.printf("[CAN] main status: ring=%u batt=%u ok=%u shore=%u sender_ok=%u ready=%u\n",
                          rx.data[0], rx.data[1], rx.data[2], rx.data[3], rx.data[4], rx.data[5]);
            break;
        case DROGUE_ACKNOWLEDGE_ID:
        case MAIN_ACKNOWLEDGE_ID:
            if (pending.waiting && rx.identifier == pending.ack_id) {
                Serial.println("[CAN] deploy acknowledged");
                pending.waiting = false;
            }
            break;
        default:
            break;
    }
}

static void poll_rx(void)
{
    twai_message_t rx;
    // 0 ticks: non-blocking poll, keeps loop() responsive
    while (twai_receive(&rx, 0) == ESP_OK) {
        handle_rx_frame(rx);
    }
}

// Checks whether a pending deploy has timed out waiting for its ack, and
// retries once before giving up.
static void service_pending_deploy(void)
{
    if (!pending.waiting) {
        return;
    }
    if (millis() - pending.sent_at < ACK_WAIT_MS) {
        return;
    }

    if (pending.attempts_left > 0) {
        Serial.println("[CAN] no ack yet, retrying deploy");
        chute_t chute = (pending.ack_id == DROGUE_ACKNOWLEDGE_ID) ? CHUTE_DROGUE : CHUTE_MAIN;
        pending.attempts_left--;
        if (send_deploy(chute)) {
            pending.sent_at = millis();
        }
    } else {
        Serial.println("[CAN] deploy failed to confirm after retries");
        pending.waiting = false;
    }
}

// ---------------------------------------------------------------------------
// Arduino entry points
// ---------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    delay(200);

    twai_general_config_t g_config =
        TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX_PIN, TWAI_RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    // Accept everything; filter by ID in software, same approach the STM32 side takes.
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        Serial.println("[CAN] driver install failed");
        return;
    }
    if (twai_start() != ESP_OK) {
        Serial.println("[CAN] start failed");
        return;
    }

    Serial.println("[CAN] TWAI started at 1 Mbit/s");
}

void loop()
{
    // Heartbeat: must run continuously so the parachute boards see us as alive.
    if (millis() - last_heartbeat_ms >= HEARTBEAT_INTERVAL_MS) {
        send_sender_heartbeat();
        last_heartbeat_ms = millis();
    }

    poll_rx();
    service_pending_deploy();

    // Example trigger, replace with your real deploy condition:
    // if (some_condition) { start_deploy(CHUTE_DROGUE); }
}
