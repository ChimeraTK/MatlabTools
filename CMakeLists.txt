cmake_minimum_required(VERSION 3.5)
project(DeviceAccess_Matlab)

set(${PROJECT_NAME}_MAJOR_VERSION 01)
set(${PROJECT_NAME}_MINOR_VERSION 01)
set(${PROJECT_NAME}_PATCH_VERSION 00)

# Set additional version-specific variables
include(${CMAKE_SOURCE_DIR}/cmake/set_version_numbers.cmake)

if(Matlab_ROOT_DIR)
  set(ENV{PATH} $ENV{PATH}:${Matlab_ROOT_DIR}/bin)
endif()

#Add the local modules directory first in the path. It contains a FindMatlab version
#from CMake 3.15 which solves linkage problems with Matlab versions >= 2018a. Like this
#we can use it already with CMake 3.5. Once we don't use Linuxes with old CMake versions
#any more, the local FindMatlab and this line can be removed.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules;${CMAKE_MODULE_PATH}")

find_package(Matlab COMPONENTS MX_LIBRARY ENG_LIBRARY REQUIRED)

include(cmake/add_dependency.cmake)
add_dependency(ChimeraTK-DeviceAccess 03.00 REQUIRED)

# C++ standard and other required flags come from DeviceAccess
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ChimeraTK-DeviceAccess_CXX_FLAGS} -Wall -Wextra -Wshadow -pedantic -Wuninitialized")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 --coverage -D__MEX_DEBUG_MODE")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

configure_file(cmake/version.h.in "${PROJECT_BINARY_DIR}/include/version.h" @ONLY)
include_directories(${PROJECT_BINARY_DIR}/include)

# We intentionally do not link to the 'correct' set of ChimeraTK-DeviceAccess-LIBRARIES, which
# also include boost_system and boost_thread. Matlab_MX also uses them, in a different version.
# The fact that we don't need them to link shows that we are not using this part of DeviceAccess
# in the matlab bindings, so we are not in trouble. If the bindings are modified and the linking
# fails, be aware that there are possibly very nasty problems ahead if you ignore the compiler
# warning if you just add the libraries to the linker.
matlab_add_mex(NAME mtca4u_mex SRC src/mtca4u_mex.cpp LINK_TO  ChimeraTK-DeviceAccess)
set_target_properties(mtca4u_mex PROPERTIES VERSION ${${PROJECT_NAME}_FULL_LIBRARY_VERSION} SOVERSION ${${PROJECT_NAME}_SOVERSION})

install( TARGETS mtca4u_mex DESTINATION lib )
install( FILES ${PROJECT_SOURCE_DIR}/matlab/mtca4u.m DESTINATION lib )
install( FILES ${PROJECT_SOURCE_DIR}/matlab/mtca4u_interface.m DESTINATION lib )

### The test section ###

enable_testing()

include_directories(${Matlab_INCLUDE_DIRS})

configure_file(test/src/mleval.cpp.in "${PROJECT_BINARY_DIR}/src/mleval.cpp" @ONLY)
add_executable(mleval ${PROJECT_BINARY_DIR}/src/mleval.cpp)
add_executable(mlcallmex test/src/mlcallmex.cpp)

foreach(testExecutable mleval mlcallmex)
  target_link_libraries(${testExecutable} ${Matlab_MX_LIBRARY} ${Matlab_ENG_LIBRARY} ${CMAKE_DL_LIBS} mtca4u_mex)
endforeach()

# prepare the tests that need version/user/directory dependent info
set(USER $ENV{USER})
set(HOME $ENV{HOME})
foreach(testFile test_version.m init_remote.m test_remote_init.m)
  configure_file(test/${testFile}.in  ${PROJECT_BINARY_DIR}/test/${testFile})
endforeach()

# the .m files in test (incl. subdir matlab, excluding src subdir)
file(COPY test DESTINATION ${PROJECT_BINARY_DIR} FILES_MATCHING PATTERN *.m PATTERN src EXCLUDE)
file(COPY test DESTINATION ${PROJECT_BINARY_DIR} FILES_MATCHING PATTERN *.map PATTERN src EXCLUDE)
file(COPY test/dummies.dmap DESTINATION ${PROJECT_BINARY_DIR}/test)
file(COPY matlab DESTINATION ${PROJECT_BINARY_DIR})

ADD_TEST(NAME mex COMMAND "mleval" "run test_mex.m" "-s" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)
ADD_TEST(NAME local_init COMMAND "mleval" "run test_local_init.m" "-s" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)
ADD_TEST(NAME local_open_close COMMAND "mleval" "run test_local_open_close.m" "-s" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)
ADD_TEST(NAME local_version COMMAND "mleval" "run init_local; run test_version.m" "-s" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)
ADD_TEST(NAME local_read COMMAND "mleval" "run init_local; run test_read.m" "-s" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)
ADD_TEST(NAME local_write COMMAND "mleval" "run init_local; run test_write.m" "-s" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)

#ADD_TEST(NAME local_device_info COMMAND "mleval" "run init_local; run test_device_info.m" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)

ADD_TEST(NAME remote_init COMMAND "mleval" "run test_remote_init.m" "-s" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)
#the init remote automatically checks the version. test_version only checks the local version, thus was duplicate
ADD_TEST(NAME remote_read COMMAND "mleval" "run init_remote; run test_read.m" "-s" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)
ADD_TEST(NAME remote_write COMMAND "mleval" "run init_remote; run test_write.m" "-s" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)

# Copy the example so we do not clutter the source directory with temporary files
file(COPY example DESTINATION ${PROJECT_BINARY_DIR})
configure_file(test/print_all_devices_reference.in ${PROJECT_BINARY_DIR}/example/print_all_devices_reference)
ADD_TEST(NAME example COMMAND "mleval" "run ${PROJECT_BINARY_DIR}/example/example.m" "-s" WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/example)

# Set LD_PRELOAD for all tests so the original libstdc++ of the Linux system is used (instead of the version shipped with Matlab)
# This is done only for Matlab R2016b and earlier. For R2019 this must not be done.
if(NOT ${Matlab_VERSION_STRING_INTERNAL} VERSION_GREATER 9.1)
  set_property(TEST mex local_init local_open_close local_version local_read local_write remote_init remote_read remote_write example
               PROPERTY ENVIRONMENT LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libstdc++.so.6)
endif()

include(cmake/enable_code_coverage_report.cmake)
