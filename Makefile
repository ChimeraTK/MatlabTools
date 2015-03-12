CFLAGS = $$CFLAGS -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x
CXXFLAGS = $$CXXFLAGS -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x
LDFLAGS = $$LDFLAGS -w -std=c++0x

MTCA4U_MATLAB_VERSION=00.01.01

#Set the correct parameters for the MTCA4U include
#You can change the path to MTCA4U.CONFIG if you want to use a custom installation
include /usr/share/mtca4u/MTCA4U.CONFIG

MTCA4U_MEX_FLAGS = $(MtcaMappedDevice_INCLUDE_FLAGS)\
                   $(MtcaMappedDevice_LIB_FLAGS) $(MtcaMappedDevice_RUNPATH_FLAGS)

DIR = $(CURDIR)


all: bin/mtca4u_mex.$(MTCA4U_MATLAB_VERSION).mexa64

bin/mtca4u_mex.$(MTCA4U_MATLAB_VERSION).mexa64: src/mtca4u_mex.cpp
#use a similar naming scheme as normal .so files with version number and symlink
#FIXME What to do about the extension? This is hard coded for intel 64 bit
	echo CXXFLAGS=$(CXXFLAGS)
	mex CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' -I./include $(MTCA4U_MEX_FLAGS)\
	  -outdir bin -output mtca4u_mex.$(MTCA4U_MATLAB_VERSION) ./src/mtca4u_mex.cpp
	(cd bin; ln -sf mtca4u_mex.$(MTCA4U_MATLAB_VERSION).mexa64 mtca4u_mex.mexa64)

#debug:
#	mex -v CFLAGS='$$CFLAGS' CXXFLAGS='$$CXXFLAGS' LDFLAGS='$$LDFLAGS' $(MTCA_INCLUDE_FLAG) $(MTCA_LIB_FLAG) -lMtcaMappedDevice -outdir bin ./src/mtca4u.cpp -D__MEX_DEBUG_MODE

.PHONY: clean install install_local test 

clean:	
	rm -rf ./bin debian_from_template debian_package

#this will fail at the moment. At least $DIR is empty and setup.m is not in bin
install: all
	cd ~ && matlab -nojvm -r "cd $(DIR)/bin, run setup.m, savepath ~/pathdef.m, exit"
	@echo "Path saved in ~/pathdef.m" 

system_install: all
	test -d /local/lib || mkdir -p /local/lib
	cp -d bin/mtca4u_mex.mexa64 bin/mtca4u_mex.$(MTCA4U_MATLAB_VERSION).mexa64 matlab/mtca4u.m /local/lib

#A target which replaces the version number in the control files for the debian packaging
configure-package-files:
	test -d debian_from_template || mkdir debian_from_template
	cat debian.in/copyright.in | sed "{s/@MTCA4U_MATLAB_VERSION@/${MTCA4U_MATLAB_VERSION}/}" > debian_from_template/copyright
	cp debian.in/compat debian.in/install debian.in/rules debian_from_template/
	#this one copies the control file and sets the maintainer name
	./setMaintainerName.sh
	cat debian.in/create_mex_deps.sh.in | sed "{s/@MTCA4U_MATLAB_VERSION@/${MTCA4U_MATLAB_VERSION}/}" > debian_from_template/create_mex_deps.sh
	chmod +x debian_from_template/create_mex_deps.sh


#This target will only succeed on debian machines with the debian packaging tools installed
debian_package: configure-package-files
	./make_debian_package.sh ${MTCA4U_MATLAB_VERSION}

#test:
#	matlab -nojvm -r "try run('./unit_test/run_test.m'), catch ex, disp(ex.message); exit; end, exit"

