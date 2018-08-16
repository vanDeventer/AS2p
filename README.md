# AS2p
E0009E Automotive Systems 2: Parallel version

A car is not anymore just a mechanical system anymore.
It is a mechatronic system of systems with electrical and electonic systems on a chassis.
(With connections to the Internet, it becomes a cyber phyrical system (CPS).)
The advantage added by the electronics is that several inputs can be used to make the correct decision about a process.
For example, an engine air fuel mixture does not depend only on RPM but also ambiant air temperature, pressure, engine temperature,...

The aim of this course is to enpower a student to understand and maybe design both the hardware and software in these mechatronic systems.
We use an old processor to flaten the learning curve.
It is only at the end of the course that we introduce micro controllers and systems on chip.

The text below this line in this README file is updated with each commit to enable a student to navigate rapidly between the diffent branch and commits.

# Version's aim
In this commit, we introduce further the idea of states (which are enumerated). 
We have a boot state, an ADC state and a text (input) state.
The ADC states uses the potentiometers on the development board to change the potential of the analog signal with respect to ground.
The user must add a wire or jumger between POT1 or POT2 to the ADC pin header.
To know which pin to use, one must refer tot he schematic of the board.
Initially, the code uses ADC0.
The value is also displayed using the 5 LEDs and the background display are controlled by the ADC in all states or modes.

## Exercises
- Change the language of initial greeting.
- Change the order of cycling (wich is only obvious at boot since you only have 3 states).
- Add a second channel, which you could select with (S3 and S1) os S5.
- Change the polarity of the PVM to the display.