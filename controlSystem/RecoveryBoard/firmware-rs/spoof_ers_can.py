"""
This script uses the Vulcan to spoof the other two parachute boards when you only have one board to use.
It's purpose is to test that the sender board is functioning properly.
Make sure to pass in the path to your Vulcan with the `-d` flag when invoking this script.
You should see that the Vulcan is sending good status messages over the CAN bus that match both drogue and main parachute board heartbeat IDs.
* Requires python-can v4.5, pySerial v3.5 *
"""

import can
import time
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

    status_buf = [
        2,  # Ring locked
        100,  # Battery voltage in 0.1 volts (> 9.9v is good)
        1,  # Batt_ok == True
        1,  # Shore power Off == true
        1,  # Received sender message in last two seconds
        1,  # Board is ready to release parachute
        0,  # unused
        0,  # unused
    ]

    while True:
        message_id = 0x710
        msg = can.Message(
            arbitration_id=message_id, data=status_buf, is_extended_id=False
        )
        bus.send(msg)
        message2_id = 0x720
        msg2 = can.Message(
            arbitration_id=message2_id, data=status_buf, is_extended_id=False
        )
        bus.send(msg2)
        logger.info("Sent messages: %s %s", msg, msg2)
        time.sleep(2)


if __name__ == "__main__":
    main()
