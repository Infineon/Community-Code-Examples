# PSoC 6 MCU: ULP Sleep

This example shows how to transition to System ULP mode, CPU Sleep mode. The example uses a timer to toggle the USER LED every 1 second and a USER BUTTON to transition the device to low power mode.

## Requirements

- [ModusToolbox™ software](https://www.cypress.com/products/modustoolbox-software-environment) v2.1
- Programming Language: C

## Supported Kits

- [PSoC 6 WiFi-BT Pioneer Kit](https://www.cypress.com/CY8CKIT-062-WiFi-BT) (CY8CKIT-062-WiFi-BT)

## Hardware Setup

This example can be used with the board's default configuration. See the kit user guide to ensure that the board is configured correctly.

The current consumed by the device can further be reduced by removing the resistor **R86** which causes a leakage of about 3 μA on VBACKUP domain that is connected to VTARG.

## Software Setup

This example requires no additional software or tools.

## Using the Code Example

Please refer to [IMPORT.md](IMPORT.md) for importing the application and programming the device.

## Operation

1. Once the device is programmed, the USER LED will be toggling every 1 second.

2. Press the kit's USER BUTTON. The green LED turns on for 100 ms to indicate that the device is transitioning to low power mode and the toggling of the USER LED stops.

3. Press the kit's USER BUTTON to start the toggling of USER LED again. The green LED turns on again for 100 ms to indicate that the device is transitioning to active mode.

4. Refer to the device kit guide to know how to measure current consumed by the device. For CY8CKIT-062-WiFi-BT, connect the ammeter across the J8 current measurement jumper. 

## Design and Implementation

This example uses a custom BSP in which all unused resourses like unused peripherals, high speed clocks, debug capability etc., are disabled. The core regulator is set to minimum current buck regulator. The initial power mode of the device is System Low Power mode which is the default mode. To use this application for another target device, please refer to [IMPORT.md](IMPORT.md).

The functionality of the application is similar to other examples and as follows:
1. The application initializes all the GPIOs - Green LED, USER LED, USER BUTTON, and a timer with period set to 1 second to toggle the USER LED.

2. An interrupt is registered for the USER BUTTON which is triggered when the user presses the button.

3. Along with this, a power mode transition callback is also used to:
    * Stop the timer in CHECK_READY mode.
    * Turn on the Green LED for 100 ms in BEFORE_TRANSITION mode.
    * Turn on the Green LED for 100 ms and start the timer in AFTER_TRANSITION mode.

4. When the device is programmed, the USER LED toggles every 1 second. If the USER BUTTON is pressed, the device goes into System Ultra Low Power mode if it is not already.

5. The CPU then transitions to CPU Sleep mode and waits for an interrupt.

6. When the USER BUTTON is pressed again, the CPU wakes up and transitions to CPU Active mode.

7. For CY8CKIT-062-WiFi-BT running this application, the current consumed by the device in System ULP, CPU Sleep mode will be around 0.3 mA.

