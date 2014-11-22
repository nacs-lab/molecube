#include "fpga.h"
#include "linux_file_util.h"

/*Hide real GPIO IDs from user.
 *User accesses inputs GPIO 0...(NGPIO_IN-1)
 *outputs GPIO 0...(NGPIO_OUT-1)
 */

#define NGPIO_IN   2
#define NGPIO_OUT  1

const int gpio_inputs[NGPIO_IN] = {12, 14};
const int gpio_outputs[NGPIO_OUT] = {10};

/* GPIO specific function */

// export GPIO. call this before all other gpio functions
static int
gpio_export(int channel)
{
    return write_int_to_file("/sys/class/gpio/export", channel);
}

// unexport GPIO. call this at shutdown
static int
gpio_unexport(int channel)
{
    return write_int_to_file("/sys/class/gpio/unexport", channel);
}

// dir = 0 for in, dir = 1 for out
static int
gpio_set_dir(int channel, int dir)
{
    char buff[100];
    sprintf(buff, "/sys/class/gpio/gpio%d/direction", channel);
    return write_str_to_file(buff, dir == 0 ? "in" : "out");
}

int
gpio_set_pin(int channel, int val)
{
    char buff[100];
    sprintf(buff, "/sys/class/gpio/gpio%d/value", channel);
    return write_int_to_file(buff, val);
}

static int
gpio_get_pin(int channel)
{
    char buff[100];
    sprintf(buff, "/sys/class/gpio/gpio%d/value", channel);

    int val = -1;
    get_int_from_file(buff, &val);
    return val;
}

/* functions called from elsewhere */
void
init_gpio()
{
    for (unsigned i = 0;i < NGPIO_IN;i++) {
        gpio_export(gpio_inputs[i]);
        gpio_set_dir(gpio_inputs[i], 0);
    }

    for(unsigned i = 0;i < NGPIO_OUT;i++) {
        gpio_export(gpio_outputs[i]);
        gpio_set_dir(gpio_outputs[i], 1);
    }
}

void
close_gpio()
{
    for (unsigned i = 0;i < NGPIO_IN;i++) {
        gpio_unexport(gpio_inputs[i]);
    }
    for(unsigned i = 0;i < NGPIO_OUT;i++) {
        gpio_unexport(gpio_outputs[i]);
    }
}

int
read_gpio(unsigned channel)
{
    if (channel < NGPIO_IN) {
        return gpio_get_pin(gpio_inputs[channel]);
    }
    return -1;
}
