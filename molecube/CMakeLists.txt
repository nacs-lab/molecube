#

set(SOURCES
  parseMisc.cpp
  parseTxtSeq.cpp
  saveloadmap.cpp
  init_system.cpp
  main.cpp
  CmdLineArgs.cpp
  linux_file_util.cpp
  AD9914.cpp)

add_executable(molecube ${SOURCES})

set_target_properties(molecube PROPERTIES
  LINK_FLAGS "-Wl,--as-needed ${ZMQ_LDFLAGS_OTHER}")

target_link_libraries(molecube rt cgicc fcgi fcgi++
  nacs-utils nacs-pulser nacs-seq
  ${ZMQ_LIBRARIES}
  )

install(TARGETS molecube DESTINATION "${CMAKE_INSTALL_BINDIR}")
