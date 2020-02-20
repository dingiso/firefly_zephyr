#include "rgb_led.h"
#include <device.h>

namespace {
const uint8_t kFrequencyHertz = 100;
const uint32_t kUsecPerSecond = 1000 * 1000;
const uint32_t kCyclePeriodUs = kUsecPerSecond / kFrequencyHertz;

uint32_t colorComponentToPulseWidth(uint8_t component) {
  return (kCyclePeriodUs / 255u) * component;
}

}

void RgbLed::SetColor(const Color& color) {
  pwm_pin_set_usec(device_, DT_ALIAS_LED_R_PWMS_CHANNEL,
    kCyclePeriodUs, colorComponentToPulseWidth(color.r), /*flags=*/0u);
  pwm_pin_set_usec(device_, DT_ALIAS_LED_G_PWMS_CHANNEL,
    kCyclePeriodUs, colorComponentToPulseWidth(color.g), /*flags=*/0u);
  pwm_pin_set_usec(device_, DT_ALIAS_LED_B_PWMS_CHANNEL,
    kCyclePeriodUs, colorComponentToPulseWidth(color.b), /*flags=*/0u);
}