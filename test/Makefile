include ../mtca4u_matlab_version
include ../matlab_root

CXX = g++
LD = g++
MCXXFLAGS=-I$(MATLAB_ROOT)/extern/include
MCXXLIBS=-L$(MATLAB_ROOT)/bin/glnxa64 -leng -lmx -ldl
CFLAGS += -g -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x $(DEBUG_FLAGS)
CXXFLAGS += -g -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x -fPIC $(DEBUG_FLAGS) $(MCXXFLAGS)
LDFLAGS += -std=c++0x $(DEBUG_FLAGS) $(MCXXLIBS) 
export LD_LIBRARY_PATH=$(MATLAB_ROOT)/bin/glnxa64/:$LD_LIBRARY_PATH
.PHONY : test test_version.m init_remote.m test_remote_init.m

all : mleval mlcallmex

mleval : ./src/mleval.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

mlcallmex : ./src/mlcallmex.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

profile : mlcallmex
	valgrind --tool=callgrind --collect-atstart=no --toggle-collect=mexFunction ./bin/mlcallmex ../bin/mtca4u_mex.mexa64 4

test : mleval test_version.m init_remote.m test_remote_init.m "/dev/mtcadummys0"
	ctest

test_version.m :
	cat test_version.m.in | sed "{s/__MTCA4U_MATLAB_VERSION__/"$(MTCA4U_MATLAB_VERSION)"/}" > $@

init_remote.m :
	cat init_remote.m.in | sed "{s/__USER__/"$(USER)"/}" | sed "{s|__HOME__|"$(HOME)"|}" > $@

test_remote_init.m :
	cat test_remote_init.m.in | sed "{s/__USER__/"$(USER)"/}" | sed "{s|__HOME__|"$(HOME)"|}" > $@

../example/example.m :
  $(shell ../example/print_all_devices_reference.in)

"/dev/mtcadummys0" : 
	modprobe mtcadummy

clean : 
	rm -f ./mleval
	rm -f ./mlcallmex 
	rm -f ./callgrind.out.*
	rm -rf *.gcda *.gcno
	rm -rf test_version.m
	rm -rf Testing
	rm -rf ../example/print_all_devices_reference

debug:
	DEBUG_FLAGS="-O0 --coverage -L../bin -lmtca4u_mex" make all
