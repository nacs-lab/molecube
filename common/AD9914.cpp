#include "AD9914.h"

#include <nacs-pulser/controller.h>
#include <nacs-utils/log.h>
#include <random>
#include <thread>
#include <chrono>

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

//Initialize the DDS.
//  force = true: always init
//           false: init only if not previopusly initialized
// return true if init was performed
bool
init(Controller &ctrl, int i, bool force)
{
    const unsigned magic_bytes = 0xf00f0000;
    if (!force) {
        // Check if magic bytes have been set (profile 7, FTW) which is
        // otherwise not used.  If already set, the board has been initialized
        // and doesn't need another init.  This avoids reboot-induced glitches.

        uint32_t u0 = ctrl.run(DDSGetFourBytes(i, 0x64));
        nacsLog("AD9914 board=%i  FTW7 = %08X\n", i, u0);
        if (u0 == magic_bytes) {
            nacsLog("No initialization required\n");
            return false;
        }
        nacsLog("Initialization required\n");
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
    ctrl.run(DDSSetTwoBytes(i, 0x0, 0x0108));

    // zero-out all other memory
    for (unsigned addr = 0x10;addr <= 0x6a;addr += 2) {
        ctrl.run(DDSSetTwoBytes(i, addr, 0x0));
    }

    ctrl.run(DDSSetFourBytes(i, 0x64, magic_bytes));

    nacsLog("Initialized AD9914 board=%i\n", i);
    return true;
}

}

void
print_AD9914_registers(Pulser::OldPulser &pulser, int i)
{
    bool bNonZeroOnly = true;

    nacsLog("*******************************\n");
    if (bNonZeroOnly) {
        nacsLog("***only show non-zero values***\n");
    }

    for (unsigned addr = 0;(addr + 3) <= 0x7F;addr += 4) {
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
//  force = true: always init
//           false: init only if not previopusly initialized
// return true if init was performed
bool
init_AD9914(Pulser::OldPulser &pulser, int i, bool force)
{
    const unsigned magic_bytes = 0xf00f0000;
    if (!force) {
        // Check if magic bytes have been set (profile 7, FTW) which is
        // otherwise not used.  If already set, the board has been initialized
        // and doesn't need another init.  This avoids reboot-induced glitches.

        unsigned u0 = pulser.get_dds_four_bytes(i, 0x64);
        nacsLog("AD9914 board=%i  FTW7 = %08X\n", i, u0);
        if (u0 == magic_bytes) {
            nacsLog("No initialization required\n");
            return false;
        }
        nacsLog("Initialization required\n");
    }

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
    return true;
}

}
