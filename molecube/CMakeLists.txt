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
  ../common/linux_file_util.cpp
  ../common/verbosity.cpp
  ../common/AD9914.cpp)

add_executable(molecube ${SOURCES})

set_target_properties(molecube PROPERTIES
  LINK_FLAGS "-Wl,--as-needed ${ZMQ_LDFLAGS_OTHER}")

target_link_libraries(molecube rt cgicc fcgi fcgi++
  nacs-utils nacs-pulser nacs-seq
  ${ZMQ_LIBRARIES}
  )

install(TARGETS molecube DESTINATION "${CMAKE_INSTALL_BINDIR}")
