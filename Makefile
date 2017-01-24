CFLAGS = $$CFLAGS -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x $(DEBUG_FLAGS) 
CXXFLAGS = $$CXXFLAGS -Wall -Wextra -Wshadow -pedantic -Wuninitialized -std=c++0x $(DEBUG_FLAGS)
#LDFLAGS = $$LDFLAGS -w -std=c++0x $(DEBUG_FLAGS)

#This will set MTCA4U_MATLAB_VERSION
include mtca4u_matlab_version

#This will set MATLAB_ROOT
include matlab_root 


#Get the libraries
DeviceAccess_LIB_FLAGS=$(shell mtca4u-deviceaccess-config --ldflags)

LDFLAGS= $$LDFLAGS -w -std=c++0x -Wl,--no-as-needed $(DEBUG_FLAGS) $(DeviceAccess_LIB_FLAGS)

#Mex requires to have the library path in special manner so use mexflags commands.
DeviceAccess_MEX_FLAGS=$(shell mtca4u-deviceaccess-config --mexflags)
CXXFLAGS += $(shell mtca4u-deviceaccess-config --cppflags)

MEXEXT = $(shell $(MATLAB_ROOT)/bin/mexext)

#Setup more stuff
PWD = $(shell pwd)

### Target
all: bin/mtca4u_mex.$(MTCA4U_MATLAB_VERSION).$(MEXEXT)
	make -C test all

debug:
	DEBUG_FLAGS="-O0 --coverage" make bin/mtca4u_mex.$(MTCA4U_MATLAB_VERSION)_d.$(MEXEXT)
	make -C test debug

bin/mtca4u_mex.$(MTCA4U_MATLAB_VERSION).$(MEXEXT): src/mtca4u_mex.cpp include/version.h
#use a similar naming scheme as normal .so files with version number and symlink 
	mex CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' $(DeviceAccess_MEX_FLAGS) \
	-outdir bin -output mtca4u_mex.$(MTCA4U_MATLAB_VERSION) ./src/mtca4u_mex.cpp
#This hack is required for R2014b Matlab as mex does not create output file as expected
	(if [ ! -f bin/mtca4u_mex.$(MTCA4U_MATLAB_VERSION).$(MEXEXT) ]; then cd bin;mv mtca4u_mex.* mtca4u_mex.$(MTCA4U_MATLAB_VERSION).$(MEXEXT); fi;)

	(cd bin;ln -sf mtca4u_mex.$(MTCA4U_MATLAB_VERSION).$(MEXEXT) mtca4u_mex.$(MEXEXT))

bin/mtca4u_mex.$(MTCA4U_MATLAB_VERSION)_d.$(MEXEXT): src/mtca4u_mex.cpp include/version.h
#use a similar naming scheme as normal .so files with version number and symlink
	mex CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' $(DeviceAccess_MEX_FLAGS) \
	-g -outdir bin -output mtca4u_mex.$(MTCA4U_MATLAB_VERSION)_d ./src/mtca4u_mex.cpp -D__MEX_DEBUG_MODE

#This hack is required for R2014b Matlab as mex does not create output file as expected
	(if [ ! -f bin/mtca4u_mex.$(MTCA4U_MATLAB_VERSION)_d.$(MEXEXT) ]; then cd bin;mv mtca4u_mex.* mtca4u_mex.$(MTCA4U_MATLAB_VERSION)_d.$(MEXEXT); fi;)

	(cd bin; ln -sf mtca4u_mex.$(MTCA4U_MATLAB_VERSION)_d.$(MEXEXT) mtca4u_mex.$(MEXEXT))
	#needed to trick gcov:
	(cd bin; ln -sf mtca4u_mex.$(MEXEXT) libmtca4u_mex.so; ln -sf ../include . ; ln -sf ../src . )

include/version.h : .FORCE
	echo "const std::string gVersion(\"$(MTCA4U_MATLAB_VERSION)\");" > include/version.h

.PHONY: clean install install_local test .FORCE

.FORCE:


clean:	
	rm -rf ./bin debian_from_template debian_package include/version.h 
	rm -rf coverage_html coverage.info coverage_all.info
	make -C test clean

#this will fail at the moment. At least $DIR is empty and setup.m is not in bin
#install: all
#	cd ~ && matlab -nojvm -r "cd $(DIR)/bin, run setup.m, savepath ~/pathdef.m, exit"
#	@echo "Path saved in ~/pathdef.m" 

system_install: all
	test -d /local/lib || mkdir -p /local/lib
	cp -d bin/mtca4u_mex.mexa64 bin/mtca4u_mex.$(MTCA4U_MATLAB_VERSION).mexa64 matlab/mtca4u.m matlab/mtca4u_interface.m /local/lib

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

test:
	make -C test test

coverage:
	rm -f `find . -name "*\.gcda"`
	make test || true
	lcov --capture --directory . --output-file coverage_all.info
	#lcov capture also includes external stuff like glibc, boost etc.
	#only extract the reports for this project
	lcov --extract coverage_all.info "$(PWD)*" -o coverage.info
	genhtml coverage.info --output-directory coverage_html
