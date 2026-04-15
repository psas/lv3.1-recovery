/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @note CAN message filters are presently defined in routine rx_thread_entry().
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
// LOG_MODULE_REGISTER(ers_can, CONFIG_CAN_LOG_LEVEL);
LOG_MODULE_REGISTER(ers_can, LOG_LEVEL_ERR);

#include <ers-app-config.h>
#include <keeper.h>
#include <motor-control.h>

K_THREAD_STACK_DEFINE(rx_thread_stack, CONFIG_CAN_RX_THREAD_STACK_SIZE);
struct k_thread rx_thread_data;

#define SLEEP_TIME K_MSEC(250)

/**
 * @note CAN frame ids for ERS:  while ERS sender won't listen for
 *   ERS drogue board CAN heartbeat nor ERS main board heartbeat, we
 *   include these message ids with the plan that one firmware project
 *   will include a build time flag to select the ids needed by each of
 *   three ERS firmwares, which are much more alike than different.
 */

// clang-format off
#define MSG_ID_TELEMETRUM_SENDER   0x700
#define MSG_ID_DROGUE_HEARTBEAT    0x710
#define MSG_ID_MAIN_HEARTBEAT      0x720
#define MSG_ID_UNLOCK_DROGUE_CHUTE 0x100
#define MSG_ID_UNLOCK_MAIN_CHUTE   0x200

#define MSG_ID_ACKNOLEDGE_DROGUE_UNLOCK 0x101
#define MSG_ID_ACKNOLEDGE_MAIN_UNLOCK   0x201
// clang-format on

#if defined(ERS_BOARD_VARIANT_DROGUE_CHUTE)
#define MSG_ID_STATUS_AND_HEARTBEAT MSG_ID_DROGUE_HEARTBEAT
#elif defined(ERS_BOARD_VARIANT_MAIN_CHUTE)
#define MSG_ID_STATUS_AND_HEARTBEAT MSG_ID_MAIN_HEARTBEAT
#else
#error "Need one of board variant 'drogue' or 'main' chute specified for build!"
#endif

#define HEARTBEAT_PERIOD_S 1
// TODO [ ] Choose a more clear name for CANBus health check period in seconds:
#define CAN_BUS_CHECK_PER_S 2

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

// TODO [x] Review whether this message queues needed:
CAN_MSGQ_DEFINE(counter_msgq, 2);

#if defined(ERS_BOARD_VARIANT_SENDER)
enum ers_state_var_indeces {
	IDX_SENDER_TELEMETRUM_STATE,
	IDX_SENDER_BATT_READ,
	IDX_SENDER_BATT_OK,
	IDX_SENDER_SHORE_POW_STATUS,
	IDX_SENDER_CAN_BUS_OK,
	IDX_SENDER_ERS_STATUS,
	IDX_SENDER_ROCKET_READY,
	IDX_RESERVED_01,
	IDX_STATE_VAR_LAST_ELEMENT
};
#elif defined(ERS_BOARD_VARIANT_DROGUE_CHUTE) || defined(ERS_BOARD_VARIANT_MAIN_CHUTE)
#if defined(ERS_BOARD_VARIANT_DROGUE_CHUTE)
#warning "- NOTICE - builing ERS board firmware variant 'Drogue'."
#elif defined(ERS_BOARD_VARIANT_MAIN_CHUTE)
#warning "- NOTICE - builing ERS board firmware variant 'Main'."
#endif
enum ers_state_var_indeces {
	IDX_DROGUE_RING_STATE,
	IDX_DROGUE_BATT_READ,
	IDX_DROGUE_BATT_OK,
	IDX_DROGUE_SHORE_POW_STATUS,
	IDX_DROGUE_CAN_BUS_OK,
	IDX_DROGUE_READY,
	IDX_RESERVED_01,
	IDX_RESERVED_02,
	IDX_STATE_VAR_LAST_ELEMENT
};
#else
#warning "ERROR no ERS board firmware variant defined."
// #warning "Need one of ERS_BOARD_VARIANT_SENDER,"
#warning "ERS_BOARD_VARIANT_DROGUE_CHUTE or ERS_BOARD_VARIANT_MAIN_CHUTE."
#endif

static uint8_t ers_state_vars_fs[IDX_STATE_VAR_LAST_ELEMENT] = {0};
static uint8_t ers_small_payload_fs[1] = {0};

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

void clear_flag_can_ok_work_handler(struct k_work *work)
{
    // LOG_INF("CAN module timer expired!");
    ekset_can_bus_ok(0);                    // in "clear CANBus OK flag" work handler
}

K_WORK_DEFINE(clear_flag_can_ok_work, clear_flag_can_ok_work_handler);

void telemetrum_check_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&clear_flag_can_ok_work);
}

K_TIMER_DEFINE(telemetrum_check_timer, telemetrum_check_timer_handler, NULL);

/**
 * @brief IRQ callback to provide to can_send() API.
 *
 * @note Different calls to can_send() may likely need distinct callbacks.
 *
 * @note This detail learned from Zephyr sample app, by Alexander Wachter
 *   from Zephyr 3.7.1, copyright 2018.
 */

void tx_irq_callback(const struct device *dev, int error, void *arg)
{
        char *sender = (char *)arg;

        ARG_UNUSED(dev);

        if (error != 0) {
                LOG_ERR("Callback! error-code: %d   Sender: %s",
                       error, sender);
        }
}

void prep_and_send_ack_unlock_command(void)
{
        struct can_frame ers_acknowledge_frame = {
                .flags = 0,
#if defined(ERS_BOARD_VARIANT_DROGUE_CHUTE)
                .id = MSG_ID_ACKNOLEDGE_DROGUE_UNLOCK,
#elif defined(ERS_BOARD_VARIANT_MAIN_CHUTE)
                .id = MSG_ID_ACKNOLEDGE_MAIN_UNLOCK,
#else
#error "Need one of board variant 'drogue' or 'main' chute specified for build!"
#endif
                .dlc = sizeof(ers_small_payload_fs)
        };

	can_send(can_dev, &ers_acknowledge_frame, K_FOREVER,
		 tx_irq_callback,
		 "ERS acklowledge frame");
}

void prep_and_send_status_frame_work_handler(struct k_work *work)
{
        struct can_frame ers_status_frame = {
                .flags = 0,
                .id = MSG_ID_STATUS_AND_HEARTBEAT,
                .dlc = sizeof(ers_state_vars_fs)
        };

	// (1)
	enum lock_ring_state ring_state = RING_STATE_UNKNOWN;
	ekget_ring_status(&ring_state);

	// (2)
	uint32_t battery_voltage = 0;
	ekget_batt_read_dv(&battery_voltage);

	// (3)
	uint32_t batt_ok_flag = 0;
	ekget_batt_ok(&batt_ok_flag);

	// (4 . . . drogue chute board detected power status)
	uint32_t not_umb_on = 0;
	ekget_not_umb_on(&not_umb_on);

	// (5)
	uint32_t can_bus_ok_flag = 0;            // define local var in "prep and send status frame"
	ekget_can_bus_ok(&can_bus_ok_flag);

	// ring status: 0 = uninitialized, 1 = unlocked, 2 = in between, 3 = locked, 4 = error
	ers_state_vars_fs[IDX_DROGUE_RING_STATE] = (uint8_t)(ring_state);
	ers_state_vars_fs[IDX_DROGUE_BATT_READ] = (uint8_t)(battery_voltage & 0xFF);
	ers_state_vars_fs[IDX_DROGUE_BATT_OK] = batt_ok_flag;
	// TODO [ ] mask not_umb_on with 0x1 to assure Boolean value:
	ers_state_vars_fs[IDX_DROGUE_SHORE_POW_STATUS] = (uint8_t)(not_umb_on);
	ers_state_vars_fs[IDX_DROGUE_CAN_BUS_OK] = (uint8_t)(can_bus_ok_flag & 0xFF);
	ers_state_vars_fs[IDX_DROGUE_READY] = 0;
	ers_state_vars_fs[IDX_RESERVED_01] = 0;
	ers_state_vars_fs[IDX_RESERVED_02] = 0;

	ers_state_vars_fs[IDX_DROGUE_READY] = 
	  (ers_state_vars_fs[IDX_DROGUE_RING_STATE] == 3) &&
	   ers_state_vars_fs[IDX_DROGUE_BATT_OK] &&
	   ers_state_vars_fs[IDX_DROGUE_CAN_BUS_OK];

	memcpy(ers_status_frame.data, ers_state_vars_fs, sizeof(ers_state_vars_fs));

	can_send(can_dev, &ers_status_frame, K_FOREVER,
		 tx_irq_callback,
		 "ERS status frame");

//----------------------------------------------------------------------
// the message The Message THE MESSAGE
//----------------------------------------------------------------------

#if 0
        LOG_INF("                  ringst battrd battok pwrsts canok  ready  reserv reserv");
	LOG_INF("drogue CAN frame:  0x%02X   0x%02X   0x%02X   0x%02X   0x%02X   0x%02X   0x%02X"
	"   0x%02X",
	  ers_state_vars_fs[IDX_DROGUE_RING_STATE],
	  ers_state_vars_fs[IDX_DROGUE_BATT_READ],
	  ers_state_vars_fs[IDX_DROGUE_BATT_OK],
	  ers_state_vars_fs[IDX_DROGUE_SHORE_POW_STATUS],
	  ers_state_vars_fs[IDX_DROGUE_CAN_BUS_OK],
	  ers_state_vars_fs[IDX_DROGUE_READY],
	  ers_state_vars_fs[IDX_RESERVED_01],
	  ers_state_vars_fs[IDX_RESERVED_02]);
#endif // 0 . . . 2026-02-15
}

K_WORK_DEFINE(prep_and_send_status_frame_work, prep_and_send_status_frame_work_handler);

void heartbeat_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&prep_and_send_status_frame_work);
}

K_TIMER_DEFINE(heartbeat_timer, heartbeat_timer_handler, NULL);

void rx_thread_entry(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);
	int32_t rc = 0;

	const struct can_filter filter_sender_heartbeat = {
		// .flags = CAN_FILTER_IDE,
		.flags = 0,
		.id = MSG_ID_TELEMETRUM_SENDER,
		// .mask = CAN_EXT_ID_MASK
		.mask = CAN_STD_ID_MASK
	};

	const struct can_filter filter_drogue_heartbeat = {
		.flags = 0,
		.id = MSG_ID_DROGUE_HEARTBEAT,
		.mask = CAN_STD_ID_MASK
	};

	const struct can_filter filter_main_heartbeat = {
		.flags = 0,
		.id = MSG_ID_MAIN_HEARTBEAT,
		.mask = CAN_STD_ID_MASK
	};

	const struct can_filter filter_unlock_drogue_chute = {
		.flags = 0,
		.id = MSG_ID_UNLOCK_DROGUE_CHUTE,
		.mask = CAN_STD_ID_MASK
	};

	const struct can_filter filter_unlock_main_chute = {
		.flags = 0,
		.id = MSG_ID_UNLOCK_MAIN_CHUTE,
		.mask = CAN_STD_ID_MASK
	};

	struct can_frame frame;
	int filter_id;

	filter_id = can_add_rx_filter_msgq(can_dev, &counter_msgq, &filter_sender_heartbeat);
	filter_id = can_add_rx_filter_msgq(can_dev, &counter_msgq, &filter_drogue_heartbeat);
	filter_id = can_add_rx_filter_msgq(can_dev, &counter_msgq, &filter_main_heartbeat);
// TODO [ ] consider adding build time symbol to select CAN filter additions
//          based on given ERS firmware variant 'sender', 'drogue', and 'main'.
	filter_id = can_add_rx_filter_msgq(can_dev, &counter_msgq, &filter_unlock_drogue_chute);
	filter_id = can_add_rx_filter_msgq(can_dev, &counter_msgq, &filter_unlock_main_chute);

	while (1) {
		k_msgq_get(&counter_msgq, &frame, K_FOREVER);

		if (IS_ENABLED(CONFIG_CAN_ACCEPT_RTR) && (frame.flags & CAN_FRAME_RTR) != 0U) {
			// LOG_INF("M2");
			continue;
		}

		if (frame.id == MSG_ID_TELEMETRUM_SENDER) {
			k_timer_start(&telemetrum_check_timer, K_SECONDS(2), K_SECONDS(2));
			LOG_INF("RX %X - telemetrum heartbeat", frame.id);
			ekset_can_bus_ok(1);
		}

		switch (frame.id)
		{
		case MSG_ID_TELEMETRUM_SENDER:
			// LOG_INF("RX %X - telemetrum heartbeat", frame.id);
			break;
		case MSG_ID_DROGUE_HEARTBEAT:
			// LOG_INF("RX %X - drogue chute heartbeat", frame.id);
			break;
		case MSG_ID_MAIN_HEARTBEAT:
			// LOG_INF("RX %X - main chute heartbeat", frame.id);
			break;
#if defined(ERS_BOARD_VARIANT_DROGUE_CHUTE)
		case MSG_ID_UNLOCK_DROGUE_CHUTE:
			LOG_INF("RX %X - got cmd unlock drogue chute", frame.id);
#elif defined(ERS_BOARD_VARIANT_MAIN_CHUTE)
		case MSG_ID_UNLOCK_MAIN_CHUTE:
			LOG_INF("RX %X - got cmd unlock main chute", frame.id);
#else
#error "Need one of board variant 'drogue' or 'main' chute specified for build!"
#endif

// TODO [ ] Add needed test before calling unlock API.  Test per ERS Google doc is:
// "If !UMB_ON = 1 (no umbilical voltage) and the the RING_STATUS = 2 (it’s locked)"
			rc = mc_unlock_ring();
			if (rc != 0)
			{
				LOG_ERR("Failed to unlock ring via CAN message, err %d", rc);
			}

			prep_and_send_ack_unlock_command();
			break;
#if defined(ERS_BOARD_VARIANT_MAIN_CHUTE)
//		case MSG_ID_UNLOCK_MAIN_CHUTE:
//			LOG_INF("RX %X - unlock main chute", frame.id);
//			break;
#endif
		default:
			LOG_WRN("RX %X <- unrecognized CAN frame id", frame.id);
		}
	}
}

char *state_to_str(enum can_state state)
{
	switch (state) {
	case CAN_STATE_ERROR_ACTIVE:
		return "error-active";
	case CAN_STATE_ERROR_WARNING:
		return "error-warning";
	case CAN_STATE_ERROR_PASSIVE:
		return "error-passive";
	case CAN_STATE_BUS_OFF:
		return "bus-off";
	case CAN_STATE_STOPPED:
		return "stopped";
	default:
		return "unknown";
	}
}

int32_t ers_init_can(void)
{
	int32_t rc = 0;
	k_tid_t rx_tid;

	if (!device_is_ready(can_dev)) {
		LOG_ERR("CAN: Device %s not ready.", can_dev->name);
		return -ENODEV;
	}

// From "./include/zephyr/drivers/can.h":
// int can_set_bitrate_data(const struct device *dev, uint32_t bitrate_data);

	rc = can_set_bitrate_data(can_dev, 500000);
	if (rc != 0)
	{
		LOG_ERR("Failed to set CAN bitrate, err %d", rc);
	}

	rc = can_start(can_dev);
 	if (rc != 0) {
		LOG_ERR("Error starting CAN controller [%d]", rc);
		return -EAGAIN;
	}

	k_timer_start(&telemetrum_check_timer, K_SECONDS(CAN_BUS_CHECK_PER_S), K_SECONDS(CAN_BUS_CHECK_PER_S));

	k_timer_start(&heartbeat_timer, K_SECONDS(HEARTBEAT_PERIOD_S), K_SECONDS(HEARTBEAT_PERIOD_S));

	rx_tid = k_thread_create(&rx_thread_data, rx_thread_stack,
				 K_THREAD_STACK_SIZEOF(rx_thread_stack),
				 rx_thread_entry, NULL, NULL, NULL,
				 CONFIG_CAN_RX_THREAD_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(rx_tid, "thread-can");

	if (!rx_tid) {
		LOG_ERR("ERROR spawning rx thread");
	}

	return rc;
}
