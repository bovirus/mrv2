# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

include(ExternalProject)

set(pyFLTK_SVN_REPOSITORY "https://svn.code.sf.net/p/pyfltk/code/branches/fltk1.4")
set(pyFLTK_SVN_REVISION )

if(NOT PYTHON_EXECUTABLE)
    if(UNIX)
	set(PYTHON_EXECUTABLE python3)
    else()
	set(PYTHON_EXECUTABLE python)
    endif()
endif()

message(STATUS "pyFLTK got PYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}" )

set(pyFLTK_PATCH ${CMAKE_COMMAND} copy_if_different "${PROJECT_SOURCE_DIR}/cmake/patches/pyFLTK-patch/swig/x.i" "${CMAKE_BINARY_DIR}/pyFLTK-prefix/src/pyFLTK/src/swig/")
set(pyFLTK_CONFIGURE ${PYTHON_EXECUTABLE} setup.py swig)
set(pyFLTK_BUILD     ${PYTHON_EXECUTABLE} setup.py build)
set(pyFLTK_INSTALL   ${PYTHON_EXECUTABLE} setup.py install)

ExternalProject_Add(
    pyFLTK
    SVN_REPOSITORY ${pyFLTK_SVN_REPOSITORY}
    SVN_REVISION ${pyFLTK_SVN_REVISION}
    DEPENDS ${PYTHON_DEP} ${FLTK_DEP}
    PATCH_COMMAND     ${pyFLTK_PATCH}
    CONFIGURE_COMMAND ${pyFLTK_CONFIGURE}
    BUILD_COMMAND     ${pyFLTK_BUILD}
    INSTALL_COMMAND   ${pyFLTK_INSTALL}
    BUILD_IN_SOURCE 1
)


set( pyFLTK_DEP pyFLTK )
