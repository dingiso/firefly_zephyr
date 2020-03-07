#pragma once
#include <device.h>

#include "timer.h"

class Buzzer {
public:
  void Beep(uint8_t volume, uint16_t frequencyHz, uint16_t duration_ms);

private:
  void Silence();

  device* device_ = device_get_binding(DT_ALIAS_BUZZER_PWMS_CONTROLLER);
  Timer t_{ [this](){ Silence(); } };
};