#!/bin/bash

# Add your QT path here by setting MY_QT_PATH variable
# MY_QT_PATH=/YOUR_PATH_HERE/Qt/5.X/gcc_64/bin/
MY_QT_PATH=/home/krzysiek/Qt/5.15.2/gcc_64/bin

MAKE_NUM_THREADS='-j 8'
wget="wget"
tool="gcc_64"
exe=""
APP_SUFFIX=""
QMAKE_CONFIG=""



export PATH=$MY_QT_PATH:$PATH

if [ ! -e "$MY_QT_PATH" ]; then
	echo " ---------------------------------"
	echo "      Error: Wrong Qt path."
	echo " ---------------------------------"
	echo " Qt not found at '$MY_QT_PATH'."	
	echo " Please set the MY_QT_PATH variable in the ./build.sh"
	echo ""
	exit 1
fi


cd build


${MY_QT_PATH}/qmake ../src/viewer.pro ${QMAKE_CONFIG} \
    && make clean && make $MAKE_NUM_THREADS 

cp viewer ../bin/
