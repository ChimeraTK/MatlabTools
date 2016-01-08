MATLAB_ROOT = /usr/local/MATLAB/R2014b

include ../mtca4u_matlab_version
MEXEXT = $(shell $(MATLAB_ROOT)/bin/mexext)

CFLAGS = $$CFLAGS -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x $(DEBUG_FLAGS)
CXXFLAGS = $$CXXFLAGS -ldl -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x $(DEBUG_FLAGS) 
LDFLAGS = $$LDFLAGS -w -std=c++0x $(DEBUG_FLAGS) -ldl
#fixme
export LD_LIBRARY_PATH=$(MATLAB_ROOT)/bin/glnxa64/:$LD_LIBRARY_PATH
#export LD_LIBRARY_PATH=$(MATLAB_ROOT)/bin/glnxa64/:/home/shehzad/work/deviceaccess/build/mtca_local_install/lib/:$LD_LIBRARY_PATH
.PHONY : test test_version.m init_remote.m test_remote_init.m

all : mleval mlcallmex

mleval : ./src/mleval.cpp
	mex -g -f '../opts.sh' CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' $<

mlcallmex : ./src/mlcallmex.cpp
	mex -g -f '../opts.sh' CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' $<

profile : mlcallmex
	valgrind --tool=callgrind --collect-atstart=no --toggle-collect=mexFunction ./bin/mlcallmex ../bin/mtca4u_mex.mexa64 4
	#valgrind --tool=callgrind --collect-atstart=no --toggle-collect=mexFunction ./bin/mleval "m = mtca4u(); m.read('SISL10','BOARD_WORD_FIRMWARE')"

test : mleval test_version.m init_remote.m test_remote_init.m "/dev/mtcadummys0"
	ctest

test_version.m :
	cat test_version.m.in | sed "{s/__MTCA4U_MATLAB_VERSION__/"$(MTCA4U_MATLAB_VERSION)"/}" > $@

init_remote.m :
	cat init_remote.m.in | sed "{s/__USER__/"$(USER)"/}" | sed "{s|__HOME__|"$(HOME)"|}" > $@
	
test_remote_init.m :
	cat test_remote_init.m.in | sed "{s/__USER__/"$(USER)"/}" | sed "{s|__HOME__|"$(HOME)"|}" > $@

"/dev/mtcadummys0" : 
	modprobe mtcadummy

clean : 
	rm -f ./mleval
	rm -f ./mlcallmex 
	rm -f ./callgrind.out.*
	rm -rf *.gcda *.gcno
	rm -rf test_version.m
	rm -rf Testing

debug:
	DEBUG_FLAGS="-O0 --coverage -L../bin -lmtca4u_mex" make all
