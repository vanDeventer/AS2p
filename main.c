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
#include "delay.h"

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
		PORTB |= (1<<DISPLAY_LED);	// Set the display back light to ON. Leave other bits as they were.
		delay_ms(255); //Wait 255 ms
		PORTB &= !(1<<DISPLAY_LED);	// Set the display back light to OFF. Leave other bits as they were.
		delay_s(1);	//Wait 1 second
	}
	return(0); // This line of code will never be executed since it is after the "while(1)" block
}