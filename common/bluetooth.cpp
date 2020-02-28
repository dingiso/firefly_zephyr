#include "bluetooth.h"

#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/services/bas.h>
#include <bluetooth/uuid.h>
#include <logging/log.h>
LOG_MODULE_DECLARE();

static const bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
                  0x0f, 0x18), /* Battery Service */
};

bt_le_adv_param ConnectableSlowAdvertisingParams() {
  return {
    .id = 0,
    .options = BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME,
    .interval_min = BT_GAP_ADV_SLOW_INT_MIN,
    .interval_max = BT_GAP_ADV_SLOW_INT_MAX,
  };
}

bt_le_adv_param ConnectableFastAdvertisingParams() {
  return {
    .id = 0,
    .options = BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME,
    .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
    .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
  };
}

void InitBleAdvertising(const bt_le_adv_param& params) {
  auto err = bt_enable(nullptr);
  if (err) {
    LOG_ERR("Bluetooth init failed (err %d)", err);
    return;
  }

  LOG_INF("Bluetooth initialized");

  err = bt_le_adv_start(&params, ad, ARRAY_SIZE(ad), nullptr, 0);
  if (err) {
    LOG_ERR("Advertising failed to start (err %d)", err);
    return;
  }

  LOG_INF("Advertising successfully started");
}

void SetBatteryLevel(uint8_t level) {
  bt_gatt_bas_set_battery_level(level);
}