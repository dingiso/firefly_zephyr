#pragma once
#include <cstdint>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>

// Service UUID 8ec87060-8865-4eca-82e0-2ea8e45e8221
// This is randomly-generated vendor-specific (i.e. Ostranna's) UUID.
// It's expected to be present on all bluetooth Ostranna devices
// (but set of characteristic can vary depending on the device purpose).
extern bt_uuid_128 firefly_service_uuid;

bt_le_adv_param ConnectableSlowAdvertisingParams();
bt_le_adv_param ConnectableFastAdvertisingParams();

// Use helpers above to create params.
void InitBleAdvertising(const bt_le_adv_param& params);

void SetBatteryLevel(uint8_t level);


