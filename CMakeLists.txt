cmake_minimum_required(VERSION 2.8)
project(DeviceAccess_Matlab)

set(${PROJECT_NAME}_MAJOR_VERSION 00)
set(${PROJECT_NAME}_MINOR_VERSION 04)
set(${PROJECT_NAME}_PATCH_VERSION 17)

## Set additional version-specific variables
include(${CMAKE_SOURCE_DIR}/cmake/set_version_numbers.cmake)

SET(MATLAB_ROOT $ENV{MATLAB_ROOT} CACHE FILEPATH "directory")

add_definitions(/DMATLAB_MEX_FILE)
#add_definitions(/DMX_COMPAT_32)

find_package(Matlab REQUIRED)

include(cmake/add_dependency.cmake)
add_dependency(ChimeraTK-DeviceAccess 01.00 REQUIRED)

configure_file(cmake/version.h.in
  "${PROJECT_BINARY_DIR}/include/version.h" @ONLY)
include_directories(${PROJECT_BINARY_DIR}/include)

IF(MATLAB_FOUND)
    message(STATUS "MATLAB Found, MATLAB MEX will be compiled.")
#	
#	find_package(ZED 2 REQUIRED)
#
#    find_package(CUDA REQUIRED)
#    find_package(OpenCV COMPONENTS core highgui imgproc REQUIRED)
#
#    include_directories(${EIGEN3_INCLUDE_DIR})
#    include_directories(include)
#
#    add_subdirectory(mex)
ELSE(MATLAB_FOUND)
	message( FATAL_ERROR "MATLAB not found...nothing will be built." )	
ENDIF(MATLAB_FOUND)
