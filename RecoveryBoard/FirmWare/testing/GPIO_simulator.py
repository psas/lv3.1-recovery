import string, os, numpy, time

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
    print("[INFO] Locking...")
    changePin(28, 1)
    changePin(27, 1)


def unlockedTest():
    print("[INFO] Unlocking...")
    changePin(28, 0)
    changePin(27, 0)


# avionics test scenario
def avionicsTest():
    print("[INFO] Running avionics scenario")
    time.sleep(2)
    lockedTest() # locking cone
    print("[INFO] Locked cone")

    time.sleep(5)
    # release drogue
    changePin(10, 1)
    print("[INFO] Drogue released")
    time.sleep(0.1)
    changePin(10, 0)
    # 1, 1 = locked - 1
    # 0, 1 = moving - 2
    # 0, 0 = open - 3
    # 1, 0 = def open - 4
    # from locked to moving
    changePin(28, 0)
    print("[INFO] Moving state")
    time.sleep(1)
    # moving to open
    changePin(27, 0)
    print("[INFO] Open state")
    time.sleep(1)
    # open to def open
    changePin(28, 1)
    print("[INFO] Open to Def Open state")
    time.sleep(5)
    # release main
    changePin(9, 1)
    print("[INFO] Main released")
    time.sleep(0.1)
    changePin(9, 0)


# run a test scenario
def testMode(ri_split):
    valid_tests = {"unlock": unlockedTest, "lock" : lockedTest, "av": avionicsTest}
    test_mode = ri_split[1]
    # perform the test
    if test_mode in valid_tests:
        valid_tests[test_mode]()
    else: 
        print("[ERROR] Invalid test mode")

# exiting program
def exitMode(dummy_arg):
    print("[INFO] Exiting...")
    exit()


def resetMode(dummy_arg):
    print("[INFO] Reset GPIO")
    initialize()


# the interface
def gatherInput():
    valid_commands = {"p" : pinMode, "r" : resetMode, "t" : testMode, "exit" : exitMode}
    raw_input = input(">GPIO-SIM\n")
    ri_split = raw_input.split(" ")
    command = ri_split[0]
    # parse through the input command
    if command in valid_commands:
        valid_commands[command](ri_split)

    else:
        print("[ERROR] Invalid command")


# main function
if __name__ == "__main__":
    initialize()
    while True:
        gatherInput()
