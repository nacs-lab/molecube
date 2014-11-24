#include "dac.h"
#include "fpga.h"

#include <common.h>

#include <string>
#include <iostream>


//set DAC voltage for channels 0-31
//if channel > 31, set offset voltage
void
SetDAC_AD5532(spi_p spi, unsigned channel, unsigned dacWord)
{
    unsigned tx = 0;
    unsigned rc = 0;

    if (channel > 31) {
        tx = 0x50000000;
    } else {
        tx = 0x40000000;
        tx |= (channel & 0x1F) << 22;
    }

    tx |= (dacWord & 0x3FFF) << 8;

    // printf("DAC<%u> = %08X (tx = %08X)\n", channel, dacWord, tx);
    SPI_Transmit(spi, &tx, &rc, 3);
}

//set DAC voltage for channels 0-31
void
SetDAC_AD5535(spi_p spi, unsigned channel, unsigned dacWord)
{
    //repeat first write twice
    static bool bNeedInit = true;

    if (bNeedInit) {
        bNeedInit = false;
        SetDAC_AD5535(spi, channel, dacWord);
    }

    unsigned tx = 0;
    unsigned rc = 0;

    tx = (channel & 0x1F) << 27;     //address bits
    tx |= (dacWord & 0x3FFF) << 13;  // 14-bit DAC word

    // printf("AD5535 DAC<%u> = %08X (tx = %08X)  SPI addr=%08X\n", channel,
    //        dacWord, tx, (unsigned)(spi->BaseAddr));

    SPI_Transmit(spi, &tx, &rc, 3);
}

//set DAC voltage for channels 0-7
void SetDAC_AD5668(spi_p spi, unsigned channel, unsigned dacWord)
{
    static bool bNeedInit = true;

    if (bNeedInit) {
        bNeedInit = false;

        unsigned tx = 0;
        unsigned rc = 0;

        tx = (0x06) << 24;   // command to load LDAC
        tx |= 0xFF;          // LDAC word

        printf("DAC init (tx = %08X)\n", tx);

        SPI_Transmit(spi, &tx, &rc, 4);
    }

    unsigned tx = 0;
    unsigned rc = 0;

    /*	tx =0xF;
       tx |=(0x2&channel)<<22;
       tx |= (dacWord&0xF) << 8;
     */

    tx = (0x02) << 24;               // command to write & update one channel
    tx |= (channel & 0x7) << 20;     //address bits
    tx |= (dacWord & 0xFFFF) << 4;   // 16-bit DAC word

    //printf("DAC<%u> = %08X (tx = %08X)\n", channel, dacWord, tx);
    //printf("DAC transmit = %i\n", tx);
    SPI_Transmit(spi, &tx, &rc, 4);
}

// set DDS phase and frequnecy (28-bit register set via two consecutive
// 16-bit transfers)
void
SetDDS_AD9833(spi_p spi, volatile void *pulse_addr, unsigned setType,
              unsigned ddsPhase, unsigned ddsFreq)
{
    unsigned tx = 0;
    unsigned rc = 0;
    unsigned short temp = 0;
    bool useSB(0), resetSB(0), setFreq(0);
    //bool setPhase(0);
    unsigned high_mask0, low_mask0, low_mask(0), updateDDS(0);

    // extract boolean values from input arguments
    useSB = bool((ddsFreq >> 31) & 1);
    resetSB = bool((ddsFreq >> 30) & 1);
    setFreq = bool(setType & 1);
    //setPhase = bool((setType >> 1) & 1);
    updateDDS = (setType & 12) << 4 ;

    //set TTL mask to control which DDSs get updated (TTLs control the FSYNC pin for the serial communication)
    PULSER_get_ttl(pulse_addr, &high_mask0, &low_mask0);  // get current TTL mask
    low_mask = low_mask0;
    low_mask = low_mask | updateDDS;


    if(useSB) { // use DDS
        if(resetSB) { // reset DDS phase to value stored in the phase register
            tx = (0x100) << 16;

            PULSER_set_ttl(pulse_addr, high_mask0, low_mask);
            usleep(1);
            SPI_Transmit(spi, &tx, &rc, 2); //write the reset command
            PULSER_set_ttl(pulse_addr, high_mask0, low_mask0);

            usleep(3);  //reset needs 7 to 8 clock cycles to take effect
            tx = 0;

            PULSER_set_ttl(pulse_addr, high_mask0, low_mask);
            usleep(1);
            SPI_Transmit(spi, &tx, &rc, 2); //take AD9833 out of reset mode
            PULSER_set_ttl(pulse_addr, high_mask0, low_mask0);
        }
        if (setFreq) { //update the DDS phase and frequency

            tx |= (0x2000) << 16; // write a full 28-bit frequency word

            PULSER_set_ttl(pulse_addr, high_mask0, low_mask);
            SPI_Transmit(spi, &tx, &rc, 2);
            PULSER_set_ttl(pulse_addr, high_mask0, low_mask0);

            temp = ddsFreq;
            temp = temp << 2;
            tx = (0x01) << 30; // write to frequency register 0
            tx |= (temp) << 14; // 14-bit DAC word

            PULSER_set_ttl(pulse_addr, high_mask0, low_mask);
            SPI_Transmit(spi, &tx, &rc, 2); //write the 14 least significant frequency bits
            PULSER_set_ttl(pulse_addr, high_mask0, low_mask0);

            temp = ddsFreq >> 14;
            temp = temp << 2;
            tx = (0x01) << 30; // write to frequency register 0
            tx |= (temp) << 14; // 14-bit DAC word

            PULSER_set_ttl(pulse_addr, high_mask0, low_mask);
            SPI_Transmit(spi, &tx, &rc, 2); //write the 14 most significant frequnecy bits
            PULSER_set_ttl(pulse_addr, high_mask0, low_mask0);

            temp = ddsPhase;
            temp = temp << 4;
            tx = (0x03) << 30;
            tx |= (temp) << 12;
            //printf("Phase transmit = %i\n", unsigned(tx));

            PULSER_set_ttl(pulse_addr, high_mask0, low_mask);
            SPI_Transmit(spi, &tx, &rc, 2); //write 12 bits to the phase register
            PULSER_set_ttl(pulse_addr, high_mask0, low_mask0);
        }
    } else { // turn off DDS
        tx = (0x100) << 16;
        SPI_Transmit(spi, &tx, &rc, 2); //write the reset command
    }
}

void cmdAD5370(spi_p spi, unsigned mode, unsigned channel, unsigned data)
{
    /* AD5370 takes 24-bit SPI commands

       I23 I22 I21 I20 I19 I18 I17 I16 I15 I14 I13 I12 I11 I10 I9 I8 I7 I6 I5 I4 I3 I2 I1 I0
       M1   M0  A5  A4  A3  A2  A1  A0 D15 D14 D13 D12 D11 D10 D9 D8 D7 D6 D5 D4 D3 D2 D1 D0

     */
    unsigned tx = 0;
    unsigned rc = 0;

    //there are 5 groups of 8 channels (group = 0 addresses all groups at once)
    unsigned group = (1 + channel / 8) & 0x7;

    //combine group bits with the 3 low bits of the channel#
    unsigned addr = (group << 3) | (channel & 0x7);

    tx  = (mode & 0x0003) << 30;
    tx |=  addr << 24;
    tx |= (data & 0xFFFF) << 8;

//	printf("AD5370 DAC mode=%u addr=%u data=%u (tx = %08X)\n", mode, addr, data, tx);
    SPI_Transmit(spi, &tx, &rc, 3);
}

void setAD5370offset(spi_p spi, unsigned reg, unsigned data)
{
    /* AD5370 takes 24-bit SPI commands

       I23 I22 I21 I20 I19 I18 I17 I16 I15 I14 I13 I12 I11 I10 I9 I8 I7 I6 I5 I4 I3 I2 I1 I0
       M1   M0  A5  A4  A3  A2  A1  A0 D15 D14 D13 D12 D11 D10 D9 D8 D7 D6 D5 D4 D3 D2 D1 D0

     */
    unsigned tx = 0;
    unsigned rc = 0;

    tx  = (reg + 2) << 24;
    tx |= (data & 0x3FFF) << 8;

//	printf("AD5370 DAC set offset reg. (tx = %08X)\n", tx);
    SPI_Transmit(spi, &tx, &rc, 3);
}

//set DAC voltage for channels 0-40
void SetDAC_AD5370(spi_p spi, unsigned channel, unsigned dacWord)
{
    static bool bNeedInit = true;

    if (bNeedInit) {
        bNeedInit = false;

        setAD5370offset(spi, 0, 0x1FFF);
        setAD5370offset(spi, 1, 0x1FFF);

        for (unsigned i = 0; i < 40; i++) {
            cmdAD5370(spi, 2, i, 0x8000); //set offset register
            cmdAD5370(spi, 1, i, 0xFFFF); //set gain register
        }
    }

    cmdAD5370(spi, 3, channel, dacWord);
    cmdAD5370(spi, 2, channel, 0x8000); //set offset register after every update (suspect corruption)
}


//start a conversion on previously spec'd ADC channel
void startConversion_AD7689(spi_p spi)
{
    unsigned tx = 0; //(0xF1C40000 | (channel << 25));

    SPI_Transfer2(spi, tx);
}

//get ADC conversion result and configure for next conversion
unsigned short getResult_AD7689(spi_p spi, unsigned)
{
    /*
       uint16_t tx0 = 0xF1C4 | (next_channel << 9);
       uint16_t tx1 = 0;
       uint16_t rc0, rc1;

       //send configuration for next conversion
       //initiate conversion when SS/CNV goes high at end of xfer
       //transfer should start at least 4.2 us (tCONV) after previous xfer ended
       rc0 = SPI_Transfer2(spi, tx0);

       //receive LSB of data.  are these valid?
       //transfer should end within 1.2 us (tDATA) of end of previous xfer
       rc1 = SPI_Transfer2(spi, tx1);

       return rc1;
     */
    /*
    unsigned tx = 0xF1C40000 | (next_channel << 25);
    unsigned rc = 0;

    XStatus s = XSpi_Transfer(spi, (u8*)&tx, (u8*)&rc, 2);
    */
    /*
    uint16_t tx = 0xF1C4 | (next_channel << 9);

    SPI_SetSlaveSelect(spi, 1);
    uint16_t rc = SPI_Transfer2(spi, tx);
    */

//new in burninator?
    uint16_t tx = 0x001;
    uint16_t rc;

    SPI_SetSlaveSelect(spi,1);
    usleep(1);
    for(unsigned j = 0; j<6; j++)
        rc = SPI_Transfer2(spi, tx);
    SPI_SetSlaveSelect(spi,0);

    return rc;
}

void Init_ADS8361(spi_p spi)
{
    uint32_t tx = 3 << 29;

    SPI_SetSlaveSelect(spi,0);

    SPI_Transfer_ADS8361(spi, tx);
    SPI_SetSlaveSelect(spi,1);
}

unsigned getResult_ADS8361(spi_p spi, unsigned next_channel)
{
    uint32_t tx = 3 << 29;
    uint16_t rc;

    if( next_channel == 1 ) {
        SPI_SetSlaveSelect(spi,0);
        rc = SPI_Transfer_ADS8361(spi, tx);
        SPI_SetSlaveSelect(spi,1);
    } else rc = SPI_Transfer_ADS8361(spi, tx);

    return rc;
}

// convert and transfer 'num_channels' from AD7656 ADC evaluation board
unsigned short getResult_AD7656(spi_p spi, unsigned num_channels, short* pValues)
{
    uint16_t tx = 0x0000;

    SPI_SetSlaveSelect(spi,2);
    SPI_SetSlaveSelect(spi,0);
    usleep(3); //data sheet spec t_convert
    for(unsigned j = 0; j<num_channels; j++)
        pValues[j] = (short)SPI_Transfer2(spi, tx);

    //printf("ADC0 value = %i\n ADC1 value = %i\n", unsigned(pValues[0]), unsigned(pValues[1]));
    SPI_SetSlaveSelect(spi,1);
    return pValues[0];
}

// convert and transfer 'num_channels' from AD7656 ADC evaluation board
unsigned short getResult_AD7656i(spi_p spi, unsigned num_channels, int* pValues)
{
    uint16_t tx = 0x0000;

    SPI_SetSlaveSelect(spi,0);
    usleep(3);
    for(unsigned j = 0; j<num_channels; j++)
        pValues[j] = (short)SPI_Transfer2(spi, tx);

    //printf("ADC0 value = %i\n ADC1 value = %i\n", unsigned(pValues[0]), unsigned(pValues[1]));
    SPI_SetSlaveSelect(spi,1);
    return pValues[0];
}

// convert and transfer 'num_channels' from AD7656 ADC evaluation board and add to pValues
unsigned short getAddResult_AD7656(spi_p spi, unsigned num_channels, int* pValues)
{
    uint16_t tx = 0x0000;

    SPI_SetSlaveSelect(spi,0); //the chip select line is wired to trigger a conversion (CONVST)
    usleep(3);
    for(unsigned j = 0; j<num_channels; j++)
        pValues[j] += (short)SPI_Transfer2(spi, tx);

    //printf("ADC0 value = %i\n ADC1 value = %i\n", unsigned(pValues[0]), unsigned(pValues[1]));
    SPI_SetSlaveSelect(spi,1);
    return pValues[0];
}

