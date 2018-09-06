/*
 * spi.h
 *
 * Created: 8/24/2018 10:31:43 AM
 *  Author: Jan van Deventer
 * The purpose of these function is to introduce the on board accelerometer which we interact with via SPI.
 * Updated September 5, 2018.
 */ 

// Constant definitions--------------------------------------------------------------------------------

#define DDR_SPI DDRB
#define DD_MISO PB3
#define DD_MOSI PB2
#define DD_SCK PB1
#define DD_SS PB0	// This pin in only connected to JP12, so one always needs a jumper
// if using more than one slave, one needs more chip select

// Type definitions-------------------------------------------------------------------------------------

// This is a data structure with three attributes: the accelerations in three directions form the chip
typedef struct  
{
	 uint16_t x,y,z;
} accelSPI;

// Function definitions--------------------------------------------------------------------------------

void SPI_MasterInit(void);

uint8_t spiWriteByte(volatile uint8_t *slavePort, uint8_t slavePinMask, uint8_t ByteAddr, uint8_t cData);

uint16_t spiReadWord(volatile uint8_t *slavePort, uint8_t slavePinMask, uint8_t lowByteAddr);