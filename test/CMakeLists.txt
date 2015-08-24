add_subdirectory(pulser)

set(test_fcgi_SOURCES test_fcgi.cpp)
add_executable(test-fcgi ${test_fcgi_SOURCES})
target_link_libraries(test-fcgi fcgi fcgi++ nacs-utils)
