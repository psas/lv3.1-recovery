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
    prog = "VulCAN Tester",
    description="Send or Receive messages via CAN using a VulCAN adapter",
)

_ = parser.add_argument("--device", "-d", required=True, help="Path to VulCAN")

group = parser.add_mutually_exclusive_group(required=True)
_ = group.add_argument(
    "--send", "-s", action="store_true", help="Send a basic CAN message"
)
_ = group.add_argument(
    "--receive", "-r", action="store_true", help="Wait for a CAN message for n seconds"
)

args = parser.parse_args()

device = cast(str, args.device)
device = str(device)
send = cast(bool, args.send)
send = bool(send)
receive = cast(bool, args.receive)
receive = bool(receive)


def main():
    logger.info("Connecting to VulCAN")
    bitrate = 500000

    # Configure the connection to the VulCAN
    bus = can.interface.Bus(channel=device, interface="slcan", bitrate=bitrate)

    if send:
        logger.info("Sending CAN message")
        # Define a CAN message with an arbitrary ID and data bytes
        message_id = 0x123
        data_bytes = [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88]
        message = can.Message(
            arbitration_id=message_id, data=data_bytes, is_extended_id=False
        )

        # Send the CAN message
        bus.send(message)
        logger.info("Sent message: %s", message)

        # Close the bus connection
        bus.shutdown()

    if receive:
        # Set a timeout for receiving messages (in seconds)
        timeout = 30
        end_time = time.time() + timeout

        logger.info("Listening for CAN messages for %s seconds", timeout)

        # Receive CAN messages until timeout is reached
        while time.time() < end_time:
            message = bus.recv(timeout=end_time - time.time())
            if message:
                logger.info("Received message: %s", message)

        # Close the bus connection
        bus.shutdown()


if __name__ == "__main__":
    main()
