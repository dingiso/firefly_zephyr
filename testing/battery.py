import datetime
import time

from pc_ble_driver_py import config
config.__conn_ic_id__ = 'NRF52'

from pc_ble_driver_py.ble_adapter import BLEAdapter
from pc_ble_driver_py.ble_driver import BLEAdvData, BLEUUIDBase, BLEUUID, BLEDriver, BLEGapScanParams
from pc_ble_driver_py.ble_driver import BLEEnableParams, BLEGattStatusCode
from pc_ble_driver_py.ble_driver import driver as swig_driver
from pc_ble_driver_py.ble_driver_types import list_to_char_array, uint8_array_to_list
from pc_ble_driver_py.observers import BLEAdapterObserver, BLEDriverObserver

import firebase_admin
from firebase_admin import credentials
from firebase_admin import firestore

CONNECTIONS = 1

cred = credentials.ApplicationDefault()
firebase_admin.initialize_app(cred)
db = firestore.client()

class BatteryCollector(BLEDriverObserver, BLEAdapterObserver):
  def __init__(self, adapter):
    super(BatteryCollector, self).__init__()
    self.adapter = adapter
    self.connection = None
    self.adapter.observer_register(self)
    self.adapter.driver.observer_register(self)
    # self.adapter.default_mtu = 250

  def open(self):
    self.adapter.driver.open()
    self.adapter.driver.ble_enable()

  def close(self):
    self.adapter.driver.close()

  def connect_and_discover(self):
    scan_duration = 50
    params = BLEGapScanParams(interval_ms=200, window_ms=150, timeout_s=scan_duration)

    self.adapter.driver.ble_gap_scan_start(scan_params=params)
    while self.connection is None:
      time.sleep(1)
    new_conn = self.connection
    self.adapter.service_discovery(new_conn)

    self.adapter.enable_notification(new_conn, BLEUUID(BLEUUID.Standard.battery_level))
    return new_conn


  def on_gap_evt_connected(self, ble_driver, conn_handle, peer_addr, role, conn_params):
    print("New connection: {}".format(conn_handle))
    self.connection = conn_handle

  def on_gap_evt_disconnected(self, ble_driver, conn_handle, reason):
    print("Disconnected: {} {}".format(conn_handle, reason))

  def on_gap_evt_adv_report(self, ble_driver, conn_handle, peer_addr, rssi, adv_type, adv_data):
    if BLEAdvData.Types.complete_local_name in adv_data.records:
      dev_name_list = adv_data.records[BLEAdvData.Types.complete_local_name]

    elif BLEAdvData.Types.short_local_name in adv_data.records:
      dev_name_list = adv_data.records[BLEAdvData.Types.short_local_name]

    else:
      return

    dev_name = "".join(chr(e) for e in dev_name_list)
    address_string = "".join("{0:02X}".format(b) for b in peer_addr.addr)
    print(
        "Received advertisment report, address: 0x{}, device_name: {}".format(
            address_string, dev_name
        )
    )

    if dev_name == 'Firefly v.1':
      print("connecting to %s" % peer_addr.addr)
      self.adapter.connect(peer_addr)

  def on_notification(self, ble_adapter, conn_handle, uuid, data):
    if len(data) > 32:
      data = "({}...)".format(data[0:10])
    print("Connection: {}, {} = {}".format(conn_handle, uuid, data))
    db.collection(u'measurements').add({
        u'time': datetime.datetime.now(),
        u'value': data[0]
    })


def main():
  print('Possible dongles are at:')
  descs = BLEDriver.enum_serial_ports()
  for _, d in enumerate(descs):
    print('  {}: {} Serial Number {}'.format(d.port, d.manufacturer,
                                             d.serial_number))
  driver = BLEDriver(
      serial_port='COM8',
      auto_flash=True,
  )
  adapter = BLEAdapter(driver)
  collector = BatteryCollector(adapter)
  collector.open()
  conn = collector.connect_and_discover()

  if conn is not None:
    while True:
      pass

  collector.close()

if __name__ == '__main__':
  main()