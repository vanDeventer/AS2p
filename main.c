/*
 * AS2p.c
 *
 * Created: 5/15/2018 10:00:41 AM
 * Modified: August 10, 2018
 * Author : Jan van Deventer
 * Course: E0009E Automotive Systems 2
 */ 

/*
 * Purpose of this version:
 * The purpose of this version is to merge two earlier software packages to display a value of the ADC on the LCD screen
 * The value of the analog signal is displayed using the five LEDs.
 * You will need a wire or jumper connecting POT1 or POT2 and ADC0.
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
volatile unsigned char textEdit= 0;	// Boolean to enable text editing.
volatile uint16_t adc_value;  //Allocate the double byte memory space into which the result of the 10 bits Analog to Digital Converter (ADC) is stored.

int initADC(){
	//Set up analog to digital conversion (ADC)
	//ADMUX register
	//AVcc with external capacitor on AREF pin (the 2 following lines)
	ADMUX &= ~(1<<REFS1);  //Clear REFS1 (although it should be 0 at reset)
	ADMUX |= (1<<REFS0);   //Set REFS0
	ADMUX &= (0b11100000); //Single ended input on ADC0
	ADMUX &= ~(1<<ADLAR);  //Making sure ADLAR is zero (somehow it was set to 1)
	//The ACDC control and status register B ADCSRB
	ADCSRB &= ~(1<<ADTS2) & ~(1<<ADTS1) & ~(1<<ADTS0);  //Free running mode
	//The ADC control and status register A ADCSRA
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) |(1<<ADPS0);//set sampling frequency pre-scaler to a division by 128
	ADCSRA |= (1<<ADEN)  | (1<<ADATE) | (1<<ADIE);//enable ADC, able ADC auto trigger, enable ADC interrupt
	return(0);
}

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
	lcdPrintData("ADC 1ch!", 8); //Display the text on the LCD
	PORTB |= 1 << DISPLAY_LED;	// Turn on the display's back light.

	return(0);
}

///** This function uses the push buttons to let the user write text.*/
//unsigned int DbTEXThandler(char *s, unsigned int position)
//{
	//switch(buttons & 0b11111000)
	//{
		//case 0b10000000:			//S5 center button
			//PORTC |= 0b00000100;	//Turn on LED5
			//textEdit = !textEdit;
			//if (textEdit){
				//lcdGotoXY(10, 0);     //Position the cursor on
				//lcdPrintData("input", 5); //Inform the user of edit mode in upper right part of the LCD
			//} else {
				//lcdGotoXY(10, 0);     //Position the cursor on
				//lcdPrintData("     ", 5); //Clear the upper right part of the LCD
			//}
			//break;
		//case 0b01000000:			//S4  upper button
			//if (s[position] < 'z' && textEdit)	// if you did not reach the ASCII letter z or 0x7A, you can go to the next letter in the table
				//s[position]++;
			//break;
		//case 0b00100000:			//S3 left button
			//if (position > 0 && textEdit) 		//If you have not reached the right side of the display, you can move the cursor one step to the right
				//position--;
			//break;
		//case 0b00010000:			//S2 lower button
			//if (s[position] > 'A' && textEdit)	// if you did not reach the ASCII letter A, you can go to the previous letter in the table
				//s[position]--;
			//break;
		//case 0b00001000:			//S1 right button
			//if (position < (DISPLAYLENGTH-1) && textEdit)		//If you have not reached the right side of the display with index starting at 0, you can move the cursor one step to the right
			//{
				//position++;
				//s[position] = 'A';
				//s[position+1] ='\0';
			//}
			//break;
		//default:
			//PORTC &= 0b11111011;	//Turn off LED5
			//break;
	//}
	//return position;
//}

/** This is the main function of our application. There is one and only one such function.
 * The code starts executing here.
 */
int main(void)
{
	unsigned char temp ;		//Allocate memory for  temp
// 	char cursor = 0;				/* allocate a variable to keep track of the cursor position and initialize it to 0 */
//	char textLine[DISPLAYLENGTH + 1];	/* allocate a consecutive array of 16 characters where your text will be stored with an end of string */
	char text[10];					//Allocate an array of 10 bytes to store text

	uint16_t adcBuffer;

	temp = initGPIO();			//Set up the data direction register for both ports B, C and G
	temp = initDisplay();	//Set up the display
//	temp = initExtInt();	//Set up the external interrupt for the push buttons
	temp = initADC();			// Setup the Analog to Digital Converter
	
	ADCSRA |= (1<<ADSC);		//Start ADC
	sei();						// Set Global Interrupts

	while(1)
	{
		ADCSRA &= ~(1<<ADIE);		//disable ADC interrupt to prevent value update during the conversion
		adcBuffer = adc_value;
		ADCSRA |= (1<<ADIE);		//re-enable ADC interrupt

		itoa(adcBuffer, text, 9);	//Convert the unsigned integer to an ascii string; look at 3.6 "The C programming language"
		OCR0A = adcBuffer >> 2;		// using the top 8 bits of the ADC, load OCR0A to compare to the timer Counter 0 to generate aPWM

		if (adcBuffer>852){
			PORTC = 0b00000111;
			PORTG |= 0b00000011;
		}
		else if(adcBuffer>682){
			PORTC = 0b00000011;
			PORTG |= 0b00000011;
		}
		else if(adcBuffer>511){
			PORTC = 0b00000001;
			PORTG |= 0b00000011;			
		}
		else if(adcBuffer>341){
			PORTC = 0b00000000;
			PORTG |= 0b00000011;
		}
		else if(adcBuffer>170){
			PORTC = 0b00000000;
			PORTG |= 0b00000001;
			PORTG &= 0b11111101;
		}
		else{
			PORTC = 0b00000000;
			PORTG &= 0b00000000;
		}
		lcdGotoXY(5, 1);     //Position the cursor on
		lcdPrintData("      ", 6); //Clear the lower part of the LCD
		lcdGotoXY(5, 1);     //Position the cursor on
		lcdPrintData(text, strlen(text)); //Display the text on the LCD

	}
}


/* the functions below are Interrupt Service Routines, they are never called by software */

/** This function is executed every time Interrupt 6 is triggered. */
SIGNAL(SIG_INTERRUPT6)  //Execute the following code if an INT6 interrupt has been generated. It is kept short.
{
	bToggle = 1;	//Some push button has been pushed or released. Action needs to be taken.
	buttons = PINC; //Take a snapshot of the input register of Port C where the push buttons are connected.
}

ISR(ADC_vect){
	adc_value = ADCL;		//Load the low byte of the ADC result
	adc_value += (ADCH<<8); //shift the high byte by 8bits to put the high byte in the variable
}
