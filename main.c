/*
 * AS2p.c
 *
 * Created: 5/15/2018 10:00:41 AM
 * Modified: September 16, 2019
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


/** This is the main function of our application. There is one and only one such function.
 * The code starts executing here.
 */
int main(void)
{
	unsigned char temp ;		//Allocate memory for  temp
	char text[10];					//Allocate an array of 10 bytes to store text

	uint16_t adcBuffer;

	temp = initGPIO();			//Set up the data direction register for both ports B, C and G
	temp = initDisplay();	//Set up the display
	temp = initADC();			// Setup the Analog to Digital Converter
	
	ADCSRA |= (1<<ADSC);		//Start ADC
	sei();						// Set Global Interrupts

	while(1)
	{
		adcBuffer = adc_value;
		/* When an ADC conversion is complete, the result is found in these two registers.
		* When ADCL is read, the ADC Data Register is not updated until ADCH is read.
		* Consequently, if the result is left adjusted and no more than 8-bit precision is required, it is sufficient to read ADCH.
		* Otherwise, ADCL must be read first, then ADCH. The ADLAR bit in ADMUX, and the MUXn bits in ADMUX affect the way the result is read from the registers.
		* If ADLAR is set, the result is left adjusted. If ADLAR is cleared (default), the result is right adjusted.
		*/

		itoa(adcBuffer, text, 10);	//Convert the unsigned integer to an ascii string; look at 3.6 "The C programming language"
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


ISR(ADC_vect){
	adc_value = ADCL;		//Load the low byte of the ADC result
	adc_value += (ADCH<<8); //shift the high byte by 8bits to put the high byte in the variable
}
