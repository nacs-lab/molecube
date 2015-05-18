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
add_executable(test-pulser_program ${test_pulser_program_SOURCES})
target_link_libraries(test-pulser_program nacs-utils nacs-pulser)

set(test_self_SOURCES test_self.cpp)
add_executable(test-self ${test_self_SOURCES})
target_link_libraries(test-self nacs-utils nacs-pulser)

set(test_program_request_SOURCES test_program_request.cpp)
add_executable(test-program_request ${test_program_request_SOURCES})
target_link_libraries(test-program_request nacs-utils nacs-pulser)

set(test_loopback_SOURCES test_loopback.cpp)
add_executable(test-loopback ${test_loopback_SOURCES})
target_link_libraries(test-loopback nacs-utils nacs-pulser)

set(test_regs_SOURCES test_regs.cpp)
add_executable(test-regs ${test_regs_SOURCES})
target_link_libraries(test-regs nacs-utils nacs-pulser)
