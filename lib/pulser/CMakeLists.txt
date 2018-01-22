set(nacs_pulser_SRCS
  controller.cpp
  driver.cpp
  instruction.cpp)
set(nacs_pulser_LINKS nacs-utils nacs-seq pthread)
configure_file(pulser-config.h.in pulser-config.h @ONLY)

add_definitions(-Ofast)

add_library(nacs-pulser SHARED ${nacs_pulser_SRCS})

target_link_libraries(nacs-pulser ${nacs_pulser_LINKS})

set_target_properties(nacs-pulser PROPERTIES
  VERSION 0.1
  SOVERSION 0
  COMPILE_FLAGS "-fvisibility=hidden"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS nacs-pulser LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
