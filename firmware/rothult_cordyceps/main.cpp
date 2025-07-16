#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <drivers/gpio.h>
#include <logging/log.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#define LOG_LEVEL LOG_LEVEL_INFO
#define LOG_MODULE_NAME smart_lock
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

// port
/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_HAS_NODE(LED0_NODE)
#define LED0 DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN DT_GPIO_PIN(LED0_NODE, gpios)
#if DT_PHA_HAS_CELL(LED0_NODE, gpios, flags)
#define FLAGS DT_GPIO_FLAGS(LED0_NODE, gpios)
#endif
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0 ""
#define PIN 0
#endif

#ifndef LED0_FLAGS
#define LED0_FLAGS 0
#endif

/* The devicetree node identifier for the "led0" alias. */
#define LED1_NODE DT_ALIAS(led1)

#if DT_HAS_NODE(LED1_NODE)
#define LED1 DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN DT_GPIO_PIN(LED1_NODE, gpios)
#if DT_PHA_HAS_CELL(LED1_NODE, gpios, flags)
#define FLAGS DT_GPIO_FLAGS(LED1_NODE, gpios)
#endif
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED1 ""
#define PIN 0
#endif

#ifndef LED1_FLAGS
#define LED1_FLAGS 0
#endif

#ifndef DT_ALIAS_SW0_GPIOS_FLAGS
#define DT_ALIAS_SW0_GPIOS_FLAGS 0
#endif
// end

static struct device* led1;
static struct device* power_en;
static struct device* button_sw1;

// constexpr gpio_dt_spec power_en = GPIO_DT_SPEC_GET(DT_NODELABEL(power_en), gpios);
// constexpr gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_NODELABEL(led1), gpios);
// constexpr gpio_dt_spec button_sw1 = GPIO_DT_SPEC_GET(DT_NODELABEL(button_sw1), gpios);

// constexpr const device* uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart1));

bool power_enabled = false;
bool lock_closed = false;
static struct gpio_callback button_callback_data;

void ActuatePowerEnabled() {
  gpio_pin_set(power_en, PIN, (int)power_enabled);
  gpio_pin_set(led1, PIN, (int)power_enabled);
}

// Cordyceps (Rothult) service, UUID  ab12c725-0a0e-4c06-9968-2b9fff64b504
bt_uuid_128 cordyceps_service_uuid =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xab12c725, 0x0a0e, 0x4c06, 0x9968, 0x2b9fff64b504));

static const bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_UUID128_SOME, cordyceps_service_uuid.val, sizeof(cordyceps_service_uuid.val))};

bt_le_adv_param* ConnectableFastAdvertisingParams() {
  // return {
  //     .id = 0,
  //     .options = BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME,
  //     .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
  //     .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
  // };
  return BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME, BT_GAP_ADV_FAST_INT_MIN_2,
                         BT_GAP_ADV_FAST_INT_MAX_2, NULL);
}

// Power on characteristic, UUID 0eb094be-5997-492e-a257-9242c57b1586
struct bt_uuid_128 power_on_characteristic_uuid =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x0eb094be, 0x5997, 0x492e, 0xa257, 0x9242c57b1586));

ssize_t ReadPowerOn(struct bt_conn* conn, const struct bt_gatt_attr* attr, void* buf, uint16_t len, uint16_t offset) {
  uint8_t as_uint8 = power_enabled ? 1 : 0;
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &as_uint8, sizeof(as_uint8));
}

ssize_t WritePowerOn(struct bt_conn* conn, const struct bt_gatt_attr* attr, const void* buf, uint16_t len,
                     uint16_t offset, uint8_t flags) {
  uint8_t enabled = (uint8_t*)buf[0];
  power_enabled = enabled != 0;
  ActuatePowerEnabled();

  return len;
}

// Open/close characteristic, UUID 34e615dd-ad8d-43d3-a5ab-f72db0931e8f
struct bt_uuid_128 open_close_characteristic_uuid =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x34e615dd, 0xad8d, 0x43d3, 0xa5ab, 0xf72db0931e8f));

ssize_t ReadOpenClose(struct bt_conn* conn, const struct bt_gatt_attr* attr, void* buf, uint16_t len, uint16_t offset) {
  uint8_t as_uint8 = lock_closed ? 1 : 0;
  return bt_gatt_attr_read(conn, attr, buf, len, offset, &as_uint8, sizeof(as_uint8));
}

ssize_t WriteOpenClose(struct bt_conn* conn, const struct bt_gatt_attr* attr, const void* buf, uint16_t len,
                       uint16_t offset, uint8_t flags) {
  bool lock_closed_next = *reinterpret_cast<const uint8_t*>(buf) != 0;
  if (lock_closed_next != lock_closed) {
    if (lock_closed_next) {
      // uart_poll_out(uart_dev, 'C');
      printk("Door Closed!\n");
    } else {
      // uart_poll_out(uart_dev, 'O');
      printk("Door Opened!\n");
    }
    lock_closed = lock_closed_next;
  }
  return len;
}

BT_GATT_SERVICE_DEFINE(
    power_meter_service, BT_GATT_PRIMARY_SERVICE(&cordyceps_service_uuid),
    BT_GATT_CHARACTERISTIC(&power_on_characteristic_uuid.uuid, BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, ReadPowerOn, WritePowerOn, NULL),
    BT_GATT_CUD("Power On", BT_GATT_PERM_READ),
    BT_GATT_CHARACTERISTIC(&open_close_characteristic_uuid.uuid, BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, ReadOpenClose, WriteOpenClose, NULL),
    BT_GATT_CUD("Lock is closed", BT_GATT_PERM_READ), );

void InitBleAdvertising(const bt_le_adv_param* params) {
  auto err = bt_enable(NULL);
  if (err) {
    LOG_ERR("Bluetooth init failed (err %d)", err);
    return;
  }

  LOG_INF("Bluetooth initialized");

  err = bt_le_adv_start(params, ad, ARRAY_SIZE(ad), NULL, 0);
  if (err) {
    LOG_ERR("Advertising failed to start (err %d)", err);
    return;
  }

  LOG_INF("Advertising successfully started");
}

void OnButtonPress(const struct device* gpio, struct gpio_callback* cb, uint32_t pins) {
  power_enabled = !power_enabled;
  ActuatePowerEnabled();
}

void main(void) {
  int ret;

  power_en = device_get_binding(LED0);
  if (!power_en) {
    LOG_ERR("Failed to get binding for power_en");
    return;
  }
  led1 = device_get_binding(LED1);
  if (!led1) {
    LOG_ERR("Failed to get binding for led1");
    return;
  }
  button_sw1 = device_get_binding(DT_ALIAS_SW0_GPIOS_CONTROLLER);
  if (!button_sw1) {
    LOG_ERR("Failed to get binding for button_sw1");
    return;
  }

  InitBleAdvertising(ConnectableFastAdvertisingParams());

  ret = gpio_pin_configure(power_en, PIN, GPIO_OUTPUT_ACTIVE | LED0_FLAGS);
  if (ret < 0) {
    LOG_ERR("Failed to configure power_en pin (err %d)", ret);
    return;
  }
  ret = gpio_pin_configure(led1, PIN, GPIO_OUTPUT_ACTIVE | LED1_FLAGS);
  if (ret < 0) {
    LOG_ERR("Failed to configure led1 pin (err %d)", ret);
    return;
  }
  ret = gpio_pin_configure(button_sw1, DT_ALIAS_SW0_GPIOS_PIN, DT_ALIAS_SW0_GPIOS_FLAGS | GPIO_INPUT);
  if (ret != 0) {
    LOG_ERR("Error %d: failed to configure pin %d '%s'\n", ret, DT_ALIAS_SW0_GPIOS_PIN, DT_ALIAS_SW0_LABEL);
    return;
  }

  ret = gpio_pin_interrupt_configure(button_sw1, DT_ALIAS_SW0_GPIOS_PIN, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret != 0) {
    LOG_ERR("Error %d: failed to configure interrupt on pin %d '%s'\n", ret, DT_ALIAS_SW0_GPIOS_PIN,
            DT_ALIAS_SW0_LABEL);
    return;
  }

  gpio_init_callback(&button_callback_data, OnButtonPress, BIT(DT_ALIAS_SW0_GPIOS_PIN));
  gpio_add_callback(button_sw1, &button_callback_data);

  for (int i = 0; i < 3; ++i) {
    gpio_pin_set(led1, PIN, (int)true);
    k_sleep(K_MSEC(200));
    gpio_pin_set(led1, PIN, (int)false);
    k_sleep(K_MSEC(500));
  }

  ActuatePowerEnabled();

  while (true) {
    k_sleep(K_MSEC(500));
    LOG_INF("Awaiting command...");
  }

  return 0;
}
