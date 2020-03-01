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
#include "magic_path_packet.h"
#include "scoped_mutex_lock.h"


LOG_MODULE_REGISTER();

namespace {

K_MUTEX_DEFINE(packet_mutex);
MagicPathRadioPacket packet = {}; // Guarded by packet_mutex

const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
                  0x0f, 0x18), /* Battery Service */
};

RgbLed led;

/* Service UUID 8ec87060-8865-4eca-82e0-2ea8e45e8221 */
struct bt_uuid_128 firefly_service_uuid = BT_UUID_INIT_128(
    0x21, 0x82, 0x5e, 0xe4, 0xa8, 0x2e, 0xe0, 0x82,
    0xca, 0x4e, 0x65, 0x88, 0x60, 0x70, 0xc8, 0x8e);

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

ssize_t read_radio_packet_id(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr,
                             void *buf, u16_t len, u16_t offset) {
  ScopedMutexLock l(packet_mutex);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &packet.id, sizeof(packet.id));
}

ssize_t write_radio_packet_id(struct bt_conn *conn,
                              const struct bt_gatt_attr *attr,
                              const void *buf, u16_t len, u16_t offset,
                              u8_t flags) {
  if (offset + len > sizeof(packet.id)) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }
  ScopedMutexLock l(packet_mutex);
  memcpy(&packet.id, buf, len);

  return len;
}

ssize_t read_radio_packet_color(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr,
                             void *buf, u16_t len, u16_t offset) {
  ScopedMutexLock l(packet_mutex);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &packet.r, 3);
}

ssize_t write_radio_packet_color(struct bt_conn *conn,
                              const struct bt_gatt_attr *attr,
                              const void *buf, u16_t len, u16_t offset,
                              u8_t flags) {
  if (offset + len > 3) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }
  ScopedMutexLock l(packet_mutex);
  memcpy(&packet.r, buf, len);
  led.SetColor({packet.r, packet.g, packet.b});
  return len;
}

ssize_t read_radio_packet_background_color(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr,
                             void *buf, u16_t len, u16_t offset) {
  ScopedMutexLock l(packet_mutex);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &packet.r_background, 3);
}

ssize_t write_radio_packet_background_color(struct bt_conn *conn,
                              const struct bt_gatt_attr *attr,
                              const void *buf, u16_t len, u16_t offset,
                              u8_t flags) {
  if (offset + len > 3) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }
  ScopedMutexLock l(packet_mutex);
  memcpy(&packet.r_background, buf, len);

  return len;
}

ssize_t read_radio_packet_config_mode(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr,
                             void *buf, u16_t len, u16_t offset) {
  ScopedMutexLock l(packet_mutex);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &packet.configure_mode, 1);
}

ssize_t write_radio_packet_config_mode(struct bt_conn *conn,
                              const struct bt_gatt_attr *attr,
                              const void *buf, u16_t len, u16_t offset,
                              u8_t flags) {
  if (offset + len > sizeof(packet.configure_mode)) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }
  ScopedMutexLock l(packet_mutex);
  memcpy(&packet.configure_mode, buf, len);

  return len;
}

BT_GATT_SERVICE_DEFINE(firefly_service,
                       BT_GATT_PRIMARY_SERVICE(&firefly_service_uuid),
                       BT_GATT_CHARACTERISTIC(&radio_packet_id_characteristic_uuid.uuid,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                              read_radio_packet_id, write_radio_packet_id, nullptr),
                       BT_GATT_CHARACTERISTIC(&radio_packet_color_characteristic_uuid.uuid,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                              read_radio_packet_color, write_radio_packet_color, nullptr),
                       BT_GATT_CHARACTERISTIC(&radio_packet_background_color_characteristic_uuid.uuid,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                              read_radio_packet_background_color, write_radio_packet_background_color, nullptr),
                       BT_GATT_CHARACTERISTIC(&radio_packet_config_mode_characteristic_uuid.uuid,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                              read_radio_packet_config_mode, write_radio_packet_config_mode, nullptr),
);

} // namespace

void main(void) {
  InitBleAdvertising(ConnectableFastAdvertisingParams());

  Cc1101 cc1101;
  cc1101.Init();
  cc1101.SetChannel(1);

  led.EnablePowerStabilizer();

  auto t2 = RunEvery([&](){
    auto v = Battery::GetInstance().GetVoltage();
    SetBatteryLevel(v / 30);
  }, 5000);

  while (true) {
    ScopedMutexLock l(packet_mutex);
    cc1101.Transmit(packet);
    k_sleep(36);
  }
}
