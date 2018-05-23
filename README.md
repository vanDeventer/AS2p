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
This version of the software it to introduce internal interrupts.
To achieve this, we use Timer and Counters (which we thereby introduce).
In this present case, LED3 is flashed on and off.

## Exercises

- There are three crytal oscillators on the course development board. Using an oscilloscope in the lab, try to mearure their "beat".
- Measure the square pulse wave between LED3 and R20 on the board. What are the proportions of ON and OFF times? What is the time of each state? What is the frequency?
- Change the pre-scalor and check on the oscilloscope the change in pulse.
- Change the Output Compare Regiter and see how that affects the square wave.
- Change the mode of the TimerCounter and see how that affects the square wave.

- You should try to add the code with delay and see that a delay does not affect the execution of the Interrupt Sercice Routines (ISR).