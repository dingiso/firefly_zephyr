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

RgbLed::RgbLed(): timer_([this](){ this->OnTimer(); }) {
}

void RgbLed::SetColor(const Color& color) {
  color_ = color;
  target_color_ = color;
  timer_period_ = 0;
  timer_.Cancel();
  ActuateColor();
}

void RgbLed::ActuateColor() {
  pwm_pin_set_usec(device_, DT_ALIAS_LED_R_PWMS_CHANNEL,
    kCyclePeriodUs, colorComponentToPulseWidth(color_.r), /*flags=*/0u);
  pwm_pin_set_usec(device_, DT_ALIAS_LED_G_PWMS_CHANNEL,
    kCyclePeriodUs, colorComponentToPulseWidth(color_.g), /*flags=*/0u);
  pwm_pin_set_usec(device_, DT_ALIAS_LED_B_PWMS_CHANNEL,
    kCyclePeriodUs, colorComponentToPulseWidth(color_.b), /*flags=*/0u);
}

void RgbLed::SetColorSmooth(const Color& color, uint32_t delay_ms) {
  target_color_ = color;
  timer_period_ = color_.DelayToTheNextAdjustment(target_color_, delay_ms);
  timer_.RunDelayed(timer_period_);
}

void RgbLed::OnTimer() {
  color_.Adjust(target_color_);
  ActuateColor();
  if (color_ != target_color_) timer_.RunDelayed(timer_period_);
}