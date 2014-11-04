#Makefile for Zynq / Linux

ROOT = .
COMMON = ../common_swsrc
XILINXINC = $(COMMON)/xilinx
DRIVERS = ../customIP/IP/drivers/pulse_controller_v4_00_b/src
FPGALIB = $(ROOT)/../lib
FPGAINC = $(ROOT)/../include

CROSS_COMPILE := arm-xilinx-linux-gnueabi-
CXX = $(CROSS_COMPILE)g++
CC = $(CROSS_COMPILE)gcc
RM := rm -rf

INC = -I$(ROOT)/include \
      -I$(COMMON) \
      -I$(COMMON)/linux \
      -I$(XILINXINC) \
      -I$(ROOT)/platform/zynq_linux \
      -I$(ROOT)/platform/zynq_linux/xilinx \
      -I$(DRIVERS) \
      -I$(FPGAINC)

CFLAGS += -pthread -std=gnu11 -O2 -g -DPLATFORM_ZYNQ -DLINUX_OS $(INC) -Wall -Wextra -flto
CXXFLAGS += -pthread -std=c++11 -O2 -g -DPLATFORM_ZYNQ -DLINUX_OS $(INC) -Wall -Wextra -flto

MC: CXXFLAGS += -DCONFIG_BB -DNSPI=2 -DNDDS=22 -DAD9914_CLK=3.5e9 -DNAD9914=8 -DN_SHIFT_SWITCHES=0

LIBS := -pthread -L$(FPGALIB) -lrt -lcgicc -lfcgi -lfcgi++ -flto -O2 -Wl,--as-needed

# All of the sources participating in the build are defined here
SRCS  = $(wildcard $(ROOT)/parser/*.cpp)
SRCS += $(wildcard $(ROOT)/platform/zynq_linux/*.cpp)
SRCS += $(wildcard $(COMMON)/*.cpp)

CSRCS = $(wildcard $(ROOT)/platform/zynq_linux/*.c)
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
