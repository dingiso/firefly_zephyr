#include "buzzer.h"

#include <drivers/gpio.h>
#include <drivers/pwm.h>

namespace {
const uint32_t kUsecPerSecond = 1000 * 1000;
}

void Buzzer::Beep(uint8_t volume, uint16_t frequency_hz, uint16_t duration_ms) {
  const uint32_t pulse_width_us = (kUsecPerSecond / (255u * frequency_hz)) * volume;
  const uint32_t cycle_period_us = kUsecPerSecond / frequency_hz;

  pwm_pin_set_usec(device_, DT_ALIAS_BUZZER_PWMS_CHANNEL, cycle_period_us, pulse_width_us, /*flags=*/0u);
  t_.RunDelayed(duration_ms);
}

void Buzzer::Silence() {
  pwm_pin_set_usec(device_, DT_ALIAS_BUZZER_PWMS_CHANNEL, 1000, 0, /*flags=*/0u);
}