import can
import time

# Replace '/dev/ttyACM0' with the appropriate port for your SLCAN device (COM1,
# /dev/tty.usbmodem, etc) and set the desired bitrate (10, 20, 50, 100, 125,
# 250, 500, 800, 1000 kbps)
channel = "/dev/ttyACM1"
bitrate = 125000
led_message_id = 0x10

# Configure the connection to the VulCAN
bus = can.interface.Bus(channel=channel, interface="slcan", bitrate=bitrate)

print(f"Mocking Telemetrum Sender heartbeat message, sending at one Hz . . .")

loop_count = 1
num=1
denom=1
while loop_count < 86400:

  message_id = 0x700

  # data_bytes = [0x55, 0x44, 0x55, 0x44, 0x55, 0x44, 0x55, 0x44]
  data_bytes = [0x55]
  message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=True)
  bus.send(message)
  print(f"Telemetrum mock sent message: {message}")
  time.sleep(num/denom)

  loop_count += 1

# Close the bus connection
bus.shutdown()
