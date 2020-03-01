#pragma once

#include <cstdint>

struct MagicPathRadioPacket {
  uint8_t id;
  Color color;
  Color background_color;
  bool configure_mode;
} __attribute__((__packed__));