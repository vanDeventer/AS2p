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
The purpose of this version is to use the accelerometer on the red development board (version 3).
 The measured acceleration are in double bytes and transmitted over SPI.
  The board also has a 3D Gyroscope, which also communicates over SPI and needs another Chip Select.
## Exercises
-  Display the Y or Z axis instead of the X axis acceleration.
- Display values of m/s^2 or g for the acceleration.
- Change the code to use the Gyroscopes.
- Use a free GPIO so select to allow you to measure both the accelesations and rotational rates.