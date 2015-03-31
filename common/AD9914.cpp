#include "AD9914.h"

#include <nacs-pulser/commands.h>
#include <nacs-utils/log.h>
#include <random>
#include <thread>
#include <chrono>

using namespace std::literals;

namespace NaCs {

// const double ad9914_clk_MHz = 3500.0;

static bool
test_val_AD9914(Pulser::OldPulser &pulser, int i, unsigned addr, unsigned val)
{
    pulser.add(Pulser::DDSSetTwoBytes(i, addr, val));
    return pulser.get_dds_two_bytes(i, addr) == val;
}

bool
test_AD9914(Pulser::OldPulser &pulser, int i)
{
    unsigned addr = 0x34; //FTW for profile 1, shouldn't affect the signal
    bool pass = true;

    for (unsigned bit = 0;bit < 16;bit++) {
        unsigned val = 1 << bit;

        if (!test_val_AD9914(pulser, i, addr, val)) {
            pass = false;
            nacsError("Error on DDS %2d at bit %2d (addr = 0x%02X)\n",
                      i, bit, addr);
        }
    }

    pulser.add(Pulser::DDSSetTwoBytes(i, addr, 0));

    if (pass) {
        nacsLog("DDS %d passed digital I/O test.\n", i);
    } else {
        nacsError("DDS %d failed digital I/O test.\n", i);
    }
    return pass;
}

void
test_dds_addr(Pulser::OldPulser &pulser, int i, unsigned low_addr,
              unsigned high_addr, unsigned ntest, FILE *f)
{
    static std::random_device rd;
    static std::uniform_int_distribution<unsigned> dist(0, 0xFF);
    unsigned nerrors = 0;
    unsigned ntested = 0;

    for (unsigned addr = low_addr;addr <= high_addr;addr++) {
        unsigned d0 = pulser.get_dds_byte(i, addr);
        unsigned d1 = pulser.get_dds_byte(i, addr + 1);

        for(unsigned j = 0;j < ntest;j++) {
            unsigned r = dist(rd);
            pulser.add(Pulser::DDSSetTwoBytes(i, addr, r));
            unsigned b = pulser.get_dds_byte(i, addr);

            if (r != b) {
                fprintf(f, "Error on DDS %d, address 0x%02x.  "
                        "Wrote 0x%02X.  Read 0x%02X.\n", i, addr, r, b);
                nerrors++;
                if (nerrors > 100) {
                    break;
                }
            }
            ntested++;
        }

        pulser.add(Pulser::DDSSetTwoBytes(i, addr, d0 | (d1 << 8)));
    }

    fprintf(f, "Tested %d read/writes in address range 0x%02x to 0x%02x.  "
            "%d errors.\n", ntested, low_addr, high_addr, nerrors);
}

void
print_AD9914_registers(Pulser::OldPulser &pulser, int i)
{
    bool bNonZeroOnly = true;

    nacsLog("*******************************\n");
    if (bNonZeroOnly) {
        nacsLog("***only show non-zero values***\n");
    }

    for (unsigned addr = 0;(addr + 3) <= 0x7F; addr += 4) {
        unsigned u0 = pulser.get_dds_two_bytes(i, addr);
        unsigned u2 = pulser.get_dds_two_bytes(i, addr + 2);
        unsigned u = ((u2 & 0xFFFF) << 16 ) | (u0 & 0xFFFF);

        if ((bNonZeroOnly && u) || !bNonZeroOnly) {
            nacsLog("AD9914 board=%i addr=0x%02X...%02X = %08X\n",
                    i, addr + 3, addr, u);
        }
    }
    nacsLog("*******************************\n");
}

//Initialize the DDS.
//  bForce = true: always init
//           false: init only if not previopusly initialized
// return true if init was performed
bool
init_AD9914(Pulser::OldPulser &pulser, int i, bool bForce)
{
    bool bInit = bForce; //init needed?
    const unsigned magic_bytes = 0xF00F0000;

    if (!bForce) {

        //Check if magic bytes have been set (profile 7, FTW) which is otherwise
        //not used.  If already set, the board has been initialized and doesn't
        //need another init.  This avoids reboot-induced glitches.

        unsigned u0 = pulser.get_dds_four_bytes(i, 0x64);
        bInit = (u0 != magic_bytes);

        nacsLog("AD9914 board=%i  FTW7 = %08X\n", i, u0);
        if (bInit) {
            nacsLog("Initialization required\n");
        } else {
            nacsLog("No initialization required\n");
        }
    }

    if (bInit) {
        pulser.dds_reset(i);

        // calibrate internal timing.  required at power-up
        pulser.add(Pulser::DDSSetTwoBytes(i, 0x0E, 0x0105));
        std::this_thread::sleep_for(1ms);
        // finish cal. disble sync_out
        pulser.add(Pulser::DDSSetTwoBytes(i, 0x0E, 0x0405));

        // enable programmable modulus and profile 0, enable SYNC_CLK output
        // pulser.add(Pulser::DDSSetTwoBytes(i, 0x05, 0x8D0B));

        // disable programmable modulus, enable profile 0,
        // enable SYNC_CLK output
        // pulser.add(Pulser::DDSSetTwoBytes(i, 0x05, 0x8009));

        // disable ramp & programmable modulus, enable profile 0,
        // disable SYNC_CLK output
        // pulser.add(Pulser::DDSSetTwoBytes(i, 0x05, 0x8001));

        // disable SYNC_CLK output
        pulser.add(Pulser::DDSSetTwoBytes(i, 0x04, 0x0100));

        // enable ramp, enable programmable modulus, disable profile mode
        // pulser.add(Pulser::DDSSetTwoBytes(i, 0x06, 0x0009));

        // disable ramp, disable programmable modulus, enable profile mode
        pulser.add(Pulser::DDSSetTwoBytes(i, 0x06, 0x0080));

        // enable amplitude control (OSK)
        pulser.add(Pulser::DDSSetTwoBytes(i, 0x0, 0x0108));

        // zero-out all other memory
        for (unsigned addr = 0x10;addr <= 0x6a;addr += 2) {
            pulser.add(Pulser::DDSSetTwoBytes(i, addr, 0x0));
        }

        pulser.add(Pulser::DDSSetFourBytes(i, 0x64, magic_bytes));

        nacsLog("Initialized AD9914 board=%i\n", i);
    }

    return bInit;
}

}
