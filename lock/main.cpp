#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/rand32.h>

#include "bluetooth.h"
#include "buzzer.h"
#include "eeprom.h"
#include "keyboard.h"
#include "rgb_led.h"
#include "sequences.h"

LOG_MODULE_REGISTER();

constexpr gpio_dt_spec reed_switch = GPIO_DT_SPEC_GET(DT_NODELABEL(reed_switch), gpios);
constexpr gpio_dt_spec sw1 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_sw1), gpios);
constexpr gpio_dt_spec sw2 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_sw2), gpios);

RgbLed led;
RgbLedSequencer led_sequencer(led);
Buzzer buzzer;

using namespace std;

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
                           const void *buf, uint16_t len, uint16_t offset,
                           uint8_t flags) {
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
                           const void *buf, uint16_t len, uint16_t offset,
                           uint8_t flags) {
  LOG_INF("Blink!");
  led_sequencer.StartOrRestart(lsqFastBlink);
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


int main() {
  LOG_WRN("Hello! Application started successfully.");
  InitBleAdvertising(ConnectableFastAdvertisingParams());

  for (auto& spec : {reed_switch, sw1, sw2}) {
    gpio_pin_configure_dt(&spec, GPIO_INPUT);
  }

  Keyboard keyboard([&](char c) {
      LOG_INF("Pressed %c", c);
      if (c == '*') {
        buzzer.Beep(200, 700, 300);
      }
  });

  led_sequencer.StartOrRestart(lsqStart);

  eeprom::EnablePower();

  for (int i = 0; i < 200; ++i) {
    uint16_t in = sys_rand32_get() % 32768 + 23;
    uint32_t address = (2 * sys_rand32_get()) % 1024;
    eeprom::Write(in, address);
    k_sleep(K_MSEC(5));
    uint16_t out = eeprom::Read<uint16_t>(address);
    if (in != out) {
      LOG_ERR("EEPROM read/write failed: %d != %d (address %d)", in, out, address);
    }
  }

  LOG_ERR("EEPROM read/write test finished");
  buzzer.Beep(100, 600, 100);

  initializer_list<pair<gpio_dt_spec, const char*>> buttons{{reed_switch, "Reed"}, {sw1, "Sw1"}, {sw2, "Sw2"}};

  while (true) {
    for (auto& [spec, name] : buttons) {
      if (gpio_pin_get_dt(&spec)) {
        LOG_INF("%s pressed", name);
      }
    }
    k_sleep(K_MSEC(100));
  }
}

