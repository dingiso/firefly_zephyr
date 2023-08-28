import asyncio
from bleak import BleakClient, BleakGATTCharacteristic

VOLTCRAFT_BLE_SERVICE_UUID = "0000fff0-0000-1000-8000-00805f9b34fb"

VOLTCRAFT_DEVICE_INFO_CHARACTERISTIC_UUID = "0000fff1-0000-1000-8000-00805f9b34fb"
VOLTCRAFT_CHARACTERISTIC_2_UUID = "0000fff2-0000-1000-8000-00805f9b34fb"
VOLTCRAFT_COMMAND_CHARACTERISTIC_UUID = "0000fff3-0000-1000-8000-00805f9b34fb"
VOLTCRAFT_NOTIFICATION_CHARACTERISTIC_UUID = "0000fff4-0000-1000-8000-00805f9b34fb"
VOLTCRAFT_CHARACTERISTIC_5_UUID = "0000fff5-0000-1000-8000-00805f9b34fb"


async def list_services_and_characteristics(mac: str):
    async with BleakClient(mac) as client:
        for handle, service in client.services.services.items():
            print(f"Service UUID = {service.uuid}, description: {service.description}")
            for characteristic in service.characteristics:
                print(f"  Characteristic UUID = {characteristic.uuid}, description: {characteristic.description}")


def notification_callback(sender: BleakGATTCharacteristic, data: bytearray):
    print(f"Received data: {data}")


async def main(mac):
    async with BleakClient(mac) as client:
        device_info = await client.read_gatt_char(VOLTCRAFT_DEVICE_INFO_CHARACTERISTIC_UUID)
        firmware_version_major = int.from_bytes(device_info[11:12], "little")
        firmware_version_minor = int.from_bytes(device_info[12:13], "little")
        hardware_version_major = int.from_bytes(device_info[13:14], "little")
        hardware_version_minor = int.from_bytes(device_info[14:15], "little")
        print(f'Firmware v{firmware_version_major}.{firmware_version_minor}, '
              f'hardware v{hardware_version_major}.{hardware_version_minor}')

        await client.start_notify(VOLTCRAFT_NOTIFICATION_CHARACTERISTIC_UUID, notification_callback)
        await client.write_gatt_char(VOLTCRAFT_COMMAND_CHARACTERISTIC_UUID, auth_with_pin_command())
        await asyncio.sleep(5)
        await client.write_gatt_char(VOLTCRAFT_COMMAND_CHARACTERISTIC_UUID, on_off_command(False))
        await asyncio.sleep(5)
        await client.write_gatt_char(VOLTCRAFT_COMMAND_CHARACTERISTIC_UUID, on_off_command(True))
        await asyncio.sleep(5)


def on_off_command(on: bool) -> bytearray:
    on_off = 0x01 if on else 0x00
    checksum = 0x01 + 0x03 + on_off
    return bytearray([0x0f, 0x06, 0x03, 0x00, on_off, 0x00, 0x00, checksum, 0xff, 0xff])


def auth_with_pin_command() -> bytearray:
    return bytearray([0x0f, 0x0c, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xff, 0xff])


asyncio.run(main("94:A9:A8:1B:54:02"))
