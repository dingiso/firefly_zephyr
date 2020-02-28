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
#include "timer.h"
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

/* Service UUID 8ec87060-8865-4eca-82e0-2ea8e45e8221 */
struct bt_uuid_128 firefly_service_uuid = BT_UUID_INIT_128(
    0x21, 0x82, 0x5e, 0xe4, 0xa8, 0x2e, 0xe0, 0x82,
    0xca, 0x4e, 0x65, 0x88, 0x60, 0x70, 0xc8, 0x8e);

/* Characteristic UUID 8ec87061-8865-4eca-82e0-2ea8e45e8221 */
struct bt_uuid_128 radio_packet_characteristic_uuid = BT_UUID_INIT_128(
    0x21, 0x82, 0x5e, 0xe4, 0xa8, 0x2e, 0xe0, 0x82,
    0xca, 0x4e, 0x65, 0x88, 0x61, 0x70, 0xc8, 0x8e);

ssize_t read_radio_packet(struct bt_conn *conn,
                          const struct bt_gatt_attr *attr,
                          void *buf, u16_t len, u16_t offset) {
  ScopedMutexLock l(packet_mutex);
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &packet, sizeof(packet));
}

ssize_t write_radio_packet(struct bt_conn *conn,
                           const struct bt_gatt_attr *attr,
                           const void *buf, u16_t len, u16_t offset,
                           u8_t flags) {
  if (offset + len > sizeof(packet)) {
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
  }
  ScopedMutexLock l(packet_mutex);
  memcpy(&packet, buf, len);

  return len;
}

// Zephyr's own definition of BT_GATT_ATTRIBUTE is not compatible with C++.
// https://github.com/zephyrproject-rtos/zephyr/pull/23167 send to fix it.
#undef BT_GATT_ATTRIBUTE
#define BT_GATT_ATTRIBUTE(_uuid, _perm, _read, _write, _value) \
  {                                                            \
    .uuid = _uuid,                                             \
    .read = _read,                                             \
    .write = _write,                                           \
    .user_data = _value,                                       \
    .handle = 0,                                               \
    .perm = _perm,                                             \
  }

BT_GATT_SERVICE_DEFINE(firefly_service,
                       BT_GATT_PRIMARY_SERVICE(&firefly_service_uuid),
                       BT_GATT_CHARACTERISTIC(&radio_packet_characteristic_uuid.uuid,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                              read_radio_packet, write_radio_packet, nullptr), );

} // namespace

void main(void) {
  InitBleAdvertising(ConnectableFastAdvertisingParams());

  auto t1 = RunEvery([](){
    ScopedMutexLock l(packet_mutex);
    LOG_INF("Current radio packet is id = %d, c = (%d %d %d), bc = (%d %d %d), cm = %d", packet.id,
        packet.r, packet.g, packet.b,
        packet.r_background, packet.g_background, packet.b_background,
        packet.configure_mode);
  }, 1000);

  auto t2 = RunEvery([&](){
    auto v = Battery::GetInstance().GetVoltage();
    SetBatteryLevel(v / 30);
  }, 5000);

  while (true) {
    k_sleep(K_FOREVER);
  }
}
