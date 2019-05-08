/*
Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#ifndef _VX_KERNELS_RPP_H_
#define _VX_KERNELS_RPP_H_

//////////////////////////////////////////////////////////////////////
// SHARED_PUBLIC - shared sybols for export
// STITCH_API_ENTRY - export API symbols
#if _WIN32
#define SHARED_PUBLIC extern "C" __declspec(dllexport)
#else
#define SHARED_PUBLIC extern "C" __attribute__ ((visibility ("default")))
#endif

//////////////////////////////////////////////////////////////////////
// common header files
#include <VX/vx.h>
#include <vx_ext_rpp.h>
#include <vx_ext_amd.h>
#include <iostream>
#include <string.h>
#define ERRMSG(status, format, ...) printf("ERROR: " format, __VA_ARGS__), status

#define VX_LIBRARY_RPP         1

enum vx_kernel_ext_amd_rpp_e

{
    VX_KERNEL_BRIGHTNESS  = VX_KERNEL_BASE(VX_ID_MCW, VX_LIBRARY_RPP) + 0x001,
};

//////////////////////////////////////////////////////////////////////
//! \brief Common data shared across all nodes in a graph
struct RPPCommonHandle {
    cl_command_queue cmdq;
};
//////////////////////////////////////////////////////////////////////
//! \brief The utility functions
vx_node createNode(vx_graph graph, vx_enum kernelEnum, vx_reference params[], vx_uint32 num);

#endif //__KERNELS_H__
