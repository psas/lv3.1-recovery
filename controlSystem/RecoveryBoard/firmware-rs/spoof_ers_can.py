# Requires python-can v4.5, pySerial v3.5
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

    while True:
        message_id = 0x710
        data_bytes = [0x01]
        msg = can.Message(
            arbitration_id=message_id, data=data_bytes, is_extended_id=False
        )
        bus.send(msg)
        message2_id = 0x720
        data_bytes2 = [0x01]
        msg2 = can.Message(
            arbitration_id=message2_id, data=data_bytes2, is_extended_id=False
        )
        bus.send(msg2)
        logger.info("Sent messages: %s %s", msg, msg2)
        time.sleep(2)


if __name__ == "__main__":
    main()
