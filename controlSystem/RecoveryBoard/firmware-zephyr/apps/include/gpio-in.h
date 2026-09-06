#ifndef GPIO_IN_H
#define GPIO_IN_H

// TODO [ ] Add comment blocks
// TODO [ ] Rename APIs per public API naming convention, which is <module_name>_.

int32_t gpio_in_init(void);

int32_t ers_gpios_set_deploy1(const uint32_t value);

int32_t ers_gpios_set_deploy2(const uint32_t value);

int32_t ers_gpios_set_not_motor_ps(const uint32_t value);

#endif // GPIO_IN_H
