#ps2-remote

##What is it?
Make my Playstation 2 reset button be controlled remotely. It uses an
infrared sensor (for the remote part), and a digital potentiometer
(for simulating a button press).

##Motivation
I don't want to get up to turn on and off my Playstation 2.

##Dependencies
Arduino Libraries:

* [Arduino-IRremote](https://github.com/shirriff/Arduino-IRremote)

##Status
- [x] Working prototype on breadboard
- [x] Transfer off of Arduino board
- [x] Design PCB
- [x] Etch PCB
- [x] Done!

![PS2 Remote Demo](https://raw.githubusercontent.com/mcastorina/ps2-remote/master/images/demo.gif)

##Future Goals
This project converts any button into an IR controlled button, however
it requires soldering and electrical knowledge to do so. A future goal
is to simplify the process to allow anyone to convert the buttons.

##Future Considerations
Next time I would use a MUX instead of resistance between the button
terminals. That way I could have a true open / close circuit which is
much cleaner than using resistance.
