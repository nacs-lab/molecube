#include "spi_util.h"

#include <nacs-utils/log.h>

#ifndef NO_XSPI

#include <xspi_l.h>
#include <endian.h>

#ifdef __arm__
//Zynq uses Xil_, Virtex/PPC uses XIo_.
#define XIo_In16  Xil_In16
#define XIo_In32  Xil_In32
#define XIo_Out16 Xil_Out16
#define XIo_Out32 Xil_Out32
#else
//Deal with macro renaming at version 3.00a of xspi.h
//Thank you, Xilinx!
#define XSpi_SetSlaveSelectReg XSpi_mSetSlaveSelectReg
#define XSpi_IntrGlobalDisable XSpi_mIntrGlobalDisable
#define XSpi_GetControlReg XSpi_mGetControlReg
#define XSpi_SetControlReg XSpi_mSetControlReg
#define XSpi_GetStatusReg XSpi_mGetStatusReg
#endif

unsigned SPI_Transmit16(spi_p spi, unsigned short *dataTX,
                        unsigned short *dataRC)
{
    unsigned short tx2 = htobe16(*dataTX);
    unsigned short rc2 = 0;

    uint8_t* tx = (uint8_t*)(&tx2);
    uint8_t* rc = (uint8_t*)(&rc2);

    if (g_debug_spi)
        printf("spi <- tx =%08x\n", (unsigned)*dataTX);

    SPI_SetSlaveSelect(spi, 1);
    XStatus s = XSpi_Transfer(spi, tx, rc, 2);
    SPI_SetSlaveSelect(spi, 0);

    *dataRC = be16toh(rc2);

    if (g_debug_spi)
        printf("spi -> rc =%08x\n", (unsigned)*dataRC);

    return s;
}

unsigned SPI_Transmit(spi_p spi, unsigned* dataTX, unsigned* dataRC, unsigned nBytes)
{

    uint8_t* tx = (uint8_t*)dataTX;
    uint8_t* rc = (uint8_t*)dataRC;

    if (g_debug_spi)
        printf("spi <- tx =%08x\n", *dataTX);

    SPI_SetSlaveSelect(spi, 1);
    XStatus s = XSpi_Transfer(spi, tx, rc, nBytes);
    SPI_SetSlaveSelect(spi, 0);
    /*
      switch(s){
      case XST_DEVICE_IS_STOPPED : printf("SPI failure.  XST_DEVICE_IS_STOPPED\n"); break;
      case XST_DEVICE_BUSY : printf("SPI failure.  XST_DEVICE_BUSY\n"); break;
      case XST_SPI_NO_SLAVE : printf("SPI failure.  XST_SPI_NO_SLAVE\n"); break;
      }
    */
    if (g_debug_spi)
        printf("spi -> rc =%08x\n", *dataRC);

    return s;
}


//optimized copy of the Xilinx driver function
int SPI_SetSlaveSelect(spi_p  InstancePtr, unsigned SlaveMask)
{
    /*
     * A single slave is either being selected or the incoming SlaveMask is
     * zero, which means the slave is being deselected. Setup the value to
     * be  written to the slave select register as the inverse of the slave
     * mask.
     */

    InstancePtr->SlaveSelectReg = ~SlaveMask;
    XSpi_SetSlaveSelectReg(InstancePtr, InstancePtr->SlaveSelectReg);

    return XST_SUCCESS;
}

int
SPI_init(spi_p spi, unsigned id, bool bActiveLow, char clockPhase)
{
    int s = XSpi_Initialize(spi, id);

    //not sure why the driver sometimes returns XST_DEVICE_IS_STARTED on startup
    if (XST_DEVICE_IS_STARTED == s) {
        nacsLog("spi<%d> already started. Stopping...\n", id);

        //Xilinx docs say to stop device and re-initialize
        XSpi_Stop(spi);
        s = XSpi_Initialize(spi, id);
    }
    if (XST_SUCCESS == s) {
        nacsInfo("spi<%d> initialized successfully\n", id);
        nacsInfo("spi<%d> base address: %p\n", id, spi->BaseAddr);

        if (bActiveLow) {
            nacsInfo("spi<%d> active low\n", id);
        } else {
            nacsInfo("spi<%d> active high\n", id);
        }

        nacsInfo("spi<%d> clock phase = %d\n", id, (int)clockPhase);
    } else {
        nacsError("spi<%d> failed to initialize\n", id);
    }

    /*
     * Set the Spi device as a master, and toggle SSELECT manually.
     */
    unsigned options = XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION;

    // unsigned options = XSP_MASTER_OPTION;

    if (bActiveLow) //Clock idles high
        options |= XSP_CLK_ACTIVE_LOW_OPTION;

    if (clockPhase) //Data is valid on 2nd clock edge
        options |= XSP_CLK_PHASE_1_OPTION;

    XSpi_SetOptions(spi, options);
    s = XSpi_Start(spi);

    /*
     * Disable Global interrupt to use polled mode operation
     */
    XSpi_IntrGlobalDisable(spi);

    //turn off inhibit
    uint16_t ControlReg;
    ControlReg = XSpi_GetControlReg(spi);
    ControlReg &= ~XSP_CR_TRANS_INHIBIT_MASK;
    XSpi_SetControlReg(spi,  ControlReg);

    // SPI_SetSlaveSelect(spi, 1);

    return s;
}

/******************************************************************************/
uint16_t SPI_Transfer2(spi_p InstancePtr, uint16_t tx)
{
    // printf("SPI_Transfer2\n");

    uint16_t rcv;
    uint8_t StatusReg;

    /*
     * Set the busy flag, which will be cleared when the transfer
     * is completely done.
     */
    InstancePtr->IsBusy = true;


    // TX should be empty from previous transmission
    // transfer data to SPI transmitter
    XIo_Out16(InstancePtr->BaseAddr + XSP_DTR_OFFSET - 1, tx);


    /*
     * Wait for the transfer to be done by polling the transmit
     * empty status bit
     */
    do {
        StatusReg = XSpi_GetStatusReg(InstancePtr);
    } while ((StatusReg & XSP_SR_TX_EMPTY_MASK) == 0);


    do {
        rcv = XIo_In16(InstancePtr->BaseAddr + XSP_DRR_OFFSET - 1);
        StatusReg = XSpi_GetStatusReg(InstancePtr);
    } while ((StatusReg & XSP_SR_RX_EMPTY_MASK) == 0);

    InstancePtr->IsBusy = false;

    return rcv;
}

uint16_t SPI_Transfer_ADS8361(spi_p  InstancePtr, unsigned tx)
{
    uint32_t rcv;
    uint16_t out;
    uint8_t StatusReg;

    /*
     * Set the busy flag, which will be cleared when the transfer
     * is completely done.
     */
    InstancePtr->IsBusy = true;

    //TX should be empty from previous transmission
    //transfer data to SPI transmitter
    XIo_Out32(InstancePtr->BaseAddr + XSP_DTR_OFFSET - 3, tx);

    /*
     * Wait for the transfer to be done by polling the transmit
     * empty status bit
     */
    do {
        StatusReg = XSpi_GetStatusReg(InstancePtr);
    } while ((StatusReg & XSP_SR_TX_EMPTY_MASK) == 0);

    do {
        rcv = XIo_In32(InstancePtr->BaseAddr + XSP_DRR_OFFSET - 3);
        StatusReg = XSpi_GetStatusReg(InstancePtr);
    } while ((StatusReg & XSP_SR_RX_EMPTY_MASK) == 0);

    InstancePtr->IsBusy = false;

    out = (rcv >> 12) + 32768;

    return out;
}

void Spi_Transfer(spi_p spi, uint8_t* tx, uint8_t* rc, unsigned nBytes)
{
    XSpi_Transfer(spi, tx, rc, nBytes);
}
#endif //NO_XSPI
