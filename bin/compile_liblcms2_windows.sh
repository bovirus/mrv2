#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script compiles a LCMS2 as a shared library (DLL).
#
if [[ ! -e etc/build_dir.sh ]]; then
    echo "You must run this script from the root of mrv2 directory like:"
    echo
    script=`basename $0`
    echo "> bin/$script"
    exit 1
fi

if [[ ! $RUNME ]]; then
    . etc/build_dir.sh
else
    . etc/functions.sh
fi

#
# Set the main tag to compile
#
LCMS_BRANCH=lcms2.16

if [[ $KERNEL != *Msys* ]]; then
    echo
    echo "This script is for Windows MSys2-64 only."
    echo
    exit 1
fi

if [[ -z $TLRENDER_RAW ]]; then
    export TLRENDER_RAW=ON
fi


MRV2_ROOT=$PWD
SUPERBUILD=$PWD/$BUILD_DIR/tlRender/etc/SuperBuild
INSTALLDIR=$PWD/$BUILD_DIR/install

if [[ $TLRENDER_RAW == ON || $TLRENDER_RAW == 1 ]]; then
    if [[ ! -e $INSTALLDIR/lib/liblcms2.lib ]]; then

	#
	# Install development tools
	#
	pacman -Sy --noconfirm

	pacman -Sy binutils --noconfirm
    
	#
	# Clone the repository
	#
	mkdir -p $SUPERBUILD
	cd $SUPERBUILD
	if [[ ! -d LCMS2 ]]; then
	    run_cmd git clone --depth 1 --branch $LCMS_BRANCH https://github.com/mm2/Little-CMS.git LCMS2 2> /dev/null
	fi
    
	#
	# Run configure
	#
	cd LCMS2
	
	export CC=cl
	export CXX=cl
	export LD=link

	#
	# Remove some cl.exe compile warnings
	#
	sed -i -e 's#-fvisibility=hidden##' ./configure
	sed -i -e 's#CFLAGS="-g"#CFLAGS=""#' ./configure
	sed -i -e 's#CXXFLAGS="-g"#CXXFLAGS=""#' ./configure
	
	run_cmd ./configure --build=mingw64 --enable-shared --disable-static --prefix=$INSTALLDIR
    
	#
	# Compile and install the library
	#
	make -j ${CPU_CORES} install

	echo "Compiled result:"
	echo
	ls $INSTALLDIR/lib/*lcms2*
	echo
    
	run_cmd mv $INSTALLDIR/lib/lcms2.dll.lib $INSTALLDIR/lib/liblcms2.lib
    
	cd $MRV2_ROOT
    else
	echo "liblcms2 already installed."
    fi
fi
