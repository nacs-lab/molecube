#ifndef FPGA_H
#define FPGA_H

namespace NaCs {
namespace GPIO {

void init();
void close();
int set_pin(int channel, int val);
int read(unsigned channel);

}
}

#endif // FPGA_H
