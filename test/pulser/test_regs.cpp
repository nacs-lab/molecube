#include <nacs-utils/timer.h>
#include <nacs-pulser/controller.h>

#include <stdint.h>

#include <iostream>
#include <cmath>

using namespace NaCs;

static auto base_addr = Pulser::mapPulserAddr();

void
print_reg(uint32_t reg, uint32_t val)
{
    Pulser::mWriteSlaveReg(base_addr, reg, val);
    auto read_val = Pulser::mReadSlaveReg(base_addr, reg);
    std::cout << "Register " << reg << ": write " << val
              << ", read " << read_val << std::endl;
}

int
main()
{
    for (auto i = 0;i < 32;i++) {
        print_reg(i, 0);
        print_reg(i, 1 << 16);
    }
    return 0;
}
