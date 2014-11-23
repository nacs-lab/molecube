add_definitions(-DCONFIG_BB -DNSPI=2 -DNDDS=22 -DAD9914_CLK=3.5e9
  -DNAD9914=8 -DN_SHIFT_SWITCHES=0)

# Current directory
include_directories(include
  zynq_linux)

set(SOURCES
  parser/parseMisc.cpp
  parser/parseTxtSeq.cpp
  parser/saveloadmap.cpp
  zynq_linux/init_platform.cpp
  zynq_linux/init_system.cpp
  zynq_linux/main.cpp)

# Lib
include_directories(../lib/xilinx)

set(SOURCES ${SOURCES}
  ../lib/xilinx/xspi_stats.c
  ../lib/xilinx/xspi_g.c
  ../lib/xilinx/xspi_sinit.c
  ../lib/xilinx/xspi_selftest.c
  ../lib/xilinx/xspi_options.c
  ../lib/xilinx/xspi.c)

# Common
include_directories(../common)

set(SOURCES ${SOURCES}
  ../common/CmdLineArgs.cpp
  ../common/dac.cpp
  ../common/linux_file_util.c
  ../common/dds_pulse.cpp
  ../common/verbosity.cpp
  ../common/spi_util.cpp
  ../common/dds_pulse_info.cpp
  ../common/fpga.c
  ../common/AD9914.cpp
  ../common/self_test.cpp
  ../common/string_func.cpp)

# Drivers
include_directories(../drivers)

set(SOURCES ${SOURCES}
  ../drivers/pulse_controller.c
  ../drivers/pulse_controller_selftest.c)

add_executable(molecube ${SOURCES})

target_link_libraries(molecube rt cgicc fcgi fcgi++ nacs-utils)
