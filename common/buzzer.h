#pragma once
#include <device.h>

#include "timer.h"

class Buzzer {
public:
  void Beep(uint8_t volume, uint16_t frequencyHz, uint16_t duration_ms);

private:
  void Silence();

  const device* device_ = DEVICE_DT_GET(DT_PWMS_CTLR(DT_ALIAS(buzzer)));
  Timer t_{ [this](){ Silence(); } };
};

