import time
import can

bitrate = 1e6  # 1Mbps

# Configure the connection to the VulCAN
bus = can.interface.Bus(channel="can0", interface="socketcan", bitrate=bitrate)


def drogue_should_acknowledge_deployment_msg():
    """
    Send a message to the drogue parachute board telling it to deploy.
    Then, receive a message every half a second for two seconds (to avoid superfluous messages),
    and check if it's the drogue board acknowledging deployment. If not, fail the test.
    """
    deploy_drogue_msg = can.Message(
        arbitration_id=0x100, data=[1], is_extended_id=False
    )
    bus.send(deploy_drogue_msg)
    acknowledged = 0
    time_start = time.time()

    while time.time() - time_start < 2.0:
        msg = bus.recv(timeout=0.5)
        if msg is not None and msg.arbitration_id == 0x101:
            acknowledged = 1

    assert acknowledged > 0


def main_should_acknowledge_deployment_msg():
    """
    Send a message to the main parachute board telling it to deploy.
    Then, receive a message every half a second for two seconds (to avoid superfluous messages),
    and check if it's the main board acknowledging deployment. If not, fail the test.
    """
    deploy_main_msg = can.Message(arbitration_id=0x200, data=[1], is_extended_id=False)
    bus.send(deploy_main_msg)
    acknowledged = 0
    time_start = time.time()

    while time.time() - time_start < 2.0:
        msg = bus.recv(timeout=0.5)
        if msg is not None and msg.arbitration_id == 0x201:
            acknowledged = 1

    assert acknowledged > 0


drogue_should_acknowledge_deployment_msg()
main_should_acknowledge_deployment_msg()
