#include "esp_twai.h"
#include "esp_twai_onchip.h"

#define SWITCH_1 1
#define SWITCH_2 2
#define SWITCH_3 3
#define SWITCH_4 4
#define debug_light 38
#define mode 5
#define silence 6
#define shutdown 7
#define reciever 8
#define transmitter 9

twai_node_handle_t node_hdl = NULL;
twai_onchip_node_config_t node_config = {
    .io_cfg.tx = 9,             // TWAI TX GPIO pin
    .io_cfg.rx = 8,             // TWAI RX GPIO pin
    .bit_timing.bitrate = 1000000,  // 200 kbps bitrate
    .tx_queue_depth = 5,        // Transmit queue depth set to 5
};

uint8_t send_buff[8] = {0};

twai_frame_t unlock_drogue = {
    .header.id = 0x100,           // Message ID
    .header.ide = true,         // Use 29-bit extended ID format
    .buffer = send_buff,        // Pointer to data to transmit
    .buffer_len = sizeof(send_buff),  // Length of data to transmit
};

twai_frame_t unlock_main = {
    .header.id = 0x200,           // Message ID
    .header.ide = true,         // Use 29-bit extended ID format
    .buffer = send_buff,        // Pointer to data to transmit
    .buffer_len = sizeof(send_buff),  // Length of data to transmit
};

twai_frame_t lock_drogue = {
    .header.id = 0x1,           // Message ID
    .header.ide = true,         // Use 29-bit extended ID format
    .buffer = send_buff,        // Pointer to data to transmit
    .buffer_len = sizeof(send_buff),  // Length of data to transmit
};

twai_frame_t lock_main = {
    .header.id = 0x1,           // Message ID
    .header.ide = true,         // Use 29-bit extended ID format
    .buffer = send_buff,        // Pointer to data to transmit
    .buffer_len = sizeof(send_buff),  // Length of data to transmit
};


void setup() {
    pinMode(SWITCH_1, INPUT);
    pinMode(SWITCH_2, INPUT);
    pinMode(SWITCH_3, INPUT);
    pinMode(SWITCH_4, INPUT);
    pinMode(debug_light, OUTPUT);
    pinMode(mode, INPUT);
    pinMode(silence, OUTPUT);
    pinMode(shutdown, OUTPUT);
    pinMode(reciever, OUTPUT);
    pinMode(transmitter, INPUT);

    digitalWrite(debug_light, HIGH);
}

void loop() {
    int lock_signal_drogue = digitalRead(SWITCH_1);
    int unlock_signal_drogue = digitalRead(SWITCH_3);
    int lock_signal_main = digitalRead(SWITCH_2);
    int unlock_signal_main = digitalRead(SWITCH_4);

    if (unlock_signal_drogue == 0) {
        ESP_ERROR_CHECK(twai_node_transmit(node_hdl, &unlock_drogue, 0));  // Timeout = 0: returns immediately if queue is full
        ESP_ERROR_CHECK(twai_node_transmit_wait_all_done(node_hdl, -1));  // Wait for transmission to finish
    }

    if (unlock_signal_main == 0) {
        ESP_ERROR_CHECK(twai_node_transmit(node_hdl, &unlock_main, 0));  // Timeout = 0: returns immediately if queue is full
        ESP_ERROR_CHECK(twai_node_transmit_wait_all_done(node_hdl, -1));  // Wait for transmission to finish
    }
}