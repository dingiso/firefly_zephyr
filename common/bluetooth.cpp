#include "bluetooth.h"

#include <bluetooth/bluetooth.h>
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

void InitBleAdvertising() {
  auto err = bt_enable(nullptr);
  if (err) {
    LOG_ERR("Bluetooth init failed (err %d)", err);
    return;
  }

  LOG_INF("Bluetooth initialized");

  bt_le_adv_param advertising_params;
  advertising_params.id = 0;
  advertising_params.options = BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME;
  advertising_params.interval_min = BT_GAP_ADV_SLOW_INT_MIN;
  advertising_params.interval_max = BT_GAP_ADV_SLOW_INT_MAX;

  err = bt_le_adv_start(&advertising_params, ad, ARRAY_SIZE(ad), nullptr, 0);
  if (err) {
    LOG_ERR("Advertising failed to start (err %d)", err);
    return;
  }

  LOG_INF("Advertising successfully started");
}

void SetBatteryLevel(uint8_t level) {
  bt_gatt_bas_set_battery_level(level);
}