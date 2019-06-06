# Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

cmake_minimum_required (VERSION 2.8)
project (rpp)

set (CMAKE_CXX_STANDARD 11)
cmake_policy(SET CMP0054 NEW)

find_package(OpenCL     QUIET)

include_directories (include)
include_directories (../../amd_openvx/openvx/include/)
include_directories (/opt/rocm/rpp/include/)

list (APPEND SOURCES
	source/brightness.cpp
	source/contrast.cpp
	source/blur.cpp
	source/flip.cpp
	source/hue.cpp
	source/saturation.cpp
	source/kernel_rpp.cpp
	source/internal_publishKernels.cpp
	source/bitwiseAND.cpp
	source/bitwiseNOT.cpp
	source/bitwise_exclusive_OR.cpp
	source/bitwise_inclusive_OR.cpp
	source/add.cpp
	source/subtract.cpp
	source/absolute_difference.cpp
	)

link_directories(/opt/rocm/rpp/lib/)

if (OpenCL_FOUND)
	message( "BUILDING WITH OPENCL BACKEND")
	set(ENABLE_OPENCL 1)
	add_definitions( -DENABLE_OPENCL=${ENABLE_OPENCL} )
	include_directories(${OpenCL_INCLUDE_DIRS} ${OpenCL_INCLUDE_DIRS}/Headers)
	add_library(vx_rpp SHARED ${SOURCES})
	target_link_libraries(vx_rpp amd_rpp ${OpenCL_LIBRARIES})
else(OpenCL_FOUND)
	message( "OPENCL NOT FOUND")
	set(ENABLE_OPENCL 0)
	add_definitions( -DENABLE_OPENCL=${ENABLE_OPENCL} )
	add_library(vx_rpp SHARED ${SOURCES})
	target_link_libraries(vx_rpp amd_rpp)
endif(OpenCL_FOUND)

install (TARGETS vx_rpp DESTINATION lib)
install (FILES include/vx_ext_rpp.h include/kernels_rpp.h DESTINATION include)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	target_link_libraries( OpenVX)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else()
	target_link_libraries( openvx)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections")
endif()
