#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/rand32.h>

#include "keyboard.h"
#include "eeprom.h"

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

  eeprom::EnablePower();

  for (int i = 0; i < 200; ++i) {
    uint16_t in = sys_rand32_get() % 32768 + 23;
    uint32_t address = (2 * sys_rand32_get()) % 1024;
    eeprom::Write(in, address);
    k_sleep(K_MSEC(5));
    uint16_t out = eeprom::Read<uint16_t>(address);
    if (in != out) {
      LOG_ERR("EEPROM read/write failed: %d != %d (address %d)", in, out, address);
    }
  }

  LOG_ERR("EEPROM read/write test finished");

  initializer_list<pair<gpio_dt_spec, const char*>> buttons{{reed_switch, "Reed"}, {sw1, "Sw1"}, {sw2, "Sw2"}};

  while (true) {
    for (auto& [spec, name] : buttons) {
      if (gpio_pin_get_dt(&spec)) {
        LOG_INF("%s pressed", name);
      }
    }
    gpio_pin_set_dt(&led, 0);
    k_sleep(K_MSEC(250));
    gpio_pin_set_dt(&led, 1);
    k_sleep(K_MSEC(250));
  }
}

