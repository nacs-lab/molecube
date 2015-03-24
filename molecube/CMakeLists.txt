add_definitions(-DNSPI=2)

# Current directory
include_directories(include zynq_linux)

set(SOURCES
  parser/parseMisc.cpp
  parser/parseTxtSeq.cpp
  parser/saveloadmap.cpp
  zynq_linux/init_system.cpp
  zynq_linux/main.cpp)

# Common
include_directories(../common)

set(SOURCES ${SOURCES}
  ../common/CmdLineArgs.cpp
  # ../common/dac.cpp
  ../common/linux_file_util.cpp
  ../common/verbosity.cpp
  ../common/spi_util.cpp
  ../common/fpga.cpp
  ../common/AD9914.cpp
  # ../common/self_test.cpp
  ../common/string_func.cpp)

add_executable(molecube ${SOURCES})

target_link_libraries(molecube rt cgicc fcgi fcgi++
  nacs-utils nacs-xspi nacs-old-pulser)

install(TARGETS molecube DESTINATION "${CMAKE_INSTALL_BINDIR}")
