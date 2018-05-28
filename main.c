/*
 * AS2p.c
 *
 * Created: 5/15/2018 10:00:41 AM
 * Modified: May 28, 2018
 * Author : Jan van Deventer
 * Course: E0009E Automotive Systems 2
 */ 


/*
 * Purpose of this version:
 * The purpose of this version of the software package is to introduce the Analog to Digital Converter (ADC)
 * The value of the analog signal is displayed using the five LEDs.
 * You will need a wire or jumper connecting POT1 or POT2 and ADC0.
*/

#include <avr/io.h> // input output header file for this AVR chip.
#include <avr/pgmspace.h>	// Contains some type definitions and functions.
#include <avr/interrupt.h>	// Contains ISR (Interrupt Service Routines) or interrupt handler details
#include "gpio.h"

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



int main(void)
{
	unsigned char temp;			//Allocate memory for  temp
	
	temp = initGPIO();			//Set up the data direction register for both ports B, C and G
	temp = initADC();			// Setup the Analog to Digital Converter
	ADCSRA |= (1<<ADSC);		//Start ADC

	sei();						// Set Global Interrupts
	
	
	while(1)
	{
		cli();					// Disable the global interrupts to prevent accidental corruption of the results while the two bytes.
		if (adc_value>852){
			PORTC = 0b00000111;
			PORTG |= 0b00000011;
		}
		else if(adc_value>682){
			PORTC = 0b00000011;
			PORTG |= 0b00000011;
		}
		else if(adc_value>511){
			PORTC = 0b00000001;
			PORTG |= 0b00000011;			
		}
		else if(adc_value>341){
			PORTC = 0b00000000;
			PORTG |= 0b00000011;
		}
		else if(adc_value>170){
			PORTC = 0b00000000;
			PORTG |= 0b00000001;
			PORTG &= 0b11111101;
		}
		else{
			PORTC = 0b00000000;
			PORTG &= 0b00000000;
		};
		
		sei();					// re-enable the global interrupts
	}
	return(0); // This line of code will never be executed since it is after the "while(1)" block
}

ISR(ADC_vect){
	adc_value = ADCL;		//Load the low byte of the ADC result
	adc_value += (ADCH<<8); //shift the high byte by 8bits to put the high byte in the variable
}