#MATLAB_ROOT = /usr/local/MATLAB/R2013b
MATLAB_ROOT = /usr/local/MATLAB/R2014b
#MATLAB_ROOT = /space/nshehzad/work/Matlab/R2013b

include ../mtca4u_matlab_version
MEXEXT = $(shell $(MATLAB_ROOT)/bin/mexext)
MCXXFLAGS=-I$(MATLAB_ROOT)/extern/include/cpp $(MFLAGS)
CFLAGS = $$CFLAGS -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x $(DEBUG_FLAGS)
CXXFLAGS = $$CXXFLAGS -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x $(DEBUG_FLAGS) $(MCXXFLAGS)
LDFLAGS = $$LDFLAGS -w -std=c++0x $(DEBUG_FLAGS) 
LDFLAGS = $$LDFLAGS $$DeviceAccess_LIB_FLAGS -leng -ldl -lm
LINKLIBS = $$LINKLIBS -leng -ldl -lm
LDEXT = ""
LDTYPE = "-exe"  
#alternatively, remove last ldflags definition, and add them directly in the option file. Also export export LD_LIBRARY_PATH=$(MATLAB_ROOT)/bin/glnxa64/:$LD_LIBRARY_PATH
#can be replaced with line below.
#LDFLAGS = $$LDFLAGS -Wl,-rpath-link,$(MATLAB_ROOT)/bin/glnxa64,$$MtcaMappedDevice_LIB_FLAGS:$(MATLAB_ROOT)/bin/glnxa64 -ldl
export LD_LIBRARY_PATH=$(MATLAB_ROOT)/bin/glnxa64/:$LD_LIBRARY_PATH
.PHONY : test test_version.m init_remote.m test_remote_init.m

all : mleval mlcallmex

mleval : ./src/mleval.cpp
	mex -v GCC='g++-4.6' -g  CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' LINKLIBS='$(LINKLIBS)' LDEXT='$(LDEXT)' LDTYPE='$(LDTYPE)' $<

mlcallmex : ./src/mlcallmex.cpp
	mex -v GCC='g++-4.6' -g  CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' LINKLIBS='$(LINKLIBS)' LDEXT='$(LDEXT)' LDTYPE='$(LDTYPE)' $<

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
