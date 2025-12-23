# Boost DC-DC Converter Control System

## Overview
This repository contains the design and implementation of a closed-loop DC-DC Boost
converter.
The objective of the project is to regulate the output voltage above the input voltage
using a digital control strategy implemented on an STM32 microcontroller with FreeRTOS.

## System Description
The system is based on a DC-DC Boost power stage composed of an inductor, power switch,
diode, and output capacitor.  
An STM32F411 microcontroller is responsible for generating the PWM signal that controls
the duty cycle of the converter and for executing the control algorithm to regulate the
output voltage.

The output voltage is measured through an analog feedback network and processed by the
ADC of the microcontroller. A closed-loop controller adjusts the PWM duty cycle to
maintain the desired voltage reference under load and input variations.

## Technologies
- STM32F411 microcontroller
- C programming language
- PWM-based duty cycle control
- ADC-based voltage measurement
- Closed-loop digital control
- FreeRTOS real-time operating system

## My contributions
- Design and implementation of the Boost converter controller
- Configuration of PWM peripherals for duty cycle control
- Configuration of the ADC and voltage scaling for feedback measurement
- Implementation of the control loop in FreeRTOS tasks
- Task synchronization and timing management using FreeRTOS
- System testing under different load and reference conditions
- Adjusting the voltage setpoint via the serial port (UART)

## Results
- Stable regulation of the output voltage above the input voltage
- Fast transient response to reference changes
- Reduced output voltage ripple after control tuning
- Reliable operation under different load conditions

## Repository Structure
/Inc        Header files  
/Src        Source code files  
/Project documentation  
/Results    Experimental results and measurements  
