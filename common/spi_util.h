#ifndef SPI_UTIL_H
#define SPI_UTIL_H

#include "fpga.h"

#include <nacs-xspi/xspi.h>

typedef XSpi spi_struct;
typedef XSpi* spi_p;

//! wrapper for XSpi_Transfer
void Spi_Transfer(spi_p spi, unsigned char* tx, unsigned char* rc, unsigned nBytes);

int SPI_init(spi_p spi, unsigned id, bool bActiveLow, char clockPhase);
unsigned SPI_Transmit(spi_p spi, unsigned* dataTX, unsigned* dataRC, unsigned nBytes);

//16 bit transfer.  MSB first.
unsigned SPI_Transmit16(spi_p spi, unsigned short* dataTX, unsigned short* dataRC);

unsigned short SPI_Transfer2(spi_p InstancePtr, unsigned short tx);
int SPI_SetSlaveSelect(spi_p InstancePtr, unsigned SlaveMask);
unsigned short SPI_Transfer_ADS8361(spi_p InstancePtr, unsigned tx);

#endif //SPI_UTIL_H
