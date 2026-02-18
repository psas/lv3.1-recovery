import can
import time

# Replace '/dev/ttyACM0' with the appropriate port for your SLCAN device (COM1, /dev/tty.usbmodem, etc) and set the desired bitrate (10, 20, 50, 100, 125, 250, 500, 800, 1000 kbps)

# channel = "/dev/ttyACM1"
channel = "/dev/ttyACM2"
# bitrate = 125000
bitrate = 1000000

# Configure the connection to the VulCAN
# bus = can.interface.Bus(channel=channel, bustype="slcan", bitrate=bitrate)
bus = can.interface.Bus(channel=channel, interface="slcan", bitrate=bitrate)

# Set a timeout for receiving messages (in seconds)
timeout = 86400 # as found value 10
end_time = time.time() + timeout

def send_frame(iter, id):
    message_id = id
    data_bytes = [0x00, 0x00]
    # message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=True)
    message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=False)
    bus.send(message)
    print(f"Sent message: {message}")

print(f"Listening for CAN messages for {timeout} seconds...")

i = 0
toggle = 0

# Receive CAN messages until timeout is reached
while time.time() < end_time:
    message = bus.recv(timeout=end_time - time.time())
    if message:
        print(f"Received message: {message}")

    i = i + 1
    # print(f"- MARK - {i}")
    if i % 10 == 0:
        if toggle == 0:
            toggle = 1
        else:
            toggle = 0
    # Compare `toggle` with value other than 0 and 1 to disable call to `send_frame`:
    if toggle == 2:
        send_frame(i, 0x700)
    time.sleep(1/10)

# Close the bus connection
bus.shutdown()
