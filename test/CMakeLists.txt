set(test_pulser_program_SOURCES test_pulser_program.cpp)

add_executable(test-pulser-program ${test_pulser_program_SOURCES})

target_link_libraries(test-pulser-program nacs-utils nacs-xspi nacs-pulser)

set(test_self_SOURCES test_self.cpp)

add_executable(test-self ${test_self_SOURCES})

target_link_libraries(test-self nacs-utils nacs-xspi nacs-pulser)

set(test_fcgi_SOURCES test_fcgi.cpp)

add_executable(test-fcgi ${test_fcgi_SOURCES})

target_link_libraries(test-fcgi fcgi fcgi++ nacs-utils nacs-xspi nacs-pulser)
