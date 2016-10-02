Title : Tennis VibrAnalyzer
Developer : V-Anand
Submitted For : Hexiwear Contest - Due date Oct 9, 2016

Tennis VibrAnalyzer measures vibrations travelling through
the hand as result of the racket hitting the ball.

This utilizes the "transient acceleration" function provided
by the NXP FXOS8700CQ Accelerometer/Magnetometer combo chip.

See "AN4461" document from NXP for more information on transient
acceleration.

This Analyzer provides a "Start"/"Stop" buttons to start/stop the analysis.

When stopped, the app displays the result of the analysis.

Good - Less than 200 vibration events detected

Pass - Between 200 to 500 vibration events detected

Poor - More than500 vibration events detected


Pressing "Stop" again, resets the analysis and clears the app for the next
analysis session. If connected to pc, the reset function also sends some
details of analysis thru the serial interface at 9600 baud.

PS: The serial interface was chosen as BLE loses connectivity with phone inside a tennis court.

