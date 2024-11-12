# Arduino-RC-Plane
An RC airplane, controlled by a custom flight controller based on Arduino Nano.

## CAD

CAD files are available on OnShape [here](https://cad.onshape.com/documents/968e81f1afc8243e275bf7ca/w/acaf7929af669ea720083764/e/5f49399fe13c6903e510c987?renderMode=0&uiState=66e208f6a35c2d67603a374c)

## Controller hardware

The following devices are necessary to recreate the flight controller:
* Arduino Nano microcontroller
* BNO055 Adafruit absolute orientation sensor
* CC1101 radio module

* Flysky radio receiver (or any receiver with iBus)

## Flight modes

The controller enables you to choose from a few different modes of flying. A mode of flying can be chosen by flipping a switch on your radio transmitter on channel 6.

* `Mode 0 - Manual`: Your transmitter inputs are directly linked to the motors.
* `Mode 1 - Take-off`: The aircraft will maintain a 10 degree pitch-up with wings level.
* `Mode 2 - Fly-by-wire` - To be implemented: The aircraft will follow the angular velocity, commanded by the transmitter. For example, a full deflection  of right joystick corresponds to `4*pi rad/s` roll rate.
* `Mode 255 - Recovery`: The aircraft will assume -10 degree pitch down and wings level