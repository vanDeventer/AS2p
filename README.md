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
This version of the software uses external interrupts to tigger a reaction from the micro-controller.
This avoids delays in the case of some other code being executed and having to poll an input to see if an input state has changed.

Since only one interrupt is used for all the buttons (via OR gates) only the first button is detected.

## Exercises
- Use a breakpoint to step through the code of the interrupt.
- Insert the code for the flashing the display back light in the *while (1)* loop (and outside the *if (bToggle)*) to see that now there is no lagging of response (ref. v0.1.2).
- What is Int4 connected to on your development board?