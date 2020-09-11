# PSoC 6 MCU: ADC UART

This example demonstrates how to use HAL APIs to configure and use the ADC component. The results are IIR filtered using a callback API to demonstrate post-processing. The filtered results are printed out through UART using retarget-io library.
## Requirements

- [ModusToolbox™ software](https://www.cypress.com/products/modustoolbox-software-environment) v2.1
- Programming Language: C

## Supported Kits

- [PSoC 6 WiFi-BT Pioneer Kit](https://www.cypress.com/CY8CKIT-062-WiFi-BT) (CY8CKIT-062-WiFi-BT)

## Hardware Setup

This example can be used with the board's default configuration. See the kit user guide to ensure that the board is configured correctly.

The ADC input can be provided on pin p 10[0]. The UART results are printed out through the device debug uart pins. Pin 5[0] Rx and pin 5[1] Tx are the pins of the device respectively.

## Software Setup

This example requires no additional software or tools.

## Using the Code Example

Please refer to [Exercises_ReadMe.pdf](../Exercises_ReadMe.pdf) for importing the application and programming the device.

## Operation

1. ADC scans a single channel.

2. The result obtained is processed (IIR filtered) in an interrupt function.

3. The result is then pushed through UART stream once the processing is complete.

## Design and Implementation

1. ADC is setup using default configurations.

2. Channel 0 is configured with p10[0] as input.

3. UART is set up using retarget_io library.

4. ADC callback function is setup and end of scan event is also enabled.

5. IIR filter is implemented in callback function to demonstrate post-processing.

