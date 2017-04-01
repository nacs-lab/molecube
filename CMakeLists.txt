project(molecube)

## Setup
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(PythonMacros)
include(CMakeCMacros)
include(GNUInstallDirs)

set(PULSER_AD9914_CLK 3.5e9)
set(PULSER_NDDS 22)

# Remove rdynamic
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

add_definitions(-Wall -Wextra -g -pthread -fvisibility=hidden
  -fno-math-errno -Wconversion -Wno-sign-conversion)

cmake_c_fix_include_path(lib/pulser nacs-pulser)
cmake_c_fix_include_path("${CMAKE_CURRENT_BINARY_DIR}/lib/pulser"
  nacs-pulser bin)

set(LINKER_FLAGS -Wl,--as-needed -Wl,--no-undefined -Wl,--gc-sections
  -pthread -fno-math-errno)

cmake_c_add_flags(CMAKE_C_FLAGS -std=gnu11)
cmake_c_add_flags(CMAKE_CXX_FLAGS -std=gnu++14)
cmake_c_add_flags(CMAKE_EXE_LINKER_FLAGS ${LINKER_FLAGS})
cmake_c_add_flags(CMAKE_STATIC_LINKER_FLAGS ${LINKER_FLAGS})
cmake_c_add_flags(CMAKE_SHARED_LINKER_FLAGS ${LINKER_FLAGS})
cmake_c_add_flags(CMAKE_MODULE_LINKER_FLAGS ${LINKER_FLAGS})

cmake_c_include_fix_path()
cmake_c_include_fix_path(bin)
add_subdirectory(data)
add_subdirectory(lib)
add_subdirectory(molecube)
add_subdirectory(test)
