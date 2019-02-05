cmake_minimum_required(VERSION 2.8)
project(DeviceAccess_Matlab)

set(${PROJECT_NAME}_MAJOR_VERSION 00)
set(${PROJECT_NAME}_MINOR_VERSION 04)
set(${PROJECT_NAME}_PATCH_VERSION 17)

## Set additional version-specific variables
include(${CMAKE_SOURCE_DIR}/cmake/set_version_numbers.cmake)

find_package(Matlab REQUIRED)

include(cmake/add_dependency.cmake)
add_dependency(ChimeraTK-DeviceAccess 01.00 REQUIRED)

## C++ standard and other required flags come from DeviceAccess
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ChimeraTK-DeviceAccess_CXX_FLAGS} -Wall -fPIC -Wextra -Wshadow -pedantic -Wuninitialized")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 --coverage")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

configure_file(cmake/version.h.in "${PROJECT_BINARY_DIR}/include/version.h" @ONLY)
include_directories(${PROJECT_BINARY_DIR}/include)

matlab_add_mex(NAME mtca4u_mex SRC src/mtca4u_mex.cpp LINK_TO  ${ChimeraTK-DeviceAccess_LIBRARIES})
set_target_properties(mtca4u_mex PROPERTIES VERSION ${${PROJECT_NAME}_FULL_LIBRARY_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})
