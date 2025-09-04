/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/kernel.h>

int main(void)
{
	static uint32_t loop_count = 0;

	while (1)
	{
		loop_count++;
		printk("Hello World! %s iter %u\n", CONFIG_BOARD_TARGET, loop_count);
		k_msleep(1000);
	}

	return 0;
}
