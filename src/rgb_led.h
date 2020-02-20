#pragma once

#include <cstdint>
#include <string_view>
#include <drivers/pwm.h>

#include "color.h"

class RgbLed {
public:
  void SetColor(const Color& color);
private:
  device* device_ = device_get_binding(DT_PWM_LEDS_PWM_LED_R_PWMS_CONTROLLER);
};

// All tree LEDs (R, G, B) must be configured in the devictree to use same PMW controller.
static_assert(std::string_view(DT_PWM_LEDS_PWM_LED_R_PWMS_CONTROLLER) == std::string_view(DT_PWM_LEDS_PWM_LED_G_PWMS_CONTROLLER));
static_assert(std::string_view(DT_PWM_LEDS_PWM_LED_R_PWMS_CONTROLLER) == std::string_view(DT_PWM_LEDS_PWM_LED_B_PWMS_CONTROLLER));
