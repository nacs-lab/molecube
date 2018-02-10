//

#include "AD9914.h"

#include <nacs-pulser/controller.h>
#include <nacs-utils/log.h>

using namespace std::literals;

namespace NaCs {
namespace AD9914 {

using namespace Pulser;

void
print_registers(Controller &ctrl, int i)
{
    static constexpr bool nonZeroOnly = true;

    nacsLog("*******************************\n");
    if (nonZeroOnly) {
        nacsLog("***only show non-zero values***\n");
    }

    for (unsigned addr = 0;addr + 3 <= 0x7F;addr += 4) {
        uint32_t u0 = ctrl.run(DDSGetTwoBytes(i, addr));
        uint32_t u2 = ctrl.run(DDSGetTwoBytes(i, addr + 2));
        uint32_t u = ((u2 & 0xffff) << 16 ) | (u0 & 0xffff);

        if (u || !nonZeroOnly) {
            nacsLog("AD9914 board = %i, addr = 0x%02X...%02X = %08X\n",
                    i, addr + 3, addr, u);
        }
    }
    nacsLog("*******************************\n");
}

// Initialize the DDS.
// return true if init was performed
bool
init(Controller &ctrl, int i, InitFlags flags)
{
    const unsigned magic_bytes = 0xf00f0000;
    bool force = flags & Force;
    bool log_verbose = flags & LogVerbose;
    bool log = log_verbose || (flags & LogAction);
    if (!force) {
        // Check if magic bytes have been set (profile 7, FTW) which is
        // otherwise not used.  If already set, the board has been initialized
        // and doesn't need another init.  This avoids reboot-induced glitches.

        uint32_t u0 = ctrl.run(DDSGetFourBytes(i, 0x64));
        if (log_verbose)
            nacsLog("AD9914 board=%i  FTW7 = %08X\n", i, u0);
        if (u0 == magic_bytes) {
            if (log_verbose)
                nacsLog("No initialization required\n");
            return false;
        }
        if (log) {
            nacsLog("Initialization required\n");
        }
    }

    ctrl.run(DDSReset(i));

    // calibrate internal timing.  required at power-up
    ctrl.run(DDSSetTwoBytes(i, 0x0E, 0x0105));
    std::this_thread::sleep_for(1ms);
    // finish cal. disble sync_out
    ctrl.run(DDSSetTwoBytes(i, 0x0E, 0x0405));

    // enable programmable modulus and profile 0, enable SYNC_CLK output
    // ctrl.run(DDSSetTwoBytes(i, 0x05, 0x8D0B));

    // disable programmable modulus, enable profile 0,
    // enable SYNC_CLK output
    // ctrl.run(DDSSetTwoBytes(i, 0x05, 0x8009));

    // disable ramp & programmable modulus, enable profile 0,
    // disable SYNC_CLK output
    // ctrl.run(DDSSetTwoBytes(i, 0x05, 0x8001));

    // disable SYNC_CLK output
    ctrl.run(DDSSetTwoBytes(i, 0x04, 0x0100));

    // enable ramp, enable programmable modulus, disable profile mode
    // ctrl.run(DDSSetTwoBytes(i, 0x06, 0x0009));

    // disable ramp, disable programmable modulus, enable profile mode
    ctrl.run(DDSSetTwoBytes(i, 0x06, 0x0080));

    // enable amplitude control (OSK)
    ctrl.run(DDSSetTwoBytes(i, 0x0, 0x0308));

    // zero-out all other memory
    for (unsigned addr = 0x10;addr <= 0x6a;addr += 2) {
        ctrl.run(DDSSetTwoBytes(i, addr, 0x0));
    }

    ctrl.run(DDSSetFourBytes(i, 0x64, magic_bytes));

    if (log)
        nacsLog("Initialized AD9914 board=%i\n", i);
    return true;
}

}
}
