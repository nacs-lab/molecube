set(test_pulser_program_SOURCES test_pulser_program.cpp)
add_executable(test-pulser-program ${test_pulser_program_SOURCES})
target_link_libraries(test-pulser-program nacs-utils nacs-xspi nacs-pulser)

set(test_self_SOURCES test_self.cpp)
add_executable(test-self ${test_self_SOURCES})
target_link_libraries(test-self nacs-utils nacs-xspi nacs-pulser)

set(test_fcgi_SOURCES test_fcgi.cpp)
add_executable(test-fcgi ${test_fcgi_SOURCES})
target_link_libraries(test-fcgi fcgi fcgi++ nacs-utils nacs-xspi nacs-pulser)

set(test_thread_SOURCES test_thread.cpp)
add_executable(test-thread ${test_thread_SOURCES})
target_link_libraries(test-thread nacs-utils)

set(test_read_thread_SOURCES test_read_thread.cpp)
add_executable(test-read_thread ${test_read_thread_SOURCES})
target_link_libraries(test-read_thread nacs-utils nacs-xspi nacs-pulser)

set(test_bits_SOURCES test_bits.cpp)
add_executable(test-bits ${test_bits_SOURCES})
target_link_libraries(test-bits nacs-utils)
