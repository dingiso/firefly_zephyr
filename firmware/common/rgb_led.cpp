#include "rgb_led.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

namespace {
const uint32_t kFrequencyHertz = 50;
const uint32_t kUsecPerSecond = 1000 * 1000;
const uint32_t kCyclePeriodUs = kUsecPerSecond / kFrequencyHertz;

uint32_t colorComponentToPulseWidth(uint8_t component) {
  return (kCyclePeriodUs / 255u) * component;
}

}

RgbLed::RgbLed(): timer_([this](){ this->OnTimer(); }) {
}

void RgbLed::EnablePowerStabilizer() {
  gpio_pin_configure_dt(&device_stabilizer_spec_, GPIO_OUTPUT_ACTIVE);
}

void RgbLed::DisablePowerStabilizer() {
  gpio_pin_set_dt(&device_stabilizer_spec_, 0);
}

void RgbLed::SetColor(const Color& color) {
  color_ = color;
  target_color_ = color;
  timer_period_ = 0;
  timer_.Cancel();
  ActuateColor();
}

const Color& RgbLed::GetColor() const {
  return color_;
}

void RgbLed::ActuateColor() {
  pwm_set(device_r_, DT_PWMS_CHANNEL(DT_ALIAS(led_r)),
    PWM_USEC(kCyclePeriodUs), PWM_USEC(colorComponentToPulseWidth(color_.r)), DT_PWMS_FLAGS(DT_ALIAS(led_r)));
  pwm_set(device_g_, DT_PWMS_CHANNEL(DT_ALIAS(led_g)),
    PWM_USEC(kCyclePeriodUs), PWM_USEC(colorComponentToPulseWidth(color_.g)), DT_PWMS_FLAGS(DT_ALIAS(led_g)));
  pwm_set(device_b_, DT_PWMS_CHANNEL(DT_ALIAS(led_b)),
    PWM_USEC(kCyclePeriodUs), PWM_USEC(colorComponentToPulseWidth(color_.b)), DT_PWMS_FLAGS(DT_ALIAS(led_b)));
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


RgbLedSequencer::RgbLedSequencer(RgbLed& led): led_(led), timer_([this](){ this->EndChunk(); }) {
}

void RgbLedSequencer::StartOrRestart(const LedChunk* sequence) {
  timer_.Cancel();
  current_ = sequence;
  StartChunk();
}

void RgbLedSequencer::StartChunk() {
  if (current_->type == LedChunkType::Finish) return;
  if (current_->type == LedChunkType::SetColor) {
    led_.SetColorSmooth(current_->color, current_->time_ms);
  }
  timer_.RunDelayed(current_->time_ms);
}

void RgbLedSequencer::EndChunk() {
  ++current_;
  StartChunk();
}
