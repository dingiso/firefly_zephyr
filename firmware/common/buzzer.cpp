#include "buzzer.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>

namespace {
const uint32_t kUsecPerSecond = 1000 * 1000;
}

void Buzzer::Beep(uint8_t volume, uint16_t frequency_hz, uint16_t duration_ms) {
  const uint32_t pulse_width_us = (kUsecPerSecond / (255u * frequency_hz)) * volume;
  const uint32_t cycle_period_us = kUsecPerSecond / frequency_hz;

  pwm_set(device_, DT_PWMS_CHANNEL(DT_ALIAS(buzzer)), PWM_USEC(cycle_period_us), PWM_USEC(pulse_width_us), /*flags=*/0u);
  t_.RunDelayed(duration_ms);
}

void Buzzer::Silence() {
  pwm_set(device_, DT_PWMS_CHANNEL(DT_ALIAS(buzzer)), PWM_USEC(1000), PWM_USEC(0), /*flags=*/0u);
}