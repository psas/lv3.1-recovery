import can
import time

# Replace '/dev/ttyACM0' with the appropriate port for your SLCAN device (COM1, /dev/tty.usbmodem, etc) and set the desired bitrate (10, 20, 50, 100, 125, 250, 500, 800, 1000 kbps)
channel = "/dev/ttyACM1"
# bitrate = 500000
bitrate = 125000

# Configure the connection to the VulCAN
# bus = can.interface.Bus(channel=channel, bustype="slcan", bitrate=bitrate)
bus = can.interface.Bus(channel=channel, interface="slcan", bitrate=bitrate)

# Set a timeout for receiving messages (in seconds)
timeout = 86400 # as found value 10
end_time = time.time() + timeout

print(f"Listening for CAN messages for {timeout} seconds...")

# Receive CAN messages until timeout is reached
while time.time() < end_time:
    message = bus.recv(timeout=end_time - time.time())
    if message:
        print(f"Received message: {message}")

# Close the bus connection
bus.shutdown()
