#include <errno.h>
#include <logging/log.h>
#include <stddef.h>
#include <string.h>
#include <sys/byteorder.h>
#include <sys/printk.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>

#include "battery.h"
#include "bluetooth.h"
#include "cc1101.h"
#include "timer.h"
#include "rgb_led.h"
#include "persistent.h"
#include "magic_path_packet.h"
#include "scoped_mutex_lock.h"

LOG_MODULE_REGISTER();

namespace {

K_MUTEX_DEFINE(packet_mutex);
Persistent<MagicPathRadioPacket> packet(0x00000011); // Guarded by packet_mutex

const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
                  0x0f, 0x18), /* Battery Service */
};

RgbLed led;

/* Radio packet ID, UUID 8ec87064-8865-4eca-82e0-2ea8e45e8221 */
struct bt_uuid_128 radio_packet_id_characteristic_uuid = BT_UUID_INIT_128(
    0x21, 0x82, 0x5e, 0xe4, 0xa8, 0x2e, 0xe0, 0x82,
    0xca, 0x4e, 0x65, 0x88, 0x64, 0x70, 0xc8, 0x8e);

/* Radio packet color, UUID 8ec87065-8865-4eca-82e0-2ea8e45e8221 */
struct bt_uuid_128 radio_packet_color_characteristic_uuid = BT_UUID_INIT_128(
    0x21, 0x82, 0x5e, 0xe4, 0xa8, 0x2e, 0xe0, 0x82,
    0xca, 0x4e, 0x65, 0x88, 0x65, 0x70, 0xc8, 0x8e);

/* Radio packet background color, UUID 8ec87066-8865-4eca-82e0-2ea8e45e8221 */
struct bt_uuid_128 radio_packet_background_color_characteristic_uuid = BT_UUID_INIT_128(
    0x21, 0x82, 0x5e, 0xe4, 0xa8, 0x2e, 0xe0, 0x82,
    0xca, 0x4e, 0x65, 0x88, 0x66, 0x70, 0xc8, 0x8e);

/* Radio packet configure mode, UUID 8ec87067-8865-4eca-82e0-2ea8e45e8221 */
struct bt_uuid_128 radio_packet_config_mode_characteristic_uuid = BT_UUID_INIT_128(
    0x21, 0x82, 0x5e, 0xe4, 0xa8, 0x2e, 0xe0, 0x82,
    0xca, 0x4e, 0x65, 0x88, 0x67, 0x70, 0xc8, 0x8e);

template<size_t Offset, size_t Size> ssize_t read_radio_packet(
  struct bt_conn *conn,
  const struct bt_gatt_attr *attr,
  void *buf, uint16_t len, uint16_t offset) {
  ScopedMutexLock l(packet_mutex);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, reinterpret_cast<uint8_t*>(&(packet.value())) + Offset, Size);
}

template<size_t Offset, size_t Size> ssize_t write_radio_packet(
  struct bt_conn *conn,
  const struct bt_gatt_attr *attr,
  const void *buf, uint16_t len, uint16_t offset,
  uint8_t flags) {
  if (offset + len > Size) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }
  ScopedMutexLock l(packet_mutex);
  memcpy(reinterpret_cast<uint8_t*>(&(packet.value())) + Offset, buf, len);
  led.SetColorSmooth(packet.value().color, 1000);
  packet.Save();
  return len;
}

BT_GATT_SERVICE_DEFINE(firefly_service,
                       BT_GATT_PRIMARY_SERVICE(&firefly_service_uuid),
                       BT_GATT_CHARACTERISTIC(&radio_packet_id_characteristic_uuid.uuid,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                              (read_radio_packet<0, 1>), (write_radio_packet<0, 1>), nullptr),
                       BT_GATT_CHARACTERISTIC(&radio_packet_color_characteristic_uuid.uuid,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                              (read_radio_packet<1, 3>), (write_radio_packet<1, 3>), nullptr),
                       BT_GATT_CHARACTERISTIC(&radio_packet_background_color_characteristic_uuid.uuid,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                              (read_radio_packet<4, 3>), (write_radio_packet<4, 3>), nullptr),
                       BT_GATT_CHARACTERISTIC(&radio_packet_config_mode_characteristic_uuid.uuid,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                              (read_radio_packet<7, 1>), (write_radio_packet<7, 1>), nullptr),
);

} // namespace

void main(void) {
  InitBleAdvertising(ConnectableFastAdvertisingParams());

  packet.LoadOrInit({
    .id = 1,
    .color = {0, 255, 0},
    .background_color = {0, 255, 0},
    .configure_mode = false
  });

  led.SetColorSmooth(packet.value().color, 1000);

  Cc1101 cc1101;
  cc1101.Init();
  cc1101.SetChannel(1);

  led.EnablePowerStabilizer();

  auto t2 = RunEvery([](){
    auto v = Battery::GetInstance().GetVoltage();
    SetBatteryLevel(v / 30);
  }, 5000);

  while (true) {
    ScopedMutexLock l(packet_mutex);
    cc1101.Transmit(packet.value());
    k_sleep(K_MSEC(36));
  }
}
