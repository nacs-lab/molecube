#include <nacs-utils/utils.h>

#ifndef DAC_H_
#define DAC_H_

#include "spi_util.h"

namespace NaCs {

//set DAC voltage for channels 0-31 (14-bit)
//if channel > 31, set offset voltage
void SetDAC_AD5532(spi_p spi, unsigned channel, unsigned dacWord);

//set DAC voltage for channels 0-31, 14 bits, 200V
void SetDAC_AD5535(spi_p spi, unsigned channel, unsigned dacWord);

//set DAC voltage for channels 0-7 (16-bit)
void SetDAC_AD5668(spi_p spi, unsigned channel, unsigned dacWord);

//set DDS frequency and phase (28-bit via 2 consecutive 16-bit transfers)
void SetDDS_AD9833(spi_p spi, volatile void *pulse_addr, unsigned setType,
                   unsigned ddsPhase, unsigned ddsFreq);

//set DAC voltage for channels 0-39 (16-bit)
void SetDAC_AD5370(spi_p spi, unsigned channel, unsigned dacWord);

//start a conversion on previously spec'd ADC channel
void startConversion_AD7689(spi_p spi);

//get ADC conversion result and configure for next conversion
unsigned short getResult_AD7689(spi_p spi, unsigned next_channel);

//get ADC conversion result and configure for next conversion
unsigned getResult_ADS8361(spi_p spi, unsigned next_channel);

//get ADC conversion result and configure for next conversion
unsigned short getResult_AD7656(spi_p spi, unsigned num_channels,
                                short *pValues);
unsigned short getResult_AD7656i(spi_p spi, unsigned num_channels,
                                 int *pValues);
unsigned short getAddResult_AD7656(spi_p spi, unsigned num_channels,
                                   int *pValues);

void Init_ADS8361(spi_p spi);

}

#endif
