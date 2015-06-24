PROJECT(mtca4u_matlab_tools)
cmake_minimum_required(VERSION 2.8)

#LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_PREFIX}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake) 

set(${PROJECT_NAME}_MAJOR_VERSION 00)
set(${PROJECT_NAME}_MINOR_VERSION 03)
set(${PROJECT_NAME}_PATCH_VERSION 00)
set(${PROJECT_NAME}_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})
set(${PROJECT_NAME}_SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION})

#Moderate version of the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wextra -ansi -Wuninitialized")

find_package(Matlab)

if(MATLAB_FOUND)
    message(STATUS "MATLAB Found, MATLAB MEX will be compiled.")
    add_subdirectory(mex)
else(MATLAB_FOUND)
    MESSAGE("MATLAB not found...nothing will be built.")
endif(MATLAB_FOUND)

#FIND_PACKAGE(FindMtcaMappedDevice 00.07.00 REQUIRED)
#turn on the rpath so libMtcaMappedDevice.so is found
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#INCLUDE_DIRECTORIES( ${MtcaMappedDevice_INCLUDE_DIRS} )

include_directories(${MATLAB_INCLUDE_DIR})
add_library(mtca4u MODULE src/mtca4u.c)
target_link_libraries(mtca4u ${MATLAB_LIBRARIES})

#configure a header file which contains the version information for use in C++
configure_file(cmake/version.h.in "${PROJECT_BINARY_DIR}/version.h" @ONLY)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
  install(DIRECTORY ${CMAKE_BINARY_DIR}/doc DESTINATION .)
else(DOXYGEN_FOUND)
  message("Doxygen not found, documentation will not be build.")
endif(DOXYGEN_FOUND)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  )

# The instructions to create a debian package are loaded from a separate file
include(${CMAKE_SOURCE_DIR}/cmake/prepare_debian_package.cmake)
