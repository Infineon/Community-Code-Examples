# PSoC 6 MCU: Hibernate

This example shows how to transition to System Hibernate mode. The example uses a timer to toggle the USER LED every 1 second, custom pin to transition the device to hibernate mode and USER BUTTON configured as wakeup source.

## Requirements

- [ModusToolbox™ software](https://www.cypress.com/products/modustoolbox-software-environment) v2.1
- Programming Language: C

## Supported Kits

- [PSoC 6 WiFi-BT Pioneer Kit](https://www.cypress.com/CY8CKIT-062-WiFi-BT) (CY8CKIT-062-WiFi-BT)

## Hardware Setup

This example can be used with the board's default configuration. See the kit user guide to ensure that the board is configured correctly. 

To transition the device to System Hibernate mode, use a jumper wire and connect the CUSTOM USER PIN (P6_0 by default) to Ground.

The current consumed by the device can further be reduced by removing the resistor **R86** which causes a leakage of about 3 μA on VBACKUP domain that is connected to VTARG.

## Software Setup

This example requires no additional software or tools.

## Using the Code Example

Please refer to [IMPORT.md](IMPORT.md) for importing the application and programming the device.

## Operation

1. Once the device is programmed, the USER LED will be toggling every 1 second.

2. Connect the CUSTOM USER PIN to Ground using a jumper wire to transition the device to System Hibernate mode.

3. The green LED turns on for 100 ms to indicate that the device is transitioning to System Hibernate mode. The blinking of the USER LED stops.

4. Press the kit's USER BUTTON to wake the device from System Hibernate mode. The device resets and the Green LED blinks twice, turning on each time for 500 ms to indicate that the reset reason is hibernate wakeup.

5. Refer to the device kit guide to know how to measure current consumed by the device. For CY8CKIT-062-WiFi-BT, connect the ammeter across the J8 current measurement jumper. 

## Design and Implementation

This example uses a custom BSP in which all unused resourses like unused peripherals, high speed clocks, debug capability etc., are disabled. The core regulator is set to minimum current buck regulator. The initial power mode of the device is System Low Power mode which is the default mode. To use this application for another target device, please refer to [IMPORT.md](IMPORT.md).

The functionality of the application is similar to other examples and as follows:

1. The application initializes all the GPIOs - Green LED, USER LED, CUSTOM USER PIN (P6_0 by default), and a timer with period set to 1 second to toggle the USER LED.

2. An interrupt is registered for the CUSTOM USER PIN which is triggered when the CUSTOM USER PIN is connected to Ground.

3. Along with this, a power mode transition callback is also used to:
    * Stop the timer in CHECK_READY mode.
    * Turn on the Green LED for 100 ms and free the GPIO used in BEFORE_TRANSITION mode.

4. AFTER_TRANSITION mode is not available in System Hibernate mode as device is reset on wakeup.

5. When the device is programmed, the USER LED toggles every 1 second. If the CUSTOM USER PIN is connected to Ground, the application initializes the USER BUTTON GPIO and configures it as the hibernate wakeup source, and the device goes into System Hibernate mode. The blinking of the USER LED stops.

6. When the USER BUTTON is pressed, the device wakes up from System Hibernate mode and resets with reset reason set to hibernate wakeup.

7. If the reset reason is set to hibernate wakeup, the Green LED blinks twice, turning on each time for 500 ms to indicate the same. The application starts again.

8. For CY8CKIT-062-WiFi-BT running this application, the current consumed by the device in System Hibernate mode will be less than 3 uA.

