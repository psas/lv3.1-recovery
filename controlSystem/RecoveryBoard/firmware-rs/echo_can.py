"""
This script is used to test that your can bus is set up properly.
With a VulCAN plugged in, run this script in one terminal,
making sure to pass in the proper path to the VulCAN with the `-d` flag.
Then have the ERS board or Nucleo run the `echo_can` task in another.
You should see the two boards echoing CAN messages back and forth.
* Requires python-can v4.5, pySerial v3.5 *
"""

import can
import argparse
import logging
from typing import cast

logger = logging.getLogger(__name__)
logging.basicConfig(
    format="%(asctime)s - [%(levelname)s]: %(message)s",
    datefmt="%H:%M:%S",
    level=logging.INFO,
)

parser = argparse.ArgumentParser(
    prog="VulCAN Tester",
    description="Send or Receive messages via CAN using a VulCAN adapter",
)

_ = parser.add_argument("--device", "-d", required=True, help="Path to VulCAN")

args = parser.parse_args()

# Ensure proper arg types
device = cast(str, args.device)
device = str(device)


def main():
    logger.info("Connecting to VulCAN")
    bitrate = 1e6

    # Configure the connection to the VulCAN
    bus = can.interface.Bus(channel="can0", interface="socketcan", bitrate=bitrate)

    logger.info("Echoing CAN messages")
    while True:
        message = bus.recv(timeout=None)
        if message:
            bus.send(message)
            logger.info("Echoed message: %s", message)


if __name__ == "__main__":
    main()
