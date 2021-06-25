#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <drivers/pwm.h>
#include <device.h>

#include "color.h"
#include "timer.h"
#include "sequences.h"

class RgbLed {
public:
  RgbLed();

  void EnablePowerStabilizer();
  void DisablePowerStabilizer();
  void SetColor(const Color& color);
  const Color& GetColor() const;
  void SetColorSmooth(const Color& color, uint32_t delay_ms);
private:
  // Changes color of the physical LED to the color_.
  void ActuateColor();
  void OnTimer();

  Color color_ = {0, 0, 0};
  Color target_color_ = {0, 0, 0};
  uint32_t timer_period_ = 0;
  const device* device_r_ = DEVICE_DT_GET(DT_PWMS_CTLR(DT_ALIAS(led_r)));
  const device* device_g_ = DEVICE_DT_GET(DT_PWMS_CTLR(DT_ALIAS(led_g)));
  const device* device_b_ = DEVICE_DT_GET(DT_PWMS_CTLR(DT_ALIAS(led_b)));
  const device* device_stabilizer_ = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(led_en), gpios));
  Timer timer_;
};


class RgbLedSequencer {
public:
  RgbLedSequencer(RgbLed& led);

  void StartOrRestart(const LedChunk* sequence);

private:
  void StartChunk();
  void EndChunk();

  RgbLed& led_;
  Timer timer_;
  const LedChunk* current_ = nullptr;
};

