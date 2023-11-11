#pragma once

#include "pw_function/function.h"
#include "timer.h"

class Keyboard {
 public:
  explicit Keyboard(pw::Function<void(char)> callback);

 private:
  int8_t last_pressed_column_ = -1;
  int8_t column = 0;
  pw::Function<void(char)> callback_;
  Timer timer_;
};
