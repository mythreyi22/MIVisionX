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


#ifndef _VX_EXT_RPP_H_
#define _VX_EXT_RPP_H_

#include <VX/vx.h>
#include "kernels_rpp.h"

//#ifndef ENABLE_OPENCL
//#define ENABLE_OPENCL 1
//#endif

#if ENABLE_OPENCL
#include <CL/cl.h>
#endif


	/*!***********************************************************************************************************
						RPP VX_API_ENTRY C Function NODE
	*************************************************************************************************************/

extern "C" SHARED_PUBLIC vx_node VX_API_CALL vxExtrppNode_brightness(vx_graph graph, vx_image pSrc, vx_image pDst, vx_float32 alpha, vx_int32 beta);
extern "C" SHARED_PUBLIC vx_node VX_API_CALL vxExtrppNode_contrast(vx_graph graph, vx_image pSrc, vx_image pDst, vx_uint32 max, vx_uint32 min);
extern "C" SHARED_PUBLIC vx_node VX_API_CALL vxExtrppNode_blur(vx_graph graph, vx_image pSrc, vx_image pDst);

#endif
