#include "keyboard.h"

#include <zephyr/drivers/gpio.h>
#include "pw_assert/check.h"
#include "timer.h"
#include "pw_log/log.h"

namespace {
constexpr gpio_dt_spec in[] = {
  GPIO_DT_SPEC_GET(DT_NODELABEL(in3), gpios),
  GPIO_DT_SPEC_GET(DT_NODELABEL(in2), gpios),
  GPIO_DT_SPEC_GET(DT_NODELABEL(in1), gpios)
};

constexpr gpio_dt_spec out[] = {
  GPIO_DT_SPEC_GET(DT_NODELABEL(out4), gpios),
  GPIO_DT_SPEC_GET(DT_NODELABEL(out3), gpios),
  GPIO_DT_SPEC_GET(DT_NODELABEL(out2), gpios),
  GPIO_DT_SPEC_GET(DT_NODELABEL(out1), gpios)
};

constexpr char keymap[] = {
  '1', '2', '3',
  '4', '5', '6',
  '7', '8', '9',
  '*', '0', '#',
};

}

Keyboard::Keyboard(pw::Function<void(char)> callback):
  callback_(std::move(callback)),
  timer_(RunEvery([this]() {
    bool pressed_new = false;
    for (int8_t row = 0; row < 4; ++row) {
      if (gpio_pin_get_dt(&out[row])) {
        pressed_new = true;
        if (last_pressed_column_ == -1) {
          callback_(keymap[row * 3 + column]);
        }
      }
    }
    if (pressed_new) {
      last_pressed_column_ = column;
    } else if (last_pressed_column_ == column) {
      last_pressed_column_ = -1;
    }
    gpio_pin_set_dt(&in[column], 0);
    column = (column + 1) % 3;
    gpio_pin_set_dt(&in[column], 1);
  }, 50)) {
  for (auto& s : in) {
    PW_CHECK_INT_EQ(0, gpio_pin_configure_dt(&s, GPIO_OUTPUT));
  }
  for (auto& s : out) {
    PW_CHECK_INT_EQ(0, gpio_pin_configure_dt(&s, GPIO_INPUT));
  }
}
