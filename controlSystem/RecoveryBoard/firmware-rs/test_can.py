# Requires python-can v4.5, pySerial v3.5
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

group = parser.add_mutually_exclusive_group(required=True)
_ = group.add_argument(
    "--send", "-s", action="store_true", help="Send a basic CAN message"
)
_ = group.add_argument(
    "--receive", "-r", action="store_true", help="Await a CAN message"
)
_ = group.add_argument(
    "--pingpong",
    "-p",
    action="store_true",
    help="Await a CAN message, then echo it back to the bus",
)

args = parser.parse_args()

# Ensure proper arg types
device = cast(str, args.device)
device = str(device)
send = cast(bool, args.send)
send = bool(send)
receive = cast(bool, args.receive)
receive = bool(receive)
pingpong = cast(bool, args.pingpong)
pingpong = bool(pingpong)


def main():
    logger.info("Connecting to VulCAN")
    bitrate = 1e6

    # Configure the connection to the VulCAN
    bus = can.interface.Bus(channel="can0", interface="socketcan", bitrate=bitrate)

    if send:
        logger.info("Sending CAN message")
        # Define a CAN message with an arbitrary ID and data bytes
        message_id = 0x101
        data_bytes = [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88]
        message = can.Message(
            arbitration_id=message_id, data=data_bytes, is_extended_id=False
        )
        bus.send(message)
        logger.info("Sent message: %s", message)
        message_id = 0x201
        data_bytes = [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88]
        message = can.Message(
            arbitration_id=message_id, data=data_bytes, is_extended_id=False
        )
        bus.send(message)
        logger.info("Sent message: %s", message)

    if receive:
        logger.info("Listening for CAN messages")
        while True:
            message = bus.recv(timeout=None)
            logger.info("Received message: %s", message)

    if pingpong:
        logger.info("Echoing CAN messages")
        while True:
            message = bus.recv(timeout=None)
            if message:
                bus.send(message)
                logger.info("Echoed message: %s", message)


if __name__ == "__main__":
    main()
