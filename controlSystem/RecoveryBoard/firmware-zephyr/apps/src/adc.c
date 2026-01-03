/*
 * Copyright (c) 2020 Libre Solar Technologies GmbH
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/shell/shell.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ers_adc, CONFIG_ADC_LOG_LEVEL);

#include "keeper.h"

//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define ADC_THREAD_STACK_SIZE 512
#define ADC_THREAD_PRIORITY 3
#define ADC_READ_PERIOD_MS 10

// #define DEV_ERS_ADC_PERIODIC_REPORTING
#undef DEV_ERS_ADC_PERIODIC_REPORTING

// TODO [ ] review whether timeout needed and whether there was an issue for
//   which timeout not implemented here, and if so document that reason:
// #define ERS_ADC_READ_TIMEOUT 1500
// static struct k_timeout_t ers_adc_read_timeout K_MSEC(ERS_ADC_READ_TIMEOUT);

#define MOTOR_ISENSE_READ_PERIOD_MS 50

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
    !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
    ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
                 DT_SPEC_AND_COMMA)
};

// TODO [ ] Compare thread configuration code in can.c with what is here:
struct k_thread adc_thread_data;

K_THREAD_STACK_DEFINE(adc_thread_stack, ADC_THREAD_STACK_SIZE);

struct k_mutex adc_mtx;

// static uint32_t hall_adc_count_1_fs = 0;
// static uint32_t hall_adc_count_2_fs = 0;

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

// TODO [ ] Amend the "read ADC all" command to accept a range of channels,
//   to support the reading of one channel with the same routine.

// TODO [x] Implement logic to update both Hall sensor values when "read ADC
//   channels" API is called to read all channels.  (This to assue that Hall
//   sensor readings are from the same cycle of readings, and not the past
//   two cycles.)

// TODO [ ] Check that following API to read ADC channel range has complete
//   comment block in ers-adc.h header file.

int32_t adc_read_channels(const enum ers_adc_values idx_begin,
			  const enum ers_adc_values idx_end)
{
        int32_t rc = k_mutex_lock(&adc_mtx, K_MSEC(1500));
	if (rc != 0)
	{
		LOG_ERR("Failed to lock ADC read channels mutex, error %d", rc);
		return rc;
	}

	if ((idx_begin < 0) || (idx_end > ARRAY_SIZE(adc_channels)))
	{
		LOG_ERR("Asked to read ADC channel out of range!");
		LOG_ERR("One of begin %d and end %d out of range %d..%d",
			idx_begin, idx_end, 0, ARRAY_SIZE(adc_channels));
		return -EINVAL;
	}

        uint16_t buf;
        struct adc_sequence sequence = { 
                .buffer = &buf,
                /* buffer size in bytes, not number of samples */
                .buffer_size = sizeof(buf),
        };

        for (size_t i = idx_begin; i <= idx_end; i++)
        {
                int32_t val_mv;

// TODO [ ] See about replacing this custom symbol with use of Zephyr module
//          level logging.  See ERS Zephyr firmware files 'Kconfig' and
//          'ers-log-levels.conf' for some details.
#ifdef DEV_ERS_ADC_PERIODIC_REPORTING
                LOG_INF("- %s, channel %d: ",
                             adc_channels[i].dev->name,
                             adc_channels[i].channel_id);
#endif

                (void)adc_sequence_init_dt(&adc_channels[i], &sequence);

                rc = adc_read_dt(&adc_channels[i], &sequence);
                if (rc < 0)
                {
                        LOG_ERR("Could not read ADC channel, error (%d)", rc);
                        continue;
                }

		// Store ADC reading in ERS app "keeper" module:
		ekset_adc_value(i, (uint32_t)buf);

                /*
                 * If using differential mode, the 16 bit value
                 * in the ADC sample buffer should be a signed 2's
                 * complement value.
                 */
                if (adc_channels[i].channel_cfg.differential)
                {
                        val_mv = (int32_t)((int16_t)buf);
                }
                else
                {
                        val_mv = (int32_t)buf;
                }

#ifdef DEV_ERS_ADC_PERIODIC_REPORTING
                LOG_INF("%"PRId32, val_mv);
#endif
                rc = adc_raw_to_millivolts_dt(&adc_channels[i], &val_mv);
                /* conversion to mV may not be supported, skip if not */
                if (rc < 0)
                {
                        LOG_WRN(" (value in mV not available)");
                }
                else
                {
			// Store ADC reading in ERS app "keeper" module:
			ekset_adc_value_in_mv(i, (uint32_t)buf);
#ifdef DEV_ERS_ADC_PERIODIC_REPORTING
                        LOG_INF(" = %"PRId32" mV", val_mv);
#endif
		}
	}

        rc = k_mutex_unlock(&adc_mtx);
	if (rc != 0)
	{
		LOG_ERR("Failed to unlock ADC read channels mutex, error %d", rc);
		return rc;
	}

	return rc;
}

int32_t cmd_ers_read_adc_in0(const struct shell *shell)
{
	uint32_t adc_reading;
	int32_t rc;

	rc = adc_read_channels(ADC_READING_HALL_1, ADC_READING_HALL_1);
	if (rc == 0)
	{
		ekget_hall_1_mv(&adc_reading);
		shell_fprintf(shell, SHELL_NORMAL, "Hall 1 sensor reads %u mV\n", adc_reading);
	}
	else
	{
		shell_fprintf(shell, SHELL_NORMAL, "failed to read Hall 1 sensor, err %d\n", rc);
	}

	return rc;
}

int32_t cmd_ers_read_adc_in1(const struct shell *shell)
{
	uint32_t adc_reading;
	int32_t rc;

	rc = adc_read_channels(ADC_READING_HALL_2, ADC_READING_HALL_2);
	if (rc == 0)
	{
		ekget_hall_2_mv(&adc_reading);
		shell_fprintf(shell, SHELL_NORMAL, "Hall 2 sensor reads %u mV\n", adc_reading);
	}
	else
	{
		shell_fprintf(shell, SHELL_NORMAL, "failed to read Hall 2 sensor, err %d\n", rc);
	}

	return rc;
}

//----------------------------------------------------------------------
// - SECTION - scheduling set up
//----------------------------------------------------------------------

void adc_thread_entry(void *arg1, void *arg2, void *arg3)
{
        ARG_UNUSED(arg1);
        ARG_UNUSED(arg2);
        ARG_UNUSED(arg3);

#ifdef DEV_ERS_ADC_PERIODIC_REPORTING
	static uint32_t count = 0;
#endif
        int32_t rc = 0;

        /* Configure channels individually prior to sampling. */
        for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++)
        {
                if (!adc_is_ready_dt(&adc_channels[i])) {
                        LOG_ERR("ADC controller device %s not ready\n", adc_channels[i].dev->name);
                        rc = -ENODEV;
                }

                rc = adc_channel_setup_dt(&adc_channels[i]);
                if (rc < 0) {
                        LOG_ERR("Could not setup channel #%d (%d)\n", i, rc);
                        rc = -EINVAL;
                }
        }

        while (1)
        {
#ifdef DEV_ERS_ADC_PERIODIC_REPORTING
		if ((count % 300) == 0)
		{
                	LOG_INF("ADC reading[%u]: (thread entry function)\n", count++);
		}
#endif
		rc = adc_read_channels(ADC_READING_HALL_1, ADC_READING_MOTOR_ISENSE); // adc_thread_entry()
                k_sleep(K_MSEC(ADC_READ_PERIOD_MS));
        }
}

int32_t adc_init(void)
{
	int32_t rc = 0;

	k_mutex_init(&adc_mtx);

        /* Configure channels individually prior to sampling. */
        for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
                if (!adc_is_ready_dt(&adc_channels[i])) {
                        LOG_ERR("ADC controller device %s not ready",
                                     adc_channels[i].dev->name);
                        rc = -ENODEV;
                }

                rc = adc_channel_setup_dt(&adc_channels[i]);
                if (rc < 0) {
                        LOG_ERR("Could not setup channel #%d (%d)", i, rc);
                        rc = -EINVAL;
                }
        }

	k_tid_t adc_tid = k_thread_create(&adc_thread_data, adc_thread_stack,
					  K_THREAD_STACK_SIZEOF(adc_thread_stack),
					  adc_thread_entry, NULL, NULL, NULL,
					  ADC_THREAD_PRIORITY, 0, K_NO_WAIT);
	if (!adc_tid)
	{
		LOG_ERR("ERROR spawning ADC thread\n");
	}
	else
	{
		LOG_INF("starting ADC thread . . .");
	}

	LOG_INF("- DEV 1026 - ADC module configured %u channels.", ARRAY_SIZE(adc_channels));
	return rc;
}
