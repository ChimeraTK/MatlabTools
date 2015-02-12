CFLAGS=$$CFLAGS -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x
CXXFLAGS=$$CXXFLAGS -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x
LDFLAGS=$$LDFLAGS -w -std=c++0x

#MTCA_INCLUDE_FLAG = -I/home/mheuer/mtca4u/mtca4u_MappedDevice/include/ 
#MTCA_LIB_FLAG = -L/home/mheuer/mtca4u/mtca4u_MappedDevice/lib/

MTCA_INCLUDE_FLAG = -I/usr/include/mtca4u/
MTCA_LIB_FLAG = -L/usr/lib/mtca4u/

DIR = $(CURDIR)


all:
	mex CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' -I./include $(MTCA_INCLUDE_FLAG) $(MTCA_LIB_FLAG) -lMtcaMappedDevice -outdir bin ./src/mtca4u_mex.cpp
#	mex CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' -I./include $(MTCA_INCLUDE_FLAG) $(MTCA_LIB_FLAG) -lMtcaMappedDevice -lMotorDriverCard -outdir bin ./src/mtca4u_dmc2.cpp
#	cp -r ./matlab/* ./bin/	
#	echo g++ -L/usr/lib $(MTCA_INCLUDE_FLAG) $(MTCA_LIB_FLAG) -lMtcaMappedDevice -o ./unit_test/test.out

#debug:
#	mex -v CFLAGS='$$CFLAGS' CXXFLAGS='$$CXXFLAGS' LDFLAGS='$$LDFLAGS' $(MTCA_INCLUDE_FLAG) $(MTCA_LIB_FLAG) -lMtcaMappedDevice -outdir bin ./src/mtca4u.cpp -D__MEX_DEBUG_MODE
	
.PHONY: clean install install_local test 
	
clean:	
	rm -rf ./bin

install:
	cd ~ && matlab -nojvm -r "cd $(DIR)/bin, run setup.m, savepath ~/pathdef.m, exit"
	@echo "Path saved in ~/pathdef.m" 

install_local:
	cp ./bin/mtca4u_mex.mexa64 /usr/lib/mtca4u_mex.mexa64
	cp ./bin/mtca4u.m /usr/lib/mtca4u.m

#test:
#	matlab -nojvm -r "try run('./unit_test/run_test.m'), catch ex, disp(ex.message); exit; end, exit"
	
