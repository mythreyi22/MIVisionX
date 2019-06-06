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


#ifndef _PUBLISH_KERNELS_H_
#define _PUBLISH_KERNELS_H_

#include "internal_rpp.h"

extern "C" SHARED_PUBLIC vx_status VX_API_CALL vxPublishKernels(vx_context context);
vx_status ADD_KERENEL(std::function<vx_status(vx_context)>);
vx_status get_kernels_to_publish();

vx_status Brightness_Register(vx_context);
vx_status Contrast_Register(vx_context);
vx_status Blur_Register(vx_context);
vx_status Flip_Register(vx_context);
vx_status Hue_Register(vx_context);
vx_status Saturation_Register(vx_context);
vx_status BitwiseAND_Register(vx_context);
vx_status BitwiseNOT_Register(vx_context);
vx_status ExclusiveOR_Register(vx_context);
vx_status InclusiveOR_Register(vx_context);
vx_status Add_Register(vx_context);
vx_status Subtract_Register(vx_context);
vx_status AbsoluteDifference_Register(vx_context);

#define VX_KERNEL_RPP_BRIGHTNESS_NAME           "org.rpp.Brightness"
#define VX_KERNEL_RPP_CONTRAST_NAME             "org.rpp.Contrast"
#define VX_KERNEL_RPP_BLUR_NAME                 "org.rpp.Blur"
#define VX_KERNEL_RPP_FLIP_NAME                 "org.rpp.Flip"
#define VX_KERNEL_RPP_HUE_NAME                  "org.rpp.Hue"
#define VX_KERNEL_RPP_SATURATION_NAME           "org.rpp.Saturation"
#define VX_KERNEL_RPP_BITWISEAND_NAME           "org.rpp.BitwiseAND"
#define VX_KERNEL_RPP_BITWISENOT_NAME           "org.rpp.BitwiseNOT"
#define VX_KERNEL_RPP_EXCLUSIVEOR_NAME          "org.rpp.ExclusiveOR"
#define VX_KERNEL_RPP_ADD_NAME                  "org.rpp.Add"
#define VX_KERNEL_RPP_SUBTRACT_NAME             "org.rpp.Subtract"
#define VX_KERNEL_ABSOLUTEDIFFERENCE_NAME       "org.rpp.AbsoluteDifference"

#endif //_AMDVX_EXT__PUBLISH_KERNELS_H_
