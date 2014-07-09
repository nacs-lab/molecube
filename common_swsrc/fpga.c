#include "fpga.h"
#include <stdio.h>

#ifdef LINUX_OS

#include "platform/zynq_linux/linux_file_util.h"

/*Hide real GPIO IDs from user.  
 *User accesses inputs GPIO 0...(NGPIO_IN-1)
 *outputs GPIO 0...(NGPIO_OUT-1)
 */

#define NGPIO_IN   2
#define NGPIO_OUT  1

const int gpio_inputs[NGPIO_IN] = {12, 14};
const int gpio_outputs[NGPIO_OUT] = {10};

/* GPIO specific function */

//export GPIO.  call this before all other gpio functions
int gpio_export(int channel)
{
  return write_int_to_file("/sys/class/gpio/export", channel);
}

//unexport GPIO.  call this at shutdown
int gpio_unexport(int channel)
{
  return write_int_to_file("/sys/class/gpio/unexport", channel);
}

// dir = 0 for in, dir = 1 for out
int gpio_set_dir(int channel, int dir)
{
  char buff[100];
  sprintf(buff, "/sys/class/gpio/gpio%d/direction", channel);
  write_str_to_file(buff, dir == 0 ? "in" : "out");
}

int gpio_set_pin(int channel, int val)
{
  char buff[100];
  sprintf(buff, "/sys/class/gpio/gpio%d/value", channel);
  write_int_to_file(buff, val);
}

int gpio_get_pin(int channel)
{
  char buff[100];
  sprintf(buff, "/sys/class/gpio/gpio%d/value", channel);
  
  int val = -1;
  get_int_from_file(buff, &val);
  return val;
}

/* functions called from elsewhere */
void init_gpio()
{
  unsigned i;
  
  for(i=0; i<NGPIO_IN; i++)
  {
    gpio_export(gpio_inputs[i]);
    gpio_set_dir(gpio_inputs[i], 0);
  }
  
  for(i=0; i<NGPIO_OUT; i++)
  {
    gpio_export(gpio_outputs[i]);
    gpio_set_dir(gpio_outputs[i], 1);
  }
}

void close_gpio()
{
  unsigned i;
  
  for(i=0; i<NGPIO_IN; i++)
    gpio_unexport(gpio_inputs[i]);
  
  for(i=0; i<NGPIO_OUT; i++)
    gpio_unexport(gpio_outputs[i]);
}

int read_gpio(unsigned channel)
{
  if(channel < NGPIO_IN)
    return gpio_get_pin(gpio_inputs[channel]);
    
  return -1;
}

#else

#ifdef PLATFORM_ZYNQ

#include "xgpiops.h"
static XGpioPs gpio;

void init_gpio(int debug_level)
{
	int Status;
	XGpioPs_Config *ConfigPtr;

	ConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
	Status = XGpioPs_CfgInitialize(&gpio, ConfigPtr, ConfigPtr->BaseAddr);
  
  if(debug_level > 0)
  {
    if (Status != XST_SUCCESS) {
      printf("[init_gpio] failed to initialize GPIO\n");
    }
    else
      printf("[init_gpio] SUCCESS\n");
    }
  }
}

//EMIO pins start at 54
#define EMIO_0 54

int read_gpio(unsigned channel)
{
	XGpioPs_SetDirectionPin(&gpio, channel+EMIO_0, 0x0);
	return XGpioPs_ReadPin(&gpio, channel+EMIO_0);
}

#else //PLATFORM_ZYNQ

#include "xgpio.h"
static XGpio TTLchan;

void init_gpio()
{
    unsigned DirectionMask = 0xFFFFFFFF;

    if(XST_SUCCESS == XGpio_Initialize(&TTLchan,XPAR_XPS_GPIO_0_DEVICE_ID))  //device ID lives in xparameters.h file
            printf("GPIO Initialized!\n  BaseAddress = %X\n  IsReady = %X\n InterruptPresent = %u\n IsDual = %u\n", TTLchan.BaseAddress, TTLchan.IsReady, TTLchan.InterruptPresent, TTLchan.IsDual);
    else
            printf("GPIO failed to initialize.\n");

    XGpio_SetDataDirection(&TTLchan,1,DirectionMask);
    unsigned dataDir = XGpio_GetDataDirection(&TTLchan, 1);
    printf("DirectionMask = %X\nGPIO Data Direction = %X\n", DirectionMask, dataDir);
}

int read_gpio(unsigned channel)
{
	//each GPIO channel has several bits (pins)
	return XGpio_DiscreteRead(&TTLchan, channel);
}
#endif //PLATFORM_ZYNQ
#endif //LINUX_OS

