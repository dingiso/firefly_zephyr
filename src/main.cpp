#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <logging/log.h>

#include <vector>

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

void main(void)
{
	LOG_WRN("Hello! Application started successfully.");

	auto device = device_get_binding(DT_ALIAS_LED2_GPIOS_CONTROLLER);
	if (device == nullptr) {
		return;
	}

	auto ret = gpio_pin_configure(device, DT_ALIAS_LED2_GPIOS_PIN, GPIO_OUTPUT_ACTIVE | DT_ALIAS_LED2_GPIOS_FLAGS);
	if (ret < 0) {
		return;
	}

	auto sleep_time_ms = 100;
	const std::vector<int> led_states = {1, 0, 0, 0, 1, 0, 0, 0, 0, 0};
	while (true) {
		LOG_INF("Current sleep time is %d", sleep_time_ms);
		for (const bool state: led_states) {
			gpio_pin_set(device, DT_ALIAS_LED2_GPIOS_PIN, state);
			k_sleep(sleep_time_ms++);
		}
	}
}
