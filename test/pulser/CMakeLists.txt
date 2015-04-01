set(test_latency_SOURCES test_latency.cpp)
add_executable(test-latency ${test_latency_SOURCES})
target_link_libraries(test-latency nacs-utils nacs-pulser)

set(test_controller_SOURCES test_controller.cpp)
add_executable(test-controller ${test_controller_SOURCES})
target_link_libraries(test-controller nacs-utils nacs-pulser)

set(test_short_pulse_SOURCES test_short_pulse.cpp)
add_executable(test-short_pulse ${test_short_pulse_SOURCES})
target_link_libraries(test-short_pulse nacs-utils nacs-pulser)

set(test_controller_threads_SOURCES test_controller_threads.cpp)
add_executable(test-controller_threads ${test_controller_threads_SOURCES})
target_link_libraries(test-controller_threads nacs-utils nacs-pulser)

set(test_dds_echo_SOURCES test_dds_echo.cpp)
add_executable(test-dds_echo ${test_dds_echo_SOURCES})
target_link_libraries(test-dds_echo nacs-utils nacs-pulser)

set(test_pulser_program_SOURCES test_pulser_program.cpp)
add_executable(test-pulser-program ${test_pulser_program_SOURCES})
target_link_libraries(test-pulser-program nacs-utils nacs-pulser)
