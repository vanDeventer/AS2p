/*
 * AS2p.c
 *
 * Created: 5/15/2018 10:00:41 AM
 * Modified: August 9, 2018
 * Author : Jan van Deventer
 * Course: E0009E Automotive Systems 2
 */ 

/*
 * Purpose of this version:
 * The purpose of this version of the software package is to introduce arrays and ASCII.
*/

#include <avr/io.h> // input output header file for this AVR chip.
#include <string.h>
#include <avr/pgmspace.h>	// Contains some type definitions and functions.
#include <avr/interrupt.h>	// Contains ISR (Interrupt Service Routines) or interrupt handler details
#include "global.h"
#include "lcd.h"
#include "gpio.h"

#define DISPLAYLENGTH 16	/* number of characters on the display */

volatile unsigned char buttons;		// This registers holds a copy of PINC when an external interrupt 6 has occurred.
volatile unsigned char bToggle = 0;	// This registers is a boolean that is set when an interrupt 6 occurs and cleared when serviced in the code.

/** This function sets up the External Interrupt, and specifically Interrupt 6 that is connected to the push buttons */
int initExtInt(void)
{
	//Set up external Interrupts
	// The five Switches are ORed to Pin PE6 which is alternatively Int6
	EICRB |= (0<<ISC61) | (1<<ISC60);  //Any logical change to INT6 generates an interrupt
	EIMSK |= (1<<INTF6);
	return(0);
}



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

/** This function uses the push buttons to let the user write text.*/
unsigned int DbTEXThandler(char *s, unsigned int position)
{
	switch(buttons & 0b11111000)
	{
		case 0b10000000:			//S5 center button
			PORTC |= 0b00000100;	//Turn on LED5
			break;
		case 0b01000000:			//S4  upper button
			if (s[position] < 'z')	// if you did not reach the ASCII letter z or 0x7A, you can go to the next letter in the table
				s[position]++;
			break;
		case 0b00100000:			//S3 left button
			if (position > 0) 		//If you have not reached the right side of the display, you can move the cursor one step to the right
				position--;
			break;
		case 0b00010000:			//S2 lower button
			if (s[position] > 'A')	// if you did not reach the ASCII letter A, you can go to the previous letter in the table
				s[position]--;
			break;
		case 0b00001000:			//S1 right button
			if (position < (DISPLAYLENGTH-1))		//If you have not reached the right side of the display with index starting at 0, you can move the cursor one step to the right
			{
				position++;
				s[position] = 'A';
				s[position+1] ='\0';
			}
			break;
		default:
			PORTC &= 0b11111011;	//Turn off LED5
			break;
	}
	return position;
}

/** This is the main function of our application. There is one and only one such function.
 * The code starts executing here.
 */
int main(void)
{
	unsigned char temp ;		//Allocate memory for  temp
	char cursor = 0;				/* allocate a variable to keep track of the cursor position and initialize it to 0 */
	char textLine[DISPLAYLENGTH + 1];	/* allocate a consecutive array of 16 characters where your text will be stored with an end of string */

	temp = initGPIO();		//Set up the data direction register for both ports C and G
	temp = initDisplay();	//Set up the display
	temp = initExtInt();	//Set up the external interrupt for the push buttons
	
	textLine[0] = 'A';				/* initialize the first ASCII character to A or 0x41 */
	textLine[1] = '\0';				/* initialize the second character to be an end of text string */
	lcdGotoXY(0, 1);     //Position the cursor at the beginning of the second line
	lcdPrintData(textLine, strlen(textLine)); //Display the text on the LCD from where you just positioned yourself

	sei();					// enable the global interrupts
	
	while(1){
		temp++;
		if (bToggle) 
		{		//This is set to true only in the interrupt service routine at the bottom of the code
			cursor = DbTEXThandler(textLine, cursor);
			bToggle = 0;			//
			lcdGotoXY(0, 1);     //Position the cursor on
			lcdPrintData(textLine, strlen(textLine)); //Display the text on the LCD
		}

	}
	return(0); // This line of code will never be executed since it is after the "while(1)" block
}

/* the functions below are Interrupt Service Routines, they are never called by software */

/** This function is executed every time Interrupt 6 is triggered. */
SIGNAL(SIG_INTERRUPT6)  //Execute the following code if an INT6 interrupt has been generated. It is kept short.
{
	bToggle = 1;	//Some push button has been pushed or released. Action needs to be taken.
	buttons = PINC; //Take a snapshot of the input register of Port C where the push buttons are connected.
}
