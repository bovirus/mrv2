# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

include( ExternalProject )

set( PYTHON_VERSION 3.10 )
set( PYTHON_PATCH   9 )
set( PYTHON_URL https://www.python.org/ftp/python/${PYTHON_VERSION}.${PYTHON_PATCH}/Python-${PYTHON_VERSION}.${PYTHON_PATCH}.tar.xz )

if(UNIX)
    set( PYTHON_CONFIGURE ./configure --enable-optimizations
        --prefix=${CMAKE_INSTALL_PREFIX}
        "CFLAGS=${CMAKE_C_FLAGS}"
        "CPPFLAGS=${CMAKE_C_FLAGS}"
        "CXXFLAGS=${CMAKE_CXX_FLAGS}"
        "MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}"
	"LDFLAGS=-Wl,-rpath=/${CMAKE_INSTALL_PREFIX}/lib,--disable-new-dtags"
    )
    set( PYTHON_BUILD    make -j 4)
    set( PYTHON_INSTALL  make altinstall )
else()
    set( platform x64 )
    if( "$ENV{ARCH}" STREQUAL "i386" )
    	set( platform Win32 )
    endif()
    set( PYTHON_CONFIGURE PCbuild/build.bat -e -q -p ${platform} )
    set( PYTHON_BUILD  PCbuild/build.bat -e -q -p ${platform} )
    set( PYTHON_INSTALL python.bat PC/layout --precompile --preset-default  --copy "${CMAKE_INSTALL_PREFIX}/bin/" )
endif()

#
# Currently, this is broken on Windows
#
ExternalProject_Add(
    Python
    URL ${PYTHON_URL}
    CONFIGURE_COMMAND ${PYTHON_CONFIGURE}
    BUILD_COMMAND     ${PYTHON_BUILD}
    INSTALL_COMMAND   ${PYTHON_INSTALL}
    BUILD_IN_SOURCE 1
)

set( PYTHON Python )

