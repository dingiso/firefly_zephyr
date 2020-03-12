#include <cstdint>
#include <device.h>
#include <drivers/eeprom.h>
#include <drivers/gpio.h>

namespace eeprom {
namespace internal {
device* kEepromDevice = device_get_binding(DT_ALIAS_EEPROM_LABEL);
}

// Enables eeprom power. Without that reads/writes won't work.
void EnablePower() {
  device* gpio = device_get_binding(DT_ALIAS_EEPROM_EN_GPIOS_CONTROLLER);
  gpio_pin_configure(gpio, DT_ALIAS_EEPROM_EN_GPIOS_PIN, GPIO_OUTPUT_ACTIVE | DT_ALIAS_EEPROM_EN_GPIOS_FLAGS);
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