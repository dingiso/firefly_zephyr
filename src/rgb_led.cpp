#include "rgb_led.h"
#include <device.h>

namespace {
const uint8_t kFrequencyHertz = 50;
const uint32_t kUsecPerSecond = 1000 * 1000;
const uint32_t kCyclePeriodUs = kUsecPerSecond / kFrequencyHertz;

uint32_t colorComponentToPulseWidth(uint8_t component) {
  return (kCyclePeriodUs / 255u) * component;
}

}

void RgbLed::SetColor(uint8_t r, uint8_t g, uint8_t b) {
  pwm_pin_set_usec(device_, DT_ALIAS_LED_R_PWMS_CHANNEL,
    kCyclePeriodUs, colorComponentToPulseWidth(r), /*flags=*/0u);
  pwm_pin_set_usec(device_, DT_ALIAS_LED_G_PWMS_CHANNEL,
    kCyclePeriodUs, colorComponentToPulseWidth(g), /*flags=*/0u);
  pwm_pin_set_usec(device_, DT_ALIAS_LED_B_PWMS_CHANNEL,
    kCyclePeriodUs, colorComponentToPulseWidth(b), /*flags=*/0u);
}