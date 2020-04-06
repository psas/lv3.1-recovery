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

### Main Assembly



### Twist Coupling



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



