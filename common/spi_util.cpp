#include "spi_util.h"

#include <nacs-utils/log.h>
#include <nacs-xspi/xspi_l.h>

#include <inttypes.h>

int
SPI_init(XSpi *spi, uint16_t id)
{
    int s = XSpi_Initialize(spi, id);

    if (XST_SUCCESS == s) {
        nacsInfo("spi<%" PRIu16 "> initialized successfully\n", id);
        nacsInfo("spi<%" PRIu16 "> base address: %p\n", id, spi->BaseAddr);
    } else {
        nacsError("spi<%" PRIu16 "> failed to initialize\n", id);
    }

    /*
     * Set the Spi device as a master, and toggle SSELECT manually.
     */
    XSpi_SetOptions(spi, (XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION |
                          XSP_CLK_ACTIVE_LOW_OPTION));
    s = XSpi_Start(spi);

    /*
     * Disable Global interrupt to use polled mode operation
     */
    XSpi_IntrGlobalDisable(spi);

    // turn off inhibit
    XSpi_SetControlReg(spi, (XSpi_GetControlReg(spi) &
                             ~XSP_CR_TRANS_INHIBIT_MASK));

    return s;
}
