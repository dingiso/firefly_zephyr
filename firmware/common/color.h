#pragma once

#include <cstdint>

struct Color {
  uint8_t r = 0, g = 0, b = 0;
  Color() {}
  Color(uint8_t r_, uint8_t g_, uint8_t b_) : r(r_), g(g_), b(b_) {}
  bool operator==(const Color& other) const { return r == other.r && g == other.g && b == other.b; };

  bool operator==(const Color& other) { return r == other.r && g == other.g && b == other.b; } ;
  bool operator!=(const Color& other) { return !(*this == other); } ;

  // Change to the direction of other. Each component changes not more than by 1.
  void Adjust(const Color& other);

  // Change to the direction of other. Each component changes not more than by step.
  void Adjust(const Color& other, uint8_t step);

  // We would like to go from this color to the other, each time adjusting this by Adjust.
  // We have total_adjustment_time time in total, what's the interval between adjustments?
  uint32_t DelayToTheNextAdjustment(const Color& other, uint32_t total_adjustment_time) const;
} __attribute__((packed));


