> [!CAUTION]
> Everything is a WIP, Information subject to change and is not rigorously verified

# Overview
There are two hall-sensors positioned to be adjacent to a magnet lodged into the ring as it spins. As the gear spins one direction it a magnet approaches one sensor and a magnet moves away from other, when it spins the opposite direction the same behavior happens but for the opposite sensors. We use this to determine the position of the ring and drive the motor as necessary. As a magnet approaches a hall sensor it increases the signal (voltage-level on the output pin) and as it moves away it decreases the signal the hall sensor receives. 

There are three high level states we consider:

- Good-Good State: Where both sensors are in agreement of the ring position and have inverse signals of each other when not in the inbetween state. 
Ex:
  - (sensor 1 unlocked (high-signal), sensor 2 unlocked (low-signal))
  - (sensor 1 inbetween, sensor 2 inbetween)

- Good-bad State: Where one sensor reports a locked or unlocked state and the other sensor does not report an inverse state and both sensors do not report the same state
Ex:
  - sensor 1 unlocked, sensor 2 under 
  - sensor 1 unlocked, sensor 2 inbetween

- Atomic state(oh shit state): Where both sensors report out of range signals or report the same state
Ex:
  - sensor 1 unlocked (low-signal), sensor 2 locked (low-signal)
  - sensor 1 over (low-signal), sensor 2 over (low-signal)

# Weird Quirks/Tips?

- Each sensor does not have the same exact range of signals 
  - Needs more testing and possibly calibration as of now [09-19-2025]

- Hardware debugging:
  - Approach the hall sensor using a magnets south pole to simulate ring movement
  - the orientation of the physical board and the pins in the board matter for reports, refer to datasheet for greater detail.
