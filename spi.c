/*
 * spi.c
 *
 * Created: 8/24/2018 10:31:43 AM
 *  Author: Jan van Deventer
 * The purpose of these function is to introduce the on board accelerometer which we interact with via SPI.
 * Updated on August 29, 2018.
 */

#include <avr/io.h> // input output header file for this AVR chip.
#include "spi.h"

void SPI_MasterInit(void)
{
	DDR_SPI |= (1<<DD_MOSI)|(1<<DD_SCK)|(1<<DD_SS); //Master out Slave in (MOSI) is an output, Clock is an output, Slave select at output(this limits the master to be both but OK for here)
//	DDR_SPI &= ~(1<<DD_MISO);	//Master in Slave out (MISO) is an input
	PORTB |= (1<<DD_SS);		//set the slave select high: idle
	/* Enable SPI, Master, set clock rate fck/64 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);
}


uint8_t spiWriteByte(volatile uint8_t *slavePort, uint8_t slavePinMask, uint8_t ByteAddr, uint8_t cData) // e.g. (&PORTB, (1<<DD_SS),0x28,0xAB)
{
	uint8_t loVal;
		
	*slavePort &= ~slavePinMask;	// Select the desired chip or slave

	SPDR = 0b00000000 | ByteAddr;	// Start the transmission to address the specific register with no auto-increment
	while(!(SPSR & (1<<SPIF)));	/* Wait for transmission complete */
	loVal = SPDR;
	SPDR = cData;	// Start the transmission with the byte to write
	while(!(SPSR & (1<<SPIF)));	/* Wait for transmission complete */
	loVal = SPDR;
	
	*slavePort |= slavePinMask;	// Release the desired chip or slave

	return loVal;
}


uint16_t spiReadWord(volatile uint8_t *slavePort, uint8_t slavePinMask, uint8_t lowByteAddr) // e.g. (&PORTB, (1<<DD_SS),0x28)
{
	uint16_t spiWord;
	uint8_t loVal, hiVal;
	
	*slavePort &= ~slavePinMask;	// Select the desired chip or slave

	SPDR = 0b11000000 | lowByteAddr;  //Read with auto-increment
	while(!(SPSR & (1<<SPIF)));	/* Wait for transmission complete */
	loVal = SPDR;
	SPDR = 0xFF;
	while(!(SPSR & (1<<SPIF)));	/* Wait for transmission complete */
	loVal = SPDR;	
	SPDR = 0xFF;
	while(!(SPSR & (1<<SPIF)));	/* Wait for transmission complete */
	hiVal = SPDR;
	
	*slavePort |= slavePinMask;	// Release the desired chip or slave

	spiWord = (hiVal<<8) | (loVal);
	return spiWord;
}