#include <array>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <logging/log.h>
#include <zephyr.h>
#include "battery.h"
#include "cc1101.h"
#include "rgb_led.h"
#include "bluetooth.h"

LOG_MODULE_REGISTER();

struct MagicPathRadioPacket {
  uint8_t id;
  uint8_t r, g, b;
  uint8_t r_background, g_background, b_background;
  bool configure_mode;
} __attribute__((__packed__));

struct ColorAndTimestamp {
  ColorAndTimestamp(): timestamp(0), color(0, 0, 0) {}
  int32_t timestamp;
  Color color;
};

class PacketsLog {
public:
  void ProcessRadioPacket(const MagicPathRadioPacket& p) {
    if (p.id >= colors_.size()) {
      LOG_WRN("Unexpected radio packet with ID = %d", p.id);
      return;
    }

    if (p.configure_mode) {
      background_color_ = Color(p.r_background, p.g_background, p.b_background);
    }

    colors_[p.id].timestamp = k_uptime_get();
    colors_[p.id].color = Color(p.r, p.g, p.b);
  }

  Color GetColor() const {
    const auto current_t = k_uptime_get();
    uint16_t r = 0, g = 0, b = 0;
    bool see_something = false;
    for (const auto& entry: colors_) {
      if (entry.timestamp != 0 && current_t - entry.timestamp < 3000) {
        r += entry.color.r;
        g += entry.color.g;
        b += entry.color.b;
        see_something = true;
      }
    }

    if (!see_something) {
      return background_color_;
    }

    return Color(std::min<uint16_t>(255, r), std::min<uint16_t>(255, g), std::min<uint16_t>(255, b));
  }

private:
  Color background_color_ = {0, 0, 0};
  std::array<ColorAndTimestamp, 20> colors_;
};

void main(void) {
  LOG_WRN("Hello! Application started successfully.");
  InitBleAdvertising();

  Cc1101 cc1101;
  cc1101.Init();
  cc1101.SetChannel(1);

  RgbLed led;
  PacketsLog log;

  auto t1 = RunEvery([&led, &log](){
    auto c = log.GetColor();
    LOG_DBG("New color is %d %d %d", c.r, c.g, c.b);
    led.SetColorSmooth(c, 1000);
  }, 1000);

  auto t2 = RunEvery([&](){
    auto v = Battery::GetInstance().GetVoltage();
    LOG_INF("Adc result: %d", v);
    SetBatteryLevel(v / 30);
  }, 5000);

  while (true) {
    MagicPathRadioPacket pkt;
    for (int ch = 0; ch < 4; ++ch) {
      cc1101.SetChannel(ch);
      if (cc1101.Receive(63, &pkt)) {
        LOG_DBG("Got packet! ID=%d, R=%d, G=%d, B=%d", pkt.id, pkt.r, pkt.g, pkt.b);
        log.ProcessRadioPacket(pkt);
      }
    }
    k_sleep(810);
  }
}
