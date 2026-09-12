#!/bin/bash
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
west build -b ers-v3p1 -p -- -DBOARD_ROOT=..
exit $?
