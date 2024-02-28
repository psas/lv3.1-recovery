# LV-3.1 Recovery Repository

The LV-3.1 Electromechanical Recovery System (ERS) is a fully-resettable recovery system for the [PSAS](http://psas.pdx.edu/) Launch Vehicle v 3.1 (LV-3.1). Each ERS "unit" releases a single parachute. The top unit deploys our ballute test parachute that's in the nose cone, the next ERS unit releases a standard drogue, and the final ERS unit releases the main parachute.

## Mechanical CAD
All mechanical CAD has been moved to OnShape and can be found [here](https://cad.onshape.com/documents/03e169a7501c9614bcd8fe34/w/d2206dc55680e29e4d8f6e15/e/43f9d4697ebc459acca1436a).

## Control System

In this Git repo is a PCB with STM32F091 microcontroller that controls the system. See the ControlSystem directory.

## Testing and Analysis

The ERS system will be tested by dropping it from a helicopter over a field. Many parameters must be considered for this test including wind speed, altitude, field size, parachute drag coefficients, and parachute release timing. Analysis of drop test dispersal patterns based on fixing some parameters and simulating variation in others enables us to choose an appropriate drop site and drop parameters. Analysis notebooks can by found in the /analysis directory.

* [Parent Directory](https://github.com/psas/lv3.1-recovery/tree/master/Analysis)
* [Primary mission analysis document](https://github.com/psas/lv3.1-recovery/blob/master/Analysis/Mission_Analysis.ipynb)
* [Jupyter Notebook to run simulations](https://github.com/psas/lv3.1-recovery/blob/master/Analysis/drop_test_simulator.ipynb)
