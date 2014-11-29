set(SOURCES test_pulser_program.cpp)

add_executable(test-pulser-program ${SOURCES})

target_link_libraries(test-pulser-program rt cgicc fcgi fcgi++
  nacs-utils nacs-xspi nacs-pulser)
