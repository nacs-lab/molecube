add_definitions(-DNSPI=2)

# Current directory
include_directories(include zynq_linux)

set(SOURCES
  parser/parseMisc.cpp
  parser/parseTxtSeq.cpp
  parser/saveloadmap.cpp
  zynq_linux/init_platform.cpp
  zynq_linux/init_system.cpp
  zynq_linux/main.cpp)

# Common
include_directories(../common)

set(SOURCES ${SOURCES}
  ../common/CmdLineArgs.cpp
  # ../common/dac.cpp
  ../common/linux_file_util.c
  ../common/dds_pulse.cpp
  ../common/verbosity.cpp
  ../common/spi_util.cpp
  # ../common/dds_pulse_info.cpp
  ../common/fpga.c
  ../common/AD9914.cpp
  # ../common/self_test.cpp
  ../common/string_func.cpp)

# Drivers
include_directories(../drivers)

set(SOURCES ${SOURCES}
  ../drivers/pulse_controller.c)

add_executable(molecube ${SOURCES})

target_link_libraries(molecube rt cgicc fcgi fcgi++
  nacs-utils nacs-xspi nacs-pulser)

install(TARGETS molecube DESTINATION "${CMAKE_INSTALL_BINDIR}")
