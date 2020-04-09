# LV-3.1 Recovery Repository

The LV-3.1 Electromechanical Recovery System (ERS) is a fully-resettable recovery system for the [PSAS](http://psas.pdx.edu/) Launch Vehicle v 3.1 (LV-3.1).  The system uses a geared motor to release the nosecone and drogue chute, and a linear actuator to release the main chute. The ERS expects deployment signals from LV-3.1 avoics  and an onboard control board uses optical encoding to determine and confirm deployment states.

## Table of Contents

* [Mechanical System (/mechanicalsystem)](#mechanical-system)
  * [Main Assembly](#main-assembly)
  * [Twist Coupling/Nose Cone Release](#twist-coupling)
  * [3-Ring Release](#3-ring-release)
  * [Parachute Cups](#parachute-cups)
  * [Interfacing and Miscellany](#interfacing-and-miscellany)

* [Control System (/controlsystem)](#control-system)
  * [Requirements](#requirements)
  * [Code Overview](#code-overview)
  * [Board Design](#board-design)
  * [Testing and Miscellany](#testing-and-miscellany)

* [Testing and Analysis (/analysis)](#testing-and-analysis)

## Mechanical System

note: Solidworks files for all components are found under /mechanicalsystem/cad, and primarily follow the naming convention of "Foo part" --> foo_part.ext (e.g. twist coupling -> twist_coupling_v4.SLDPRT). The complete assembly can be found [here](https://github.com/psas/lv3.1-airframe/tree/master/cad/nose); all parts in its parent directory are needed to properly view the assembly.

### Main Assembly

The ERS main assembly houses all the necessary components for functionality of the system. The upper ring houses the electromechanical components, and is coupled with the upper ring by the [twist coupling](#twist-coupling). The [nose cone](https://github.com/psas/lv3.1-airframe/tree/master/cad/nose) is fastened to the upper ring using the [LV-3.1 arc-clamp system](https://github.com/psas/lv3.1-airframe/tree/master/cad/coupling) and houses the [parachute cups](#parachute-cups) above the main assembly. The keeper ring is fastened near the bottom of the lower ring and serves as a platform for the battery, control board, geared motor, linear actuator, parachute anchors, and the [pizza table](#pizza-table). Attached to the inner diameter of the upper ring is the tube ring which serves as an attachment point for the surgical tubing used to eject the nose cone from the rocket. The ring components of the main assembly are machined out of 6061 aluminum.

// Add schematic

### Pizza Table

The pizza table elevates the parachute cups above the electromechanical components and supports the weight of the parachutes and cups and the preload force of the surgical tubing. The legs of the pizza table also secure the control board and positions the optical sensors needed for function of the [control system](#control-system). In the current iteration, the pizza table is made of of 3D-printed carbon-fiber reinforced nylon.

// pizza table image

### Twist Coupling

The twist coupling couples the nose cone the rest of the rocket assembly and releases the nose cone by twisting out of a locked state. Once released, the surgical tubing rapidly deploys the nose cone, which deploys the drogue parachute (details in [parachute-cups](#parachute-cups)). The twist coupling has six L-shaped channels that mesh with six pins on the upper ring, and an internal spur gear that meshes with a gear on the motor. Upon [receiving the necessary signal](#control-system), the motor will rotate the twist coupling to release the upper ring and nose cone. The grey code tape necessary for control logic is attached to the inside of a skirt on the twist coupling, and is read my the sensors on the control board. Machined Delrin was used to fabricate the twist coupling to reduce friction the the aluminum upper and lower rings

// twist coupling image

### 3-Ring Release



### Parachute Cups



### Interfacing and Miscellany



## Control System

### Requirements
The control system is a PCBA with an STM32F0 microcontroller. The PCBA drives a motor and linear actuator which will release the main chute and the nose cone when told to do so by the main avionics. The full requirements are summarized here: 

* Receive two incoming signals from main avionics
	* Release nose cone
	* Release parachute
* All logic controlled by an STM32F0
* Use BPR-301 phototransistor sensors to verify nose cone separation
* Run off LiPo batteries


### Code Overview

The firmware being run by the STM32F0 is written in C/C++. It was written in an IDE provided by STMicroelectronics called STM32CubeIDE, and the firmware folder contains this project. Within this project directory, the /Core subdirectory contains all the control code. 


### Board Design

The board was designed in EAGLE to support the requirements of the system. this is in the /eagle subdirectory of the control system.


### Testing and Miscellany



## Testing and Analysis
