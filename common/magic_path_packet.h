#pragma once

#include <cstdint>

struct MagicPathRadioPacket {
  uint8_t id;
  uint8_t r, g, b;
  uint8_t r_background, g_background, b_background;
  bool configure_mode;
} __attribute__((__packed__));