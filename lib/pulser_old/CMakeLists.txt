set(nacs_pulser_SRCS
  pulser.cpp
  program.cpp
  sequence.cpp)
set(nacs_pulser_LINKS nacs-utils nacs-pulser pthread)

add_library(nacs-old-pulser SHARED ${nacs_pulser_SRCS})

target_link_libraries(nacs-old-pulser ${nacs_pulser_LINKS})

set_target_properties(nacs-old-pulser PROPERTIES
  VERSION 0.1
  SOVERSION 0
  COMPILE_FLAGS "-fvisibility=hidden"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS nacs-old-pulser LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
