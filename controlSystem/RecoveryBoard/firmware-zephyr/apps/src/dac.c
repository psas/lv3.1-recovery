/*
 * @file
 * @brief ERS Zephyr digital-to-analog conversion module.
 * @note Parts of this code copied from Zephyr 3.7.1 DAC sample app.
 */

#include "ers-util.h"
#include "keeper.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/dac.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ers_dac, CONFIG_ERS_DAC_LOG_LEVEL);

//----------------------------------------------------------------------
// - SECTION - defines
//----------------------------------------------------------------------

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

#define DAC_COUNT_HIGHEST_VAL ((1 << DAC_RESOLUTION) - 1)

//----------------------------------------------------------------------
// - SECTION - file scoped
//----------------------------------------------------------------------

static const struct device *const dac_dev = DEVICE_DT_GET(DAC_NODE);

static const struct dac_channel_cfg dac_ch_cfg = { 
	.channel_id  = DAC_CHANNEL_ID,
	.resolution  = DAC_RESOLUTION,
	.buffered = true
};

static atomic_t dac_value_fs = ATOMIC_INIT(0);

K_MUTEX_DEFINE(dac_mtx);

static uint32_t dac_initialized_fs = 0;

//----------------------------------------------------------------------
// - SECTION - routines
//----------------------------------------------------------------------

int32_t dac_write_output_reg(const uint32_t value)
{
// Following two const variables from Zephyr 3.7.1 DAC sample app:
	const int32_t dac_values = 1U << DAC_RESOLUTION;
	// TODO [ ] Determine whether hard-coded 4096 values are DAC resolution,
	//          e.g. 4096 = 2 ^ 12:
	const int32_t sleep_time = 4096 / dac_values > 0 ? 4096 / dac_values : 1;
	int32_t rc = 0;
	ERS_MUTEX_LOCK(dac_mtx, CONFIG_DAC_API_TIMEOUT_MS, dac);

	if (!dac_initialized_fs) {
		LOG_ERR("DAC device not yet initialized, dac_init() called?");
		rc = -ENODEV;
		goto unlock;
	}

	if (value > DAC_COUNT_HIGHEST_VAL) {
		LOG_ERR("DAC value %u to write too large, 0..%u possible",
			value, DAC_COUNT_HIGHEST_VAL);
		rc = -EINVAL;
		goto unlock;
	}

	rc = dac_write_value(dac_dev, DAC_CHANNEL_ID, value);
	if (rc != 0) {
		LOG_ERR("Failed to write output value %d to DAC, err %d", value, rc);
	} else {
		atomic_set(&dac_value_fs, (atomic_val_t)value);
		keeper_set_DAC_val_for_ring_motor(value);
		k_sleep(K_MSEC(sleep_time));
	}

unlock:
	ERS_MUTEX_UNLOCK(dac_mtx, dac);
done:
	return rc;
}

int32_t dac_present_value(uint32_t *dac_setting)
{
	int32_t rc = 0;
	ERS_MUTEX_LOCK(dac_mtx, CONFIG_DAC_API_TIMEOUT_MS, dac);

	if (!dac_initialized_fs) {
		LOG_ERR("DAC module not initialized, call to dac_init() missed?");
		rc = -EINVAL;
		goto done;
	}

	*dac_setting = atomic_get(&dac_value_fs);

	ERS_MUTEX_UNLOCK(dac_mtx, dac);
done:
	return rc;
}

int32_t dac_range(int32_t *bound_low, int32_t *bound_high)
{
	int32_t rc = 0;
	ERS_MUTEX_LOCK(dac_mtx, CONFIG_DAC_API_TIMEOUT_MS, dac);

	if (!dac_initialized_fs) {
		LOG_ERR("DAC module not initialized, call to dac_init() missed?");
		rc = -EINVAL;
		goto done;
	}

	*bound_low = 0;
	*bound_high = DAC_COUNT_HIGHEST_VAL;

	rc = k_mutex_unlock(&dac_mtx);
	if (rc != 0) {
		LOG_ERR("dac_range() unlock mutex failed, err %d", rc);
		return rc;
	}

	ERS_MUTEX_UNLOCK(dac_mtx, dac);
done:
	return rc;
}

int32_t dac_init(void)
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

	dac_initialized_fs = 1;

	LOG_INF("Ready to produce signal on DAC channel %d.",
		DAC_CHANNEL_ID);

	return rc;
}
