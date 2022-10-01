#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

#include <zephyr/drivers/pwm.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

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
  const gpio_dt_spec device_stabilizer_spec_ = GPIO_DT_SPEC_GET(DT_ALIAS(led_en), gpios);
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

