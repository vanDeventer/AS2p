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
In this commit, we introduce the Analog to Digital Converter.
We use the potentiometers on the development board to change the potential of the analog signal with respect to ground.
The user must add a wire or jumger between POT1 or POT2 to the ADC pin header.
To know which pin to use, one must refer tot he schematic of the board.
Initially, the code uses ADC0.
The value is displayed using the 5 LEDs.

## Exercises
- Reverse the order of the LEDs as a function of the analog value.
- Change the input from ADC0 to ADC1 or ADC2.
- Using both potentiometers, take the difference ADC1 - ADC2.
- Alternate between ADC0 and ADC1 at runtime.
- Move to ADC setup routine to a .h and .c pair of files and run the that code. (Do not forget to add the files so your solution.)