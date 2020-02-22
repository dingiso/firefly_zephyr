#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <drivers/pwm.h>

#include "color.h"

class RgbLed {
public:
  RgbLed();
  ~RgbLed();

  void SetColor(const Color& color);
  void SetColorSmooth(const Color& color, uint32_t delay_ms);
private:
  // Changes color of the physical LED to the color_.
  void ActuateColor();
  void OnTimer();

  Color color_ = {0, 0, 0};
  Color target_color_ = {0, 0, 0};
  uint32_t timer_period_ = 0;
  device* device_ = device_get_binding(DT_PWM_LEDS_PWM_LED_R_PWMS_CONTROLLER);
  k_timer timer_;
  const std::function<void()> timer_callback_;
};

// All tree LEDs (R, G, B) must be configured in the devictree to use same PMW controller.
static_assert(std::string_view(DT_PWM_LEDS_PWM_LED_R_PWMS_CONTROLLER) == std::string_view(DT_PWM_LEDS_PWM_LED_G_PWMS_CONTROLLER));
static_assert(std::string_view(DT_PWM_LEDS_PWM_LED_R_PWMS_CONTROLLER) == std::string_view(DT_PWM_LEDS_PWM_LED_B_PWMS_CONTROLLER));
