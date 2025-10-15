/*
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * @note Parts of this code copied from Zephyr 3.7.1 DAC sample app.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/dac.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ers_dac, LOG_LEVEL_INF);

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

#if (DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac) && \
	DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_channel_id) && \
	DT_NODE_HAS_PROP(ZEPHYR_USER_NODE, dac_resolution))
#define DAC_NODE DT_PHANDLE(ZEPHYR_USER_NODE, dac)
#define DAC_CHANNEL_ID DT_PROP(ZEPHYR_USER_NODE, dac_channel_id)
#define DAC_RESOLUTION DT_PROP(ZEPHYR_USER_NODE, dac_resolution)
#else
#error "Unsupported board: see README and check /zephyr,user node"
#define DAC_NODE DT_INVALID_NODE
#define DAC_CHANNEL_ID 0
#define DAC_RESOLUTION 0
#endif

static const struct device *const dac_dev = DEVICE_DT_GET(DAC_NODE);

static const struct dac_channel_cfg dac_ch_cfg = { 
	.channel_id  = DAC_CHANNEL_ID,
	.resolution  = DAC_RESOLUTION,
	.buffered = true
};

static uint32_t dac_initialized_fs = 0;

// TODO [ ] Implement a mutex for "set DAC" API.  It has a system sleep call
//   which while brief, is recommended by Zephyr's DAC sample.  While ERS
//   app is unlikely to have more than one calling point to the DAC set output
//   API, a mutex would assure that the delay after setting is honored.

int32_t ers_set_dac_output(const uint32_t value)
{
// Following two const variables from Zephyr 3.7.1 DAC sample app:
	const int32_t dac_values = 1U << DAC_RESOLUTION;
	const int32_t sleep_time = 4096 / dac_values > 0 ? 4096 / dac_values : 1;
	int32_t rc = 0;

	if (dac_initialized_fs < 1)
	{
		LOG_ERR("DAC device not yet initialized, ers_init_dac() called?");
		return -ENODEV;
	}

#define DAC_COUNT_HIGHEST_VAL ((1 << DAC_RESOLUTION) - 1)
	if (value > DAC_COUNT_HIGHEST_VAL)
	{
		LOG_ERR("DAC value %u to write too large, 0..%u possible",
			value, DAC_COUNT_HIGHEST_VAL);
		return -EINVAL;
	}

	rc = dac_write_value(dac_dev, DAC_CHANNEL_ID, value);
	k_sleep(K_MSEC(sleep_time));
	return rc;
}

int32_t ers_init_dac(void)
{
	int32_t rc = 0;

	if (!device_is_ready(dac_dev)) {
		LOG_ERR("DAC device %s is not ready", dac_dev->name);
		return -ENODEV;
	}

	rc = dac_channel_setup(dac_dev, &dac_ch_cfg);

	if (rc != 0) {
		LOG_ERR("Setting up of DAC channel failed with code %d", rc);
		return -EIO;
	}

	// Simple way for this tiny module to indicate it is initialized:
	dac_initialized_fs = 1;

	LOG_INF("Ready to produce signal on DAC channel %d.",
		DAC_CHANNEL_ID);

	LOG_INF("Per device tree DAC has resolution of %d counts", DAC_RESOLUTION);

	return rc;
}
