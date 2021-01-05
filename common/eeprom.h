#pragma once

#include <cstdint>
#include <device.h>
#include <drivers/eeprom.h>
#include <drivers/gpio.h>

namespace eeprom {
namespace internal {
const device* kEepromDevice = device_get_binding(DT_LABEL(DT_ALIAS(eeprom)));
}

// Enables eeprom power. Without that reads/writes won't work.
void EnablePower() {
  const device* gpio = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(eeprom_en), gpios));
  gpio_pin_configure(gpio, DT_GPIO_PIN(DT_ALIAS(eeprom_en), gpios), GPIO_OUTPUT_ACTIVE | DT_GPIO_FLAGS(DT_ALIAS(eeprom_en), gpios));
}

template<typename T> T Read(uint32_t offset) {
  T result;
  eeprom_read(internal::kEepromDevice, offset, &result, sizeof(result));
  return result;
}

template<typename T> void Write(const T& value, uint32_t offset) {
  eeprom_write(internal::kEepromDevice, offset, &value, sizeof(value));
}

}