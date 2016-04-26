# bldc-drive

This is a project developing a simple BLDC servo controller. It is based on an STM32F103 microcontroller.

Some parts, especially the PID control loop is based on development done by user mcm_xyz in cnczone.com forum. Thank you! Check the thread http://www.cnczone.com/forums/open-source-controller-boards/283428-cnc.html 


##Firmware
Firmware is a work-in-progress (and will be for a while :), however following features are included:
* Trapezoidal BLDC commutation using either HALL sensors or quadrature encoder.
* Step+Dir input interface with PID position control loop.
* PWM+Dir input interface in velocity mode.
* USART communitacation for configuration. Configuration settings saved to flash memory.

###ToDo:
* ADC current limiting. Hardware support is there, and initial ADC functionality is in firmware, but needs finalization.

##Hardware:
First prototype hardware is built and it is working well. However, there are couple of known issues:
* 6n137 optocoupler is not officially supporting 3.3V supply voltage which is used in the board. In reality they seem to work, but the optocoupler should be changed or additional levelshifting circuitry added on next revision. Also, the optocoupler input circuit supports only push-pull-type encoder output. Many encoders seem to have open drain output. :(
* Few connectors in the layout missed solder stop openings. They were bit painfull to solder. :)






