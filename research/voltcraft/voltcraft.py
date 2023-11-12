from abc import abstractmethod
import asyncio
from bleak import BleakClient, BleakGATTCharacteristic

VOLTCRAFT_BLE_SERVICE_UUID = "0000fff0-0000-1000-8000-00805f9b34fb"

VOLTCRAFT_DEVICE_INFO_CHARACTERISTIC_UUID  = "0000fff1-0000-1000-8000-00805f9b34fb"
VOLTCRAFT_CHARACTERISTIC_2_UUID            = "0000fff2-0000-1000-8000-00805f9b34fb"
VOLTCRAFT_COMMAND_CHARACTERISTIC_UUID      = "0000fff3-0000-1000-8000-00805f9b34fb"
VOLTCRAFT_NOTIFICATION_CHARACTERISTIC_UUID = "0000fff4-0000-1000-8000-00805f9b34fb"
VOLTCRAFT_CHARACTERISTIC_5_UUID            = "0000fff5-0000-1000-8000-00805f9b34fb"

class Command:
  @abstractmethod
  def command_body(self) -> bytearray:
    pass

  @abstractmethod
  def digest_response(self, response: bytearray):
    pass

  def full_command(self) -> bytearray:
    cmd_len = len(self.command_body()) + 1  # +1 for checksum
    checksum = (0x01 + sum(self.command_body())) & 0xff
    return bytearray([0x0f, cmd_len]) + self.command_body() + bytearray([checksum, 0xff, 0xff])

class VoltcraftSmartSocket:
  def __init__(self, client: BleakClient):
    self.client = client
    self.subscribed = False
    self.responses = asyncio.Queue()

  async def send_command(self, command: Command):
    if not self.subscribed:
      await self.client.start_notify(VOLTCRAFT_NOTIFICATION_CHARACTERISTIC_UUID, lambda _, data: self._notification_callback(data))
      self.subscribed = True
    await self.client.write_gatt_char(VOLTCRAFT_COMMAND_CHARACTERISTIC_UUID, command.full_command())
    command.digest_response(self._decode_reply(await self.responses.get()))

  async def print_device_info(self):
    device_info = await self.client.read_gatt_char(VOLTCRAFT_DEVICE_INFO_CHARACTERISTIC_UUID)
    firmware_version_major = int.from_bytes(device_info[11:12], "little")
    firmware_version_minor = int.from_bytes(device_info[12:13], "little")
    hardware_version_major = int.from_bytes(device_info[13:14], "little")
    hardware_version_minor = int.from_bytes(device_info[14:15], "little")
    print(f'Firmware v{firmware_version_major}.{firmware_version_minor}, '
          f'hardware v{hardware_version_major}.{hardware_version_minor}')


  def _notification_callback(self, data: bytearray):
    self.responses.put_nowait(data)

  def _decode_reply(self, reply: bytearray) -> bytearray:
    if len(reply) < 5:
        raise ValueError("Invalid notification format, too short")
    # Reply to "Get status" command (which has 0x0400 code) is missign 0xffff end sequence
    # for some reason. So hack it back.
    if reply[2:4] == bytearray([0x04, 0x00]):
      reply = reply + bytearray([0xff, 0xff])
    if not (reply[0] == 0x0f):
        raise ValueError("Invalid notification format, unexpected start sequence")
    if not (reply[-1] == 0xff and reply[-2] == 0xff):
        raise ValueError("Invalid notification format, unexpected end sequence")
    if reply[1] != len(reply) - 4:
        raise ValueError("Invalid notification format, length byte mismatch")
    content = reply[2:-3]
    checksum = reply[-3]
    if checksum != (0x01 + sum(content)) & 0xff:
        raise ValueError("Invalid notification format, checksum mismatch")
    return content

class OnOffCommand(Command):
  def __init__(self, on: bool):
    self.on = on

  def command_body(self) -> bytearray:
    return bytearray([0x03, 0x00, 0x01 if self.on else 0x00, 0x00, 0x00])

  def digest_response(self, response: bytearray):
    if response != bytearray([0x03, 0x00, 0x00]):
      raise ValueError(f"Unexpected response: {response}")


class AuthCommand(Command):
  def __init__(self, pin: bytearray = bytearray([0x00, 0x00, 0x00, 0x00])):
    self.pin = pin

  def command_body(self) -> bytearray:
    return bytearray([0x17, 0x00, 0x00]) + self.pin + bytearray([0x00, 0x00, 0x00, 0x00])

  def digest_response(self, response: bytearray):
    if len(response) != 5:
      raise ValueError(f"Invalid response length: {response}")

    if response[2] == 0x01:
      raise ValueError(f"Authentication failed: {response}")

    if response != bytearray([0x17, 0x00, 0x00, 0x00, 0x00]):
      raise ValueError(f"Unexpected response: {response}")

class GetStatusCommand(Command):
  def __init__(self):
    self.powered_on = False
    self.power_watt = 0.0
    self.voltage = 0
    self.current_ampere = 0.0
    self.frequency_hz = 0
    self.power_factor = 0.0

  def command_body(self) -> bytearray:
    return bytearray([0x04, 0x00, 0x00])

  def digest_response(self, response: bytearray):
    if len(response) != 16:
      raise ValueError(f"Invalid response length: {response}")

    if response[0:2] != bytearray([0x04, 0x00]) or response[10:] != bytearray([0x00, 0x00, 0x00, 0x00, 0x00, 0x00]):
      raise ValueError(f"Unexpected response: {response}")

    self.powered_on = response[2] == 0x01
    self.power_watt = int.from_bytes(response[3:6], "big") / 1000
    self.voltage = int.from_bytes(response[6:7], "big")
    self.current_ampere = int.from_bytes(response[7:9], "big") / 1000
    self.frequency_hz = int.from_bytes(response[9:10], "big")
    self.power_factor = self.power_watt / (self.voltage * self.current_ampere) if self.current_ampere > 0.0 else 0.0
