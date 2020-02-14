#include <device.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <logging/log.h>
#include <zephyr.h>

#include <vector>

#include "cc1101.h"

LOG_MODULE_REGISTER();

struct MagicPathRadioPacket {
  uint8_t r, g, b;
} __attribute__((__packed__));

void main(void) {
  LOG_WRN("Hello! Application started successfully.");

  auto device = device_get_binding(DT_ALIAS_LED2_GPIOS_CONTROLLER);
  if (device == nullptr) {
    return;
  }

  auto ret = gpio_pin_configure(device, DT_ALIAS_LED2_GPIOS_PIN, GPIO_OUTPUT_ACTIVE | DT_ALIAS_LED2_GPIOS_FLAGS);
  if (ret < 0) {
    return;
  }

  Cc1101 cc1101;
  cc1101.Init();
  cc1101.SetChannel(1);
  MagicPathRadioPacket pkt = {
      .r = 0,
      .g = 0,
      .b = 255};

  const std::vector<int> led_states = {1, 0, 0, 0, 1, 0, 0, 0, 0, 0};
  while (true) {
    for (const bool state : led_states) {
      gpio_pin_set(device, DT_ALIAS_LED2_GPIOS_PIN, state);
      k_sleep(100);
    }
    cc1101.Transmit(pkt);

    k_sleep(1000);
  }
}
