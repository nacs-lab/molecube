set(nacs_xspi_SRCS
  xspi_sinit.cpp
  xspi_options.cpp
  xspi.cpp)
set(nacs_xspi_LINKS nacs-utils pthread)

add_library(nacs-xspi SHARED ${nacs_xspi_SRCS})

target_link_libraries(nacs-xspi ${nacs_xspi_LINKS})

set_target_properties(nacs-xspi PROPERTIES
  VERSION 0.1
  SOVERSION 0
  COMPILE_FLAGS "-fvisibility=hidden"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS nacs-xspi LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
