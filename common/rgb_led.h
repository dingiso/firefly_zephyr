#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <drivers/pwm.h>

#include "color.h"
#include "timer.h"

class RgbLed {
public:
  RgbLed();

  void EnablePowerStabilizer();
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
  device* device_stabilizer_ = device_get_binding(DT_ALIAS_LED_EN_GPIOS_CONTROLLER);
  Timer timer_;
};

// All tree LEDs (R, G, B) must be configured in the devictree to use same PMW controller.
static_assert(std::string_view(DT_PWM_LEDS_PWM_LED_R_PWMS_CONTROLLER) == std::string_view(DT_PWM_LEDS_PWM_LED_G_PWMS_CONTROLLER));
static_assert(std::string_view(DT_PWM_LEDS_PWM_LED_R_PWMS_CONTROLLER) == std::string_view(DT_PWM_LEDS_PWM_LED_B_PWMS_CONTROLLER));
