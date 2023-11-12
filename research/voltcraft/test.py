import asyncio
from bleak import BleakClient
from voltcraft import VoltcraftSmartSocket, OnOffCommand, AuthCommand, GetStatusCommand

async def list_services_and_characteristics(mac: str):
    async with BleakClient(mac) as client:
        for _, service in client.services.services.items():
            print(f"Service UUID = {service.uuid}, description: {service.description}")
            for characteristic in service.characteristics:
                print(f"  Characteristic UUID = {characteristic.uuid}, description: {characteristic.description}")

async def main(mac):
    async with BleakClient(mac, timeout=300, winrt={'use_cached_services': False}) as client:
        voltcraft = VoltcraftSmartSocket(client)
        await voltcraft.print_device_info()
        await voltcraft.send_command(AuthCommand(bytearray([0x00, 0x00, 0x00, 0x00])))
        await voltcraft.send_command(OnOffCommand(False))
        await asyncio.sleep(2)

        c = GetStatusCommand()
        await voltcraft.send_command(c)
        print(f"Powered on: {c.powered_on}, power: {c.power_watt}, current: {c.current_ampere}, volt: {c.voltage}, power factor: {c.power_factor}, frequency: {c.frequency_hz}")

        await asyncio.sleep(1)

        await voltcraft.send_command(OnOffCommand(True))
        await asyncio.sleep(20)

        c = GetStatusCommand()
        await voltcraft.send_command(c)
        print(f"Powered on: {c.powered_on}, power: {c.power_watt}, current: {c.current_ampere}, volt: {c.voltage}, power factor: {c.power_factor}, frequency: {c.frequency_hz}")


asyncio.run(main("94:A9:A8:1B:54:02"))
