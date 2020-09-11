# PSoC 6 MCU: LPComp Hibernate Mode Wake-up

This example demonstrates how to wake-up the device from hibernate mode using LPComp trigger only using PDL APIs. The application will toggle the LED twice initially. The device is moved to hibernate mode then. If the LPComp is not triggered, the device continues to be in hibernate mode. Once triggered the device wakes up from hibernate mode and the LED starts to blink. 

## Requirements

- [ModusToolbox™ software](https://www.cypress.com/products/modustoolbox-software-environment) v2.1
- Programming Language: C

## Supported Kits

- [PSoC 6 WiFi-BT Pioneer Kit](https://www.cypress.com/CY8CKIT-062-WiFi-BT) (CY8CKIT-062-WiFi-BT)

## Hardware Setup

This example can be used with the board's default configuration. See the kit user guide to ensure that the board is configured correctly.

Connect pin 5[6] to ground to keep the device in hibernate mode. Connect the pin to a voltage higher than LPcomp local reference(0.4V-0.7V) to wake-up the device.

The current consumed by the device can further be reduced by removing the resistor **R86** which causes a leakage of about 3 μA on VBACKUP domain that is connected to VTARG.

## Software Setup

This example requires no additional software or tools.

## Using the Code Example

Please refer to [Exercises_ReadMe.pdf](../Exercises_ReadMe.pdf) for importing the application and programming the device.

## Operation

1. If the positive terminal of the LPComp is connected to ground, the comparator is not triggered. The device moves into hibernate mode after blinking the LED twice.

2. The current consumption in this mode will be around ~3uA.

3. Once the positive terminal is connected to a higher voltage, the LPComp is triggered. The device comes out of Hibernate mode and LED blinks again. The LPComp is triggered again and again in this arrangement, so the device comes out of hibernate every time, which makes it look like the LED is blinking continuously.

4. The current consumption in this configuration is in ~10mA. 

## Design and Implementation

1. At first the application initializes the GREEN LED to strong drive.

2. The local reference and inputs are set for the LPComp and the component is initialized.

3. When the device is programmed, the device blinks the GREEN LED twice. Further blinking is dependent on whether the LPComp is triggered.

4. LPComp High output is set as the wake-up source for the device hibernate mode.

5. The device is sent to hibernate mode after the LED blink. If the comparator is not triggered the device continues in hibernate mode. Once the device is triggered, the device wake-up from hibernate and the LED blinks again.

6. For CY8CKIT-062-WiFi-BT running this application, the current consumed by the device in Hibernate mode will be around 3 uA.
