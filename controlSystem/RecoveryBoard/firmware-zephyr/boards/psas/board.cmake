# SPDX-License-Identifier: Apache-2.0
#
# Note this file copied from Zephyr 3.7.0 boards/st/nucleo_f091rc/board.cmake

board_runner_args(jlink "--device=STM32F091RC" "--speed=4000")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
