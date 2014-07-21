#Makefile for Zynq / Linux

ROOT = .
COMMON = ../common_swsrc
XILINXINC = $(COMMON)/xilinx
DRIVERS = ../customIP/IP/drivers/pulse_controller_v4_00_b/src

CXX = arm-xilinx-linux-gnueabi-g++
CC = arm-xilinx-linux-gnueabi-gcc
RM := rm -rf

INC = -I$(ROOT)/include \
      -I$(ROOT)/../../thirdparty \
      -I$(COMMON) \
      -I$(COMMON)/linux \
      -I$(XILINXINC) \
      -I$(ROOT)/platform/zynq_linux \
      -I$(ROOT)/platform/zynq_linux/xilinx \
      -I$(DRIVERS) \
      -I/usr/local/zynq/fcgi/include 
      
CFLAGS   = -O1 -g -DPLATFORM_ZYNQ -DLINUX_OS $(INC)
CXXFLAGS = -Werror -std=c++11 -O3 -g -DPLATFORM_ZYNQ -DLINUX_OS $(INC)

MC: CXXFLAGS += -DCONFIG_BB -DNSPI=2 -DNDDS=22 -DAD9914_CLK=3.5e9 -DNAD9914=8 -DN_SHIFT_SWITCHES=0

LIBS := -Wl,--start-group,-L/usr/local/zynq/fcgi/lib,-lgcc,-lc,-lstdc++,-lrt,-lpthread,-lfcgi,-lfcgi++,--end-group
#LIBS += -Wl,--start-group,-L/usr/local/zynq/curl-7.37.0/lib,-lcurl,--end-group

# All of the sources participating in the build are defined here
SRCS  = $(wildcard $(ROOT)/*.cpp)
SRCS  = $(wildcard $(ROOT)/parser/*.cpp)
SRCS += $(wildcard $(ROOT)/platform/zynq_linux/*.cpp)
SRCS += $(wildcard $(ROOT)/BB/*.cpp)
SRCS += $(wildcard $(COMMON)/*.cpp)

CSRCS  = $(wildcard $(ROOT)/*.c)
CSRCS += $(wildcard $(ROOT)/platform/zynq_linux/*.c)
CSRCS += $(wildcard $(COMMON)/*.c)
CSRCS += $(wildcard $(COMMON)/linux/*.c)
CSRCS += $(wildcard $(XILINXINC)/*.c)
CSRCS += $(wildcard $(DRIVERS)/*.c)

OBJS = $(SRCS:.cpp=.o)
OBJS += $(CSRCS:.c=.o)

default: MC

MC: molecube

molecube SHB barebones: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building: $@'
	@echo
	@echo "**************" Determining Size of ELF File "**************"
	@echo
	arm-xilinx-linux-gnueabi-size $@
		
clean:
	-$(RM) $(OBJS) planAhead*
	
