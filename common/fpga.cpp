#include "fpga.h"
#include "linux_file_util.h"

/*Hide real GPIO IDs from user.
 *User accesses inputs GPIO 0...(NGPIO_IN-1)
 *outputs GPIO 0...(NGPIO_OUT-1)
 */

#define NGPIO_IN   2
#define NGPIO_OUT  1

/* GPIO specific function */

namespace NaCs {
namespace GPIO {

const int inputs[NGPIO_IN] = {12, 14};
const int outputs[NGPIO_OUT] = {10};

// export GPIO. call this before all other gpio functions
static int
export_chn(int channel)
{
    return write_int_to_file("/sys/class/gpio/export", channel);
}

// unexport GPIO. call this at shutdown
static int
unexport_chn(int channel)
{
    return write_int_to_file("/sys/class/gpio/unexport", channel);
}

// dir = 0 for in, dir = 1 for out
static int
set_dir(int channel, int dir)
{
    char buff[100];
    sprintf(buff, "/sys/class/gpio/gpio%d/direction", channel);
    return write_str_to_file(buff, dir == 0 ? "in" : "out");
}

static int
get_pin(int channel)
{
    char buff[100];
    sprintf(buff, "/sys/class/gpio/gpio%d/value", channel);

    int val = -1;
    get_int_from_file(buff, &val);
    return val;
}

void
init()
{
    for (unsigned i = 0;i < NGPIO_IN;i++) {
        export_chn(inputs[i]);
        set_dir(inputs[i], 0);
    }

    for(unsigned i = 0;i < NGPIO_OUT;i++) {
        export_chn(outputs[i]);
        set_dir(outputs[i], 1);
    }
}

void
close()
{
    for (unsigned i = 0;i < NGPIO_IN;i++) {
        unexport_chn(inputs[i]);
    }
    for(unsigned i = 0;i < NGPIO_OUT;i++) {
        unexport_chn(outputs[i]);
    }
}

int
set_pin(int channel, int val)
{
    char buff[100];
    sprintf(buff, "/sys/class/gpio/gpio%d/value", channel);
    return write_int_to_file(buff, val);
}

int
read(unsigned channel)
{
    if (channel < NGPIO_IN) {
        return get_pin(inputs[channel]);
    }
    return -1;
}

}
}
