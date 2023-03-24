#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER();

constexpr gpio_dt_spec led_en = GPIO_DT_SPEC_GET(DT_NODELABEL(led_en), gpios);
constexpr gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(led_b), gpios);

int main() {
  LOG_WRN("Hello! Application started successfully.");

  gpio_pin_configure_dt(&led_en, GPIO_OUTPUT);
  gpio_pin_configure_dt(&led, GPIO_OUTPUT);

  gpio_pin_set_dt(&led_en, 0);

  while (true) {
    LOG_INF("Blink!");
    gpio_pin_set_dt(&led, 0);
    k_sleep(K_MSEC(500));
    gpio_pin_set_dt(&led, 1);
    k_sleep(K_MSEC(500));
  }
}
