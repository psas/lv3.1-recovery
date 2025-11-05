import can
import time

# Replace '/dev/ttyACM0' with the appropriate port for your SLCAN device (COM1, /dev/tty.usbmodem, etc) and set the desired bitrate (10, 20, 50, 100, 125, 250, 500, 800, 1000 kbps)
channel = "/dev/ttyACM1"
bitrate = 125000
led_message_id = 0x10
unlock_drogue_msg_id = 0x100
unlock_main_msg_id = 0x200

# Configure the connection to the VulCAN
bus = can.interface.Bus(channel=channel, interface="slcan", bitrate=bitrate)

loop_count = 1
while loop_count <= 1:

  message_id = unlock_drogue_msg_id
  data_bytes = [0x01]
  message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=True)
  bus.send(message)
  print(f"Sent message with id {message_id}: {message}")
  time.sleep(1/2)

  message_id = 0x710
  data_bytes = [0x55, 0x44, 0x55, 0x44]
  message = can.Message(arbitration_id=message_id, data=data_bytes, is_extended_id=True)
  bus.send(message)
  print(f"Sent message with id {message_id}: {message}")
  time.sleep(3/2)

  loop_count += 1

# Close the bus connection
bus.shutdown()
