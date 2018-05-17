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

This version of the code introduces delays to see a flashing light. In this case, we use the display back light.

## Exercises
- Change the lenght of the time on and off for the display back light.
- Push the buttons to turn on the LEDs (nothing should happen).
- Flash the LEDs instead of the display backlight LED.
- Insert the code from the master branch that connects thef push buttons to the LEDs, which you have used in the previous exercises.
You should notice that the use of *delay* creates a lag in response. So empty looping is not such a great solution.