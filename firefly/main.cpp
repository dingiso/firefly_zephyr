#include <array>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/spi.h>
#include <logging/log.h>
#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>

#include "battery.h"
#include "buzzer.h"
#include "cc1101.h"
#include "rgb_led.h"
#include "bluetooth.h"
#include "magic_path_packet.h"

LOG_MODULE_REGISTER();

namespace {
Buzzer buzzer;
}

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

/* Service UUID 8ec87060-8865-4eca-82e0-2ea8e45e8221 */
struct bt_uuid_128 firefly_service_uuid = BT_UUID_INIT_128(
    0x21, 0x82, 0x5e, 0xe4, 0xa8, 0x2e, 0xe0, 0x82,
    0xca, 0x4e, 0x65, 0x88, 0x60, 0x70, 0xc8, 0x8e);

/* Beep Characteristic, UUID 8ec87062-8865-4eca-82e0-2ea8e45e8221 */
struct bt_uuid_128 beep_characteristic_uuid = BT_UUID_INIT_128(
    0x21, 0x82, 0x5e, 0xe4, 0xa8, 0x2e, 0xe0, 0x82,
    0xca, 0x4e, 0x65, 0x88, 0x62, 0x70, 0xc8, 0x8e);

/* Blink Characteristic, UUID 8ec87063-8865-4eca-82e0-2ea8e45e8221 */
struct bt_uuid_128 blink_characteristic_uuid = BT_UUID_INIT_128(
    0x21, 0x82, 0x5e, 0xe4, 0xa8, 0x2e, 0xe0, 0x82,
    0xca, 0x4e, 0x65, 0x88, 0x63, 0x70, 0xc8, 0x8e);

ssize_t write_beep(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr,
                           const void *buf, u16_t len, u16_t offset,
                           u8_t flags) {
  if (len <= 0) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }
  uint8_t volume = *reinterpret_cast<const uint8_t*>(buf);
  LOG_INF("Beep!");
  buzzer.Beep(volume, 600, 300);
  return len;
}

ssize_t write_blink(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr,
                           const void *buf, u16_t len, u16_t offset,
                           u8_t flags) {
  LOG_INF("Blink!");
  return len;
}

BT_GATT_SERVICE_DEFINE(firefly_service,
                       BT_GATT_PRIMARY_SERVICE(&firefly_service_uuid),
                       BT_GATT_CHARACTERISTIC(&beep_characteristic_uuid.uuid,
                                              BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_WRITE,
                                              nullptr, write_beep, nullptr),
                       BT_GATT_CUD("Beep", BT_GATT_PERM_READ),
                       BT_GATT_CHARACTERISTIC(&blink_characteristic_uuid.uuid,
                                              BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_WRITE,
                                              nullptr, write_blink, nullptr),
                       BT_GATT_CUD("Blink", BT_GATT_PERM_READ),
);


void main(void) {
  LOG_WRN("Hello! Application started successfully.");
  InitBleAdvertising(ConnectableSlowAdvertisingParams());

  Cc1101 cc1101;
  cc1101.Init();
  cc1101.SetChannel(1);

  RgbLed led;
  led.EnablePowerStabilizer();
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
