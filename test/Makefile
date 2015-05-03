MATLAB_ROOT = /net/ttfoptsvr/export/LOCALHOST/opt/matlab_R2013b/

CFLAGS = $$CFLAGS -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x
CXXFLAGS = $$CXXFLAGS -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x
LDFLAGS = $$LDFLAGS -w -std=c++0x

export LD_LIBRARY_PATH=/net/ttfoptsvr/export/LOCALHOST/opt/matlab_R2013b/bin/glnxa64/:$LD_LIBRARY_PATH

TEST_MFILES := $(wildcard ./*.m)

.phony : all profile test clean .FORCE

all : mleval mlcallmex

mleval : ./src/mleval.cpp
	mex -g -f $(MATLAB_ROOT)/bin/engopts.sh CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' $<

mlcallmex : ./src/mlcallmex.cpp
	mex -g -f $(MATLAB_ROOT)/bin/engopts.sh CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' $<
 
profile : mlcallmex
	valgrind --tool=callgrind --collect-atstart=no --toggle-collect=mexFunction ./bin/mlcallmex ../bin/mtca4u_mex.mexa64 4
	#valgrind --tool=callgrind --collect-atstart=no --toggle-collect=mexFunction ./bin/mleval "m = mtca4u(); m.read('SISL10','BOARD_WORD_FIRMWARE')"

test : mleval "/dev/mtcadummys0" $(TEST_MFILES)

%.m : .FORCE
	@printf "Test: $@ \n"
	@./mleval "run $@" -s || { tput setaf 1; echo " ... failed"; tput sgr0; exit 0; }

"/dev/mtcadummys0" : 
	modprobe mtcadummy

.FORCE :

clean : 
	rm ./mleval
	rm ./mlcallmex 
	rm ./callgrind.out.*
