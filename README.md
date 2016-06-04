# bldc-drive

This is a project developing a simple BLDC servo controller. It is based on an STM32F103 microcontroller.

Some parts, especially the PID control loop is based on development done by user mcm_xyz in cnczone.com forum. Thank you! Check the thread http://www.cnczone.com/forums/open-source-controller-boards/283428-cnc.html 

##Disclaimer
This software and hardware is provided "AS IS", WITHOUT ANY WARRANTY. The software is released under GPL v2. Authors accept no liability for any harm or loss resulting from use of this hardware or software.

##Firmware
Firmware is a work-in-progress (and will be for a while :), however following features are included:
* Trapezoidal BLDC commutation using either HALL sensors or quadrature encoder.
* Step+Dir input interface with PID position control loop.
* PWM+Dir input interface in velocity mode.
* USART communitacation for configuration. Configuration settings saved to flash memory.

###Update 1st May 2016
Some updates to the firmware. 
* ADC current limiting implemented. Appears to work OK
* I created a quick and ugly Python gui for tuning
* Updated the PID loop to have feedfoward coefficients FF1 and FF2 (like LinuxCNC). This means that the PID output can be adjusted by the requested speed and the requested acceleration. This made the PID tuning a lot easier for me at least, I'm able to get a motor to follow the requested position very well also during acceleration and during constant drive.


##Hardware:
First prototype hardware is built and it is working well. However, there are couple of known issues:
* 6n137 optocoupler is not officially supporting 3.3V supply voltage which is used in the board. In reality they seem to work, but the optocoupler should be changed or additional levelshifting circuitry added on next revision. Also, the optocoupler input circuit supports only push-pull-type encoder output. Many encoders seem to have open drain output. :(
* Few connectors in the layout missed solder stop openings. They were bit painfull to solder. :)
* The current amplifier INA27x connection is wrong. The IN+ and IN- should be swapped.

Second hardware revision with the above findings fixed is published. There are no known issues, two servo drives are succesfully driving my CNC router. Pics and videos to come..





