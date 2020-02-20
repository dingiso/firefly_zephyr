#include "color.h"

#include <algorithm>
#include <cstdlib>

namespace {
inline void moveByOne(uint8_t& move_what, uint8_t move_where) {
  if (move_what < move_where) move_what++;
  else if (move_what > move_where) move_what--;
}

inline void moveByStep(uint8_t& move_what, uint8_t move_where, uint8_t step) {
  if (move_what < move_where) {
    if (move_what <= 255 - step) {
      move_what += step;
    } else {
      move_what = 255;
    }
  } else if (move_what > move_where) {
    if (move_what >= step) {
      move_what -= step;
    } else {
      move_what = 0;
    }
  }
}

inline uint32_t CalculateDelay(int16_t difference, uint32_t total_time) {
    return total_time / (abs(difference) + 4) + 1;
}
}

void Color::Adjust(const Color& other) {
  moveByOne(r, other.r);
  moveByOne(g, other.g);
  moveByOne(b, other.b);
}

void Color::Adjust(const Color& other, uint8_t step) {
  moveByStep(r, other.r, step);
  moveByStep(g, other.g, step);
  moveByStep(b, other.b, step);
}

// Adjustment
uint32_t Color::DelayToTheNextAdjustment(const Color& other, uint32_t total_adjustment_time) const {
  return std::min({
    CalculateDelay(r - other.r, total_adjustment_time),
    CalculateDelay(g - other.g, total_adjustment_time),
    CalculateDelay(b - other.b, total_adjustment_time)
  });
}