import string
import os
import numpy

# setting file name to write to
global gpio_stream
gpio_stream = "gpio_sim.txt"


# initialize the file to all 0 values
def initialize():
    array = numpy.zeros((32), dtype=int)
    numpy.savetxt(gpio_stream, array, fmt='%d')


# changing a pin value
def changePin(pin, value):
    cur_state = numpy.loadtxt(gpio_stream, dtype=int)
    cur_state[pin - 1] = value
    numpy.savetxt(gpio_stream, cur_state, fmt='%d')


# behavior in pin mode
def pinMode(ri_split):
        valid_values = [0, 1]
        pin, value = int(ri_split[1]), int(ri_split[2])
        # checking for valid value
        if value in valid_values:
            print("[INFO] Changing pin {0} to {1}".format(pin, value))
            changePin(pin, value)
        else:
            print("[ERROR] Value must be 0 or 1")


# test scenarios--------
# locking the nosecone scenario
def lockedTest():
    print("[INFO] Running locking scenario")
    changePin(28, 1)
    changePin(27, 1)

# run a test scenario
def testMode(ri_split):
    valid_tests = {"lock" : lockedTest}
    test_mode = ri_split[1]
    # perform the test
    if test_mode in valid_tests:
        valid_tests[test_mode]()
    else: 
        print("[ERROR] Invalid Test Mode")

# exiting program
def exitMode(dummy_arg):
    print("[INFO] Exiting...")
    exit()


# the interface
def gatherInput():
    valid_commands = {"p" : pinMode, "t" : testMode, "exit" : exitMode}
    raw_input = input(">GPIO-SIM\n")
    ri_split = raw_input.split(" ")
    command = ri_split[0]
    # parse through the input command
    if command in valid_commands:
        valid_commands[command](ri_split)

    else:
        print("[ERROR] Invalid Command")


# main function
if __name__ == "__main__":
    initialize()
    while True:
        gatherInput()
