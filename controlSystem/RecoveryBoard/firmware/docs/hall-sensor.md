> [!CAUTION]
> Everything is a WIP, Information subject to change and is not rigorously verified

# Overview
There are two hall-sensors positioned to be adjacent to a magnet lodged into the ring as it spins. As the gear spins one direction a magnet approaches one sensor and a magnet moves away from other, when it spins the opposite direction the same behavior happens but for the opposite sensors. We use this to determine the position of the ring and drive the motor as necessary. As a magnet approaches a hall sensor it increases the signal (voltage-level on the output pin) and as it moves away it decreases the signal the hall sensor receives. 

There are three high level states we consider:

- Good-Good State: Where both sensors are in agreement of the ring position and have inverse signals of each other when not in the inbetween state. 

Ex:
  - sensor 1 unlocked (high-signal), sensor 2 unlocked (low-signal)
  - sensor 1 inbetween, sensor 2 inbetween

- Good-bad State: Where one sensor reports a locked or unlocked state and the other sensor does not report an inverse state and both sensors do not report the same state

Ex:
  - sensor 1 unlocked, sensor 2 under 
  - sensor 1 unlocked, sensor 2 inbetween

- Atomic state(oh shit state): Where both sensors report out of range signals or report the same state

Ex:
  - sensor 1 unlocked (low-signal), sensor 2 locked (low-signal)
  - sensor 1 over (low-signal), sensor 2 over (low-signal)

> [!Important] Sensor Readings
> One of the sensors will have a higher value then the other as there is a difference of distance to the magnet in the unlocked/locked position of the ring. The sensor that have a **larger** value range will be designated as **Sensor 1**, the sensor that has a **smaller** value range will be **Sensor 2**.

>[!Important] Current limits
> The bare minimum to drive the motors will be around (400mA), ideally we will use 1A to unlock and lock while on the ground. While in flight we at least need it to be 1A to 1.5A. The max current limit will be 1.75A.
>   - Additionally, we might want to have an emergency state that while an error is detected in flight we will want to push the current very high to the point of possibly damaging the motor in order to save the rocket.

# Weird Quirks/Tips?

  - It would be best to implement them having values that are the states of inactive and active instead of unlocked/locked.
  - After some testing they still have values that are constantly jumping up and down. Having a value that will be the base threshold of being in an active/unactive state will probably be the best approach as we don't care if they have a signal higher than the upper range.

- Hardware debugging:
  - Approach the hall sensor using a magnets south pole to simulate ring movement
  - the orientation of the physical board and the pins in the board matter for reports, refer to datasheet for greater detail.
