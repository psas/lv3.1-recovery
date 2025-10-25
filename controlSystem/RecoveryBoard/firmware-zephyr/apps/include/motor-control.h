/**
 * Copyright (c) 2025 Portland State Aerospace Society
 *
 * SPDX-License-Identifier: Apache-2.0
 */

int32_t ers_init_motor_ctrl(void);

// TODO [ ] rename to 'drive_to_lock' and refine as needed for production:
int32_t motor_ctrl_drive_deploy1_high(void);

// TODO [ ] rename to 'drive_to_unlock' and refine as needed for production:
int32_t motor_ctrl_drive_deploy2_high(void);

int32_t motor_ctrl_set_not_motor_ps(const uint32_t value);

// TODO [ ] refactor to separate source file, as LED is not directly motor related:
int32_t motor_ctrl_set_led0(const uint32_t value);
