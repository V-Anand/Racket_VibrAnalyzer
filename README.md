Title : Tennis VibrAnalyzer
Developer : V-Anand
Submitted For : Hexiwear Contest - Due date Oct 9, 2016

Project Introduction
====================

Tennis VibrAnalyzer measures vibrations travelling through
the hand as result of the racket hitting the ball.

This utilizes the "transient acceleration" function provided
by the NXP FXOS8700CQ Accelerometer/Magnetometer combo chip.

See "AN4461" document from NXP for more information on transient
acceleration.

This Analyzer provides a "Start"/"Stop" buttons to start/stop the analysis.

When stopped, the app displays the result of the analysis.

Pressing "Stop" again, resets the analysis and clears the app for the next
analysis session. If connected to pc, the reset function also sends some
details of analysis thru the serial interface at 9600 baud.

PS: The serial interface was chosen as BLE loses connectivity with phone inside a tennis court.

Build Instructions
==================
These instructions that "ARM mbed CLI" is available.

Clone the project.

cd project-folder
mbed deploy
mbed toolchain GCC_ARM
mbed target hexiwear
mbed compile

Copy .bin file created to hexiwear over the "DAPLINK".


