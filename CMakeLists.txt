project(molecule)

## Setup
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(PythonMacros)
include(CMakeCMacros)

# Remove rdynamic
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

add_definitions(-Wall -Wextra -g -O2 -pthread -flto -fvisibility=hidden
  -DPLATFORM_ZYNQ -DLINUX_OS)

set(LINKER_FLAGS -Wl,--as-needed -Wl,--no-undefined -pthread -flto -O2
  # FIXME Hack before porting to Archlinux ARM
  -L${PROJECT_SOURCE_DIR}/lib)

cmake_c_add_flags(CMAKE_C_FLAGS -std=gnu11)
cmake_c_add_flags(CMAKE_CXX_FLAGS -std=c++11)
cmake_c_add_flags(CMAKE_EXE_LINKER_FLAGS ${LINKER_FLAGS})
cmake_c_add_flags(CMAKE_STATIC_LINKER_FLAGS ${LINKER_FLAGS})
cmake_c_add_flags(CMAKE_SHARED_LINKER_FLAGS ${LINKER_FLAGS})
cmake_c_add_flags(CMAKE_MODULE_LINKER_FLAGS ${LINKER_FLAGS})

# FIXME Hack before porting to Archlinux ARM
include_directories(include)

add_subdirectory(molecube)
