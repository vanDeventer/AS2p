/*
 * AS2p.c
 *
 * Created: 5/15/2018 10:00:41 AM
 * Modified: June 26, 2018
 * Author : Jan van Deventer
 * Course: E0009E Automotive Systems 2
 */ 

/*
 * Purpose of this version:
 * The purpose of this version of the software package is to introduce arrays and ASCII.
*/

#include <avr/io.h> // input output header file for this AVR chip.
#include <string.h>
#include "global.h"
#include "lcd.h"
#include "gpio.h"

#define DISPLAYLENGTH 16	/* number of characters on the display */

/** This function initializes the LCD display and should be called only once before the while(1) loop in the main(). */
int initDisplay(void)
{
	lcdInit();	//initialize the LCD
	lcdClear();	//clear the LCD
	lcdHome();	//go to the home of the LCD
	lcdPrintData("Hello World!", 12); //Display the text on the LCD
	PORTB |= 1 << DISPLAY_LED;	// Turn on the display's back light.

	return(0);
}

/** This is the main function of our application. There is one and only one such function.
 * The code starts executing here.
 */
int main(void)
{
	unsigned char temp ;		//Allocate memory for  temp
	char cursor = 0;				/* allocate a variable to keep track of the cursor position and initialize it to 0 */
	char textLine[DISPLAYLENGTH + 1];	/* allocate a consecutive array of 16 characters where your text will be stored with an end of string */

	temp = initGPIO();				//Set up the data direction register for both ports C and G
	temp = initDisplay();
	
	textLine[0] = 0x3A;
	textLine[1] = 0x2D;
	textLine[2] = 0x29;
	textLine[3] = 'A';					/* initialize the first ASCII character to A or 0x41 */
	textLine[4] = '\0';				/* initialize the second character to be an end of text string */
	lcdGotoXY(0, 1);     //Position the cursor at the beginning of the second line
	lcdPrintData(textLine, strlen(textLine)); //Display the text on the LCD from where you just positioned yourself
	
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