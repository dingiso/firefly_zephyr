import argparse
import os
from pathlib import Path

import serial

from pw_hdlc.rpc import HdlcRpcClient, default_channels

PROTO = Path(os.path.dirname(os.path.realpath(__file__)), 'test.proto')


def script(device: str) -> None:
    ser = serial.Serial(device, 115200, timeout=0.01)
    client = HdlcRpcClient(
        lambda: ser.read(4096), [PROTO], default_channels(ser.write)
    )

    echo_service = client.rpcs().common.rpc.EchoService
    status, payload = echo_service.Echo(age=13)

    if status.ok():
        print('The payload was', payload)
    else:
        print('Uh oh, this RPC returned', status)

def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        '--device', '-d', default='/dev/ttyACM0', help='serial device to use'
    )
    script(**vars(parser.parse_args()))

if __name__ == '__main__':
    main()