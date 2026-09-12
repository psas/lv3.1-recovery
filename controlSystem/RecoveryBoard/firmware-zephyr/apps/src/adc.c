/*
 * Copyright (c) 2020 Libre Solar Technologies GmbH
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ers-util.h"
#include "keeper.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/shell/shell.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ers_adc, CONFIG_ERS_ADC_LOG_LEVEL);

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define ADC_READ_PERIOD_MS 10

#undef DEV_ERS_ADC_PERIODIC_REPORTING

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

K_THREAD_STACK_DEFINE(adc_thread_stack, CONFIG_ADC_THREAD_STACK_SIZE);

struct k_thread adc_thread_data;

struct k_mutex adc_mtx;

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

int32_t adc_read_channels(const enum ers_adc_values idx_begin,
			  const enum ers_adc_values idx_end)
{
	int32_t rc = k_mutex_lock(&adc_mtx, K_MSEC(CONFIG_ADC_API_TIMEOUT_MS));
	if (rc != 0) {
		LOG_ERR("Failed to lock ADC read channels mutex, error %d", rc);
		goto done;
	}

	if ((idx_begin < 0) || (idx_end > ARRAY_SIZE(adc_channels)))
	{
		LOG_ERR("Asked to read ADC channel out of range!");
		LOG_ERR("One of begin %d and end %d out of range %d..%d",
			idx_begin, idx_end, 0, ARRAY_SIZE(adc_channels));
		goto unlock;
	}

        uint16_t buf;
        struct adc_sequence sequence = { 
                .buffer = &buf,
                /* buffer size in bytes, not number of samples */
                .buffer_size = sizeof(buf),
        };

        for (size_t i = idx_begin; i <= idx_end; i++) {

#ifdef DEV_ERS_ADC_PERIODIC_REPORTING
                LOG_INF("- %s, channel %d: ",
                             adc_channels[i].dev->name,
                             adc_channels[i].channel_id);
#endif

                (void)adc_sequence_init_dt(&adc_channels[i], &sequence);

                rc = adc_read_dt(&adc_channels[i], &sequence);
                if (rc < 0) {
                        LOG_ERR("Could not read ADC channel, error (%d)", rc);
                        continue;
                }

		// Store ADC reading in ERS app "keeper" module:
		keeper_set_adc_value(i, (uint32_t)buf);
	}

unlock:
#if 0
	rc = k_mutex_unlock(&adc_mtx);
	if (rc != 0) {
		LOG_ERR("Failed to unlock mutex in ADC read channels, err %d", rc);
		return rc;
	}
#endif // 0
	ERS_MUTEX_UNLOCK(adc_mtx, ADC module);
done:
	return rc;
}

int32_t cmd_ers_read_adc_in0(const struct shell *shell)
{
	uint32_t adc_reading;

	int32_t rc = k_mutex_lock(&adc_mtx, K_MSEC(CONFIG_ADC_API_TIMEOUT_MS));
	if (rc != 0) {
		LOG_ERR("Failed to lock mutex in read ADC_IN0, err %d", rc);
		goto done;
	}

	rc = adc_read_channels(ADC_READING_HALL_1, ADC_READING_HALL_1);
	if (rc == 0) {
		keeper_get_hall_1_mv(&adc_reading);
		shell_fprintf(shell, SHELL_NORMAL, "Hall 1 sensor reads %u mV\n", adc_reading);
	} else {
		shell_fprintf(shell, SHELL_NORMAL, "failed to read Hall 1 sensor, err %d\n", rc);
	}

	rc = k_mutex_unlock(&adc_mtx);
	if (rc != 0) {
		LOG_ERR("Failed to unlock mutex in read ADC_IN0, err %d", rc);
		return rc;
	}

done:
	return rc;
}

int32_t cmd_ers_read_adc_in1(const struct shell *shell)
{
	uint32_t adc_reading;

	int32_t rc = k_mutex_lock(&adc_mtx, K_MSEC(CONFIG_ADC_API_TIMEOUT_MS));
	if (rc != 0) {
		LOG_ERR("Failed to lock mutex in read ADC_IN1, err %d", rc);
		goto done;
	}

	rc = adc_read_channels(ADC_READING_HALL_2, ADC_READING_HALL_2);
	if (rc == 0) {
		keeper_get_hall_2_mv(&adc_reading);
		shell_fprintf(shell, SHELL_NORMAL, "Hall 2 sensor reads %u mV\n", adc_reading);
	} else {
		shell_fprintf(shell, SHELL_NORMAL, "failed to read Hall 2 sensor, err %d\n", rc);
	}

	rc = k_mutex_unlock(&adc_mtx);
	if (rc != 0) {
		LOG_ERR("Failed to unlock mutex in read ADC_IN1, err %d", rc);
		return rc;
	}

done:
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
			goto error;
                }

                rc = adc_channel_setup_dt(&adc_channels[i]);
                if (rc < 0) {
                        LOG_ERR("Could not setup channel #%d (%d)\n", i, rc);
                        rc = -EINVAL;
			goto error;
                }
        }

        while (1) {
#ifdef DEV_ERS_ADC_PERIODIC_REPORTING
		if ((count % 300) == 0) {
                	LOG_INF("ADC reading[%u]: (thread entry function)\n", count++);
		}
#endif
		rc = adc_read_channels(ADC_READING_HALL_1, ADC_READING_MOTOR_ISENSE); // adc_thread_entry()
                k_sleep(K_MSEC(ADC_READ_PERIOD_MS));
        }

error:
	return;
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
					  CONFIG_ADC_THREAD_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(adc_tid, "adc_thread");

	if (!adc_tid) {
		LOG_ERR("ERROR spawning ADC thread\n");
	} else {
		LOG_INF("starting ADC thread . . .");
	}

	LOG_INF("ADC module configured %u channels.", ARRAY_SIZE(adc_channels));
	LOG_INF("- DEV 0906 - ADC API timeout set to %d ms", CONFIG_ADC_API_TIMEOUT_MS);

	return rc;
}
