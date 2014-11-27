#include "AD9914.h"

#include <nacs-utils/log.h>

#include "fpga.h"
#include "dds_pulse.h"

namespace NaCs {

// const double ad9914_clk_MHz = 3500.0;

static void
set_dds_four_bytes(volatile void *base_addr, int i,
                   unsigned addr, unsigned data)
{
    // printf("AD9914 board=%i set byte [0x%02X] = 0x%02X\n", addr, data);

    // put addr in bits 14...9 (maps to DDS opcode_reg[14:9] )?
    unsigned dds_addr = (addr & 0x3F) << 9;
    PULSER_short_pulse(base_addr, 0x1000000F | (i << 4) |
                       (dds_addr << 9), data); // takes 0.3 us
}

void
set_freq_AD9914PM(volatile void *base_addr, int i, unsigned ftw,
                  unsigned a, unsigned b, FILE *f)
{
    if (f) {
        fprintf(f, "AD9914 board=%i set frequency FTW=0x%08X  A=0x%08X  "
                "B=0x%08X\n", i, ftw, a, b);
        fflush(f);
    }

    set_dds_four_bytes(base_addr, i, 0x10, ftw);
    set_dds_four_bytes(base_addr, i, 0x14, b);
    set_dds_four_bytes(base_addr, i, 0x18, a);
    set_dds_four_bytes(base_addr, i, 0x2C, ftw);
}


static bool
test_val_AD9914(volatile void *base_addr, int i, unsigned addr, unsigned val)
{
    PULSER_set_dds_two_bytes(base_addr, i, addr, val);

    return PULSER_get_dds_two_bytes(base_addr, i, addr) == val;
}

bool
test_AD9914(volatile void *base_addr, int i)
{
    unsigned addr = 0x34; //FTW for profile 1, shouldn't affect the signal
    bool pass = true;

    for (unsigned bit = 0;bit<16; bit++) {
        unsigned val = 1 << bit;

        if (!test_val_AD9914(base_addr, i, addr, val)) {
            pass = false;
            nacsError("Error on DDS %2d at bit %2d (addr = 0x%02X)\n",
                      i, bit, addr);
        }
    }

    PULSER_set_dds_two_bytes(base_addr, i, addr, 0);

    if (pass) {
        nacsLog("DDS %d passed digital I/O test.\n", i);
    } else {
        nacsError("DDS %d failed digital I/O test.\n", i);
    }
    return pass;
}

void
test_dds_addr(volatile void* base_addr, int i, unsigned low_addr,
              unsigned high_addr, unsigned ntest, FILE *f)
{
    unsigned nerrors = 0;
    unsigned ntested = 0;

    for (unsigned addr = low_addr;addr <= high_addr;addr++) {
        unsigned d0 = PULSER_get_dds_byte(base_addr, i, addr);
        unsigned d1 = PULSER_get_dds_byte(base_addr, i, addr + 1);

        for(unsigned j = 0;j < ntest;j++) {
            unsigned r = rand() & 0xFF;
            PULSER_set_dds_two_bytes(base_addr, i, addr, r);

            unsigned b = PULSER_get_dds_byte(base_addr, i, addr);

            if( r!= b) {
                fprintf(f, "Error on DDS %d, address 0x%02x.  Wrote 0x%02X."
                        "  Read 0x%02X.\n",
                        i, addr, r, b);
                nerrors++;

                if (nerrors > 100) {
                    break;
                }
            }

            ntested++;
        }

        PULSER_set_dds_two_bytes(base_addr, i, addr, d0 | (d1 << 8));
    }

    fprintf(f, "Tested %d read/writes in address range 0x%02x to 0x%02x."
            "  %d errors.\n",
            ntested, low_addr, high_addr, nerrors);
}

void
print_AD9914_registers(volatile void *base_addr, int i)
{
    bool bNonZeroOnly = true;

    nacsLog("*******************************\n");
    if (bNonZeroOnly) {
        nacsLog("***only show non-zero values***\n");
    }

    for (unsigned addr = 0;(addr + 3) <= 0x7F; addr += 4) {
        unsigned u0 = PULSER_get_dds_two_bytes(base_addr, i, addr);
        unsigned u2 = PULSER_get_dds_two_bytes(base_addr, i, addr + 2);
        unsigned u =  ((u2 & 0xFFFF) << 16 ) | (u0 & 0xFFFF);

        if ((bNonZeroOnly && u) || !bNonZeroOnly)
            nacsLog("AD9914 board=%i addr=0x%02X...%02X = %08X\n",
                    i, addr + 3, addr, u);
    }
    nacsLog("*******************************\n");
}

//Initialize the DDS.
//  bForce = true: always init
//           false: init only if not previopusly initialized
// return true if init was performed

bool init_AD9914(volatile void *base_addr, int i, bool bForce)
{
    bool bInit = bForce; //init needed?
    const unsigned magic_bytes = 0xF00F0000;

    if (!bForce) {

        //Check if magic bytes have been set (profile 7, FTW) which is otherwise
        //not used.  If already set, the board has been initialized and doesn't
        //need another init.  This avoids reboot-induced glitches.

        unsigned u0 = PULSER_get_dds_four_bytes(base_addr, i, 0x64);
        bInit = (u0 != magic_bytes);

        nacsLog("AD9914 board=%i  FTW7 = %08X\n", i, u0);
        if (bInit) {
            nacsLog("Initialization required\n");
        } else {
            nacsLog("No initialization required\n");
        }
    }

    if (bInit) {
        PULSER_dds_reset(base_addr, i);

        //calibrate internal timing.  required at power-up
        PULSER_set_dds_two_bytes(base_addr, i, 0x0E, 0x0105);
        usleep(1000);
        //finish cal. disble sync_out
        PULSER_set_dds_two_bytes(base_addr, i, 0x0E, 0x0405);

        //enable programmable modulus and profile 0, enable SYNC_CLK output
        //set_dds_two_bytes(base_addr, i, 0x05, 0x8D0B);


        //disable programmable modulus, enable profile 0, enable SYNC_CLK output
        //PULSER_set_dds_two_bytes(base_addr, i, 0x05, 0x8009);

        //disable ramp & programmable modulus, enable profile 0, disable SYNC_CLK output
        //PULSER_set_dds_two_bytes(base_addr, i, 0x05, 0x8001);

        //disable SYNC_CLK output
        PULSER_set_dds_two_bytes(base_addr, i, 0x04, 0x0100);

        //enable ramp, enable programmable modulus, disable profile mode
        //PULSER_set_dds_two_bytes(base_addr, i, 0x06, 0x0009);

        //disable ramp, disable programmable modulus, enable profile mode
        PULSER_set_dds_two_bytes(base_addr, i, 0x06, 0x0080);


        //disable programmable modulus and enable profile 0
        //set_dds_byte_AD9914(base_addr, i, 0x06, 0x80);

        //enable amplitude control (OSK)
        PULSER_set_dds_two_bytes(base_addr, i, 0x0, 0x0108);

        //zero-out all other memory
        for (unsigned addr = 0x10;addr <= 0x6a;addr += 2) {
            PULSER_set_dds_two_bytes(base_addr, i, addr, 0x0);
        }

        PULSER_set_dds_four_bytes(base_addr, i, 0x64, magic_bytes);

        nacsLog("Initialized AD9914 board=%i\n", i);
    }

    return bInit;
}

}
