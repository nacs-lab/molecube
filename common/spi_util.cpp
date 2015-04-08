#include "spi_util.h"

#include <nacs-utils/log.h>

void
SPI_init(XSpi *spi, unsigned id)
{
    if (XSpi_Initialize(spi, uint16_t(id)) == XST_SUCCESS) {
        nacsInfo("spi<%d> initialized successfully\n", id);
        nacsInfo("spi<%d> base address: %p\n", id, spi->BaseAddr);
    } else {
        nacsError("spi<%d> failed to initialize\n", id);
    }

    /*
     * Set the Spi device as a master, and toggle SSELECT manually.
     */
    XSpi_SetOptions(spi, (XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION |
                          XSP_CLK_ACTIVE_LOW_OPTION));
    XSpi_Start(spi);

    /*
     * Disable Global interrupt to use polled mode operation
     */
    XSpi_IntrGlobalDisable(spi);

    // turn off inhibit
    XSpi_SetControlReg(spi, (XSpi_GetControlReg(spi) &
                             ~XSP_CR_TRANS_INHIBIT_MASK));
}
