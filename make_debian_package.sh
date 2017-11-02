#!/bin/bash

#This script is designed to be called from the Makefile.
#It requires a configured 'debian_from_template' directory
#Simply run 'make debian_package' instead of executing it directly

#drop out if an error occurs
set -e

#check that the input parameter is set
#arg1 = TAG_VERSION
if [ $# -ne 1 ] ; then
    echo Wrong number of parameters. Run \'make debian_package\' instead of using this scipt directly.
    exit -1
fi

TAG_VERSION=$1

#Check that the debian files are configured
test -d debian_from_template || echo "Debian files not configured. Run \'make debian_package\' instead of using this scipt directly."
test -d debian_from_template || exit -2


#We only allow to make debian packages from tagged versions.
#For this we check out the tag from svn to make sure it's there, and build
#the package from this checkout to make sure the code has not been modified.
CHECKOUT_DIRECTORY=${PWD}/debian_package/mtca4u-matlab-tools-${TAG_VERSION}
ORIGINAL_BUILD_DIR=${PWD}

rm -rf debian_package
mkdir debian_package

# clone from the checkout copy because doocsdev16 does not have internet connection
git clone . -b ${TAG_VERSION} ${CHECKOUT_DIRECTORY}

cp -r ${ORIGINAL_BUILD_DIR}/debian_from_template ${CHECKOUT_DIRECTORY}/debian

cd ${CHECKOUT_DIRECTORY}

#The package versions for Debian / Ubuntu contain the codename of the distribution. Get it from the system.
CODENAME=`lsb_release -c | sed "{s/Codename:\s*//}"`

#To create the changelog we use debchange because it does the right format and the current date, user name and email automatically for us.
#Use the NAME and EMAIL environment variables to get correct values if needed (usually the email is
# user@host instead of first.last@institute, for instance killenb@mskpcx18571.desy.de instead of martin.killenberg@desy.de).
debchange --create --package mtca4u-matlab-tools -v ${TAG_VERSION}-${CODENAME}1 --distribution ${CODENAME} Debian package for the MTCA4U Matlab tools version ${TAG_VERSION}.

#Now everything is prepared and we can actually build the package.
#If you have a gpg signature you can remove the -us and -uc flags and sign the package.
dpkg-buildpackage -rfakeroot -us -uc
