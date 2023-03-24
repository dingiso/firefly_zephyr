#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "keyboard.h"

LOG_MODULE_REGISTER();

constexpr gpio_dt_spec led_en = GPIO_DT_SPEC_GET(DT_NODELABEL(led_en), gpios);
constexpr gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(led_b), gpios);

constexpr gpio_dt_spec reed_switch = GPIO_DT_SPEC_GET(DT_NODELABEL(reed_switch), gpios);
constexpr gpio_dt_spec sw1 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_sw1), gpios);
constexpr gpio_dt_spec sw2 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_sw2), gpios);

using namespace std;

int main() {
  LOG_WRN("Hello! Application started successfully.");

  gpio_pin_configure_dt(&led_en, GPIO_OUTPUT);
  gpio_pin_configure_dt(&led, GPIO_OUTPUT);

  for (auto& spec : {reed_switch, sw1, sw2}) {
    gpio_pin_configure_dt(&spec, GPIO_INPUT);
  }

  gpio_pin_set_dt(&led_en, 0);

  Keyboard keyboard([](char c) {
      LOG_INF("Pressed %c", c);
  });

  initializer_list<pair<gpio_dt_spec, const char*>> buttons{{reed_switch, "Reed"}, {sw1, "Sw1"}, {sw2, "Sw2"}};

  while (true) {
    for (auto& [spec, name] : buttons) {
      if (gpio_pin_get_dt(&spec)) {
        LOG_INF("%s pressed", name);
      }
    }
    LOG_INF("Blink!");
    gpio_pin_set_dt(&led, 0);
    k_sleep(K_MSEC(250));
    gpio_pin_set_dt(&led, 1);
    k_sleep(K_MSEC(250));
  }
}

