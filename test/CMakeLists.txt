add_subdirectory(utils)
add_subdirectory(pulser)

set(test_fcgi_SOURCES test_fcgi.cpp)
add_executable(test-fcgi ${test_fcgi_SOURCES})
target_link_libraries(test-fcgi fcgi fcgi++ nacs-utils)

set(test_phy_addr_SOURCES test_phy_addr.cpp)
add_executable(test-phy_addr ${test_phy_addr_SOURCES})
target_link_libraries(test-phy_addr nacs-utils)
