#pragma once
#include <cstdint>
#include <bluetooth/bluetooth.h>

bt_le_adv_param ConnectableSlowAdvertisingParams();
bt_le_adv_param ConnectableFastAdvertisingParams();

// Use helpers above to create params.
void InitBleAdvertising(const bt_le_adv_param& params);

void SetBatteryLevel(uint8_t level);


