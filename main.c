/*
 * AS2p.c
 *
 * Created: 5/15/2018 10:00:41 AM
 * Modified: May 17, 2018
 * Author : Jan van Deventer
 * Course: E0009E Automotive Systems 2
 */ 

#include <avr/io.h> // input output header file for this AVR chip.
#include "gpio.h"

/*
 * Purpose of this version:
 * The purpose of this version of the software package is to introduce the idea of the include files.
 * The code is the same as that of the previous version but spread in different files.
*/




int main(void)
{
	unsigned char temp ;		//Allocate memory for  temp
	
	temp = initGPIO();				//Set up the data direction register for both ports C and G
	
	while(1)
	{
			temp = PINC;			// Copy Input Port C register (PINC) to temp.
			temp &= 0b11111000;		// Prepare to turn off Port C LEDs
			
			if (temp & 0b10000000)
				temp |= 0b00000100;		// Prepare to turn on Led5 if S5 is on
			if (temp & 0b01000000)
				temp |= 0b00000010;		// Prepare to turn on Led4 if S4 is on
			if (temp & 0b00100000)
				temp |= 0b00000001;		// Prepare to turn on Led3 if S3 is on			
			PORTC = temp & 0b00000111;	// Copy the last 3 bits of temp to Port C to turn on the LEDs.
			
			temp &= 0b11111000;		//Clear all LEDs so we do not turn on what we do not want
			if (temp & 0b00010000)
				temp |= 0b00000010;		// Prepare to turn on Led2 if S1 is on
			if (temp & 0b00001000)
				temp |= 0b00000001;		// Prepare to turn on Led1 if S1 is on			
			temp &= 0b00000011;		// Clear the upper bits of temp to then turn on only the 2 LEDs in the next line
			PORTG &= 0b11111100;	// Turn off the LEDs if they are on.
			PORTG |= temp;			// Copy the last 2 bits of temp to Port G to turn on the LEDs.
	}
	return(0); // This line of code will never be executed since it is after the "while(1)" block
}