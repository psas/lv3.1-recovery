import can
import time

# Replace '/dev/ttyACM0' with the appropriate port for your SLCAN device (COM1, /dev/tty.usbmodem, etc) and set the desired bitrate (10, 20, 50, 100, 125, 250, 500, 800, 1000 kbps)
channel = "/dev/ttyACM1"
bitrate = 125000
led_message_id = 0x10

# Configure the connection to the VulCAN
bus = can.interface.Bus(channel=channel, interface="slcan", bitrate=bitrate)

## Define a CAN message with an arbitrary ID and data bytes
#message_id = 0x123
#data_bytes = [0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88]
#message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=False)
#
## Send the CAN message
#bus.send(message)
#print(f"Sent message: {message}")


# (2)

# Define a CAN message with an arbitrary ID and data bytes
message_id = 0x12345
data_bytes = [0x0c, 0x18]
message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=False)

# Send the CAN message
bus.send(message)
print(f"Sent message: {message}")


# (3)

# Define a CAN message with an arbitrary ID and data bytes
message_id = led_message_id
data_bytes = [0x11]
message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=False)

# Send the CAN message
bus.send(message)
print(f"Sent message: {message}")


loop_count = 1
while loop_count < 10:

  message_id = led_message_id

  data_bytes = [0x00]
  message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=False)
  bus.send(message)
  print(f"Sent message: {message}")
  time.sleep(1/4)

  data_bytes = [0x01]
  message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=False)
  bus.send(message)
  print(f"Sent message: {message}")
  time.sleep(1/4)

  message_id = 0x12345

  data_bytes = [0x0c, 0x18]
  message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=True)
  bus.send(message)
  print(f"Sent message: {message}")
  time.sleep(1/4)

  data_bytes = [0x00, 0x06]
  message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=True)
  bus.send(message)
  print(f"Sent message: {message}")
  time.sleep(1/4)

  message_id = 0x700

  data_bytes = [0x55, 0x44, 0x55, 0x44, 0x55, 0x44, 0x55, 0x44]
  message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=True)
  bus.send(message)
  print(f"Sent message: {message}")
  time.sleep(1/4)

  message_id = 0x710

  data_bytes = [0x55, 0x44, 0x55, 0x44, 0x55, 0x44, 0x55, 0x44]
  message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=True)
  bus.send(message)
  print(f"Sent message: {message}")
  time.sleep(1/4)

  message_id = 0x720

  data_bytes = [0x55, 0x44, 0x55, 0x44, 0x55, 0x44, 0x55, 0x44]
  message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=True)
  bus.send(message)
  print(f"Sent message: {message}")
  time.sleep(1/4)

  loop_count += 1

# Close the bus connection
bus.shutdown()
