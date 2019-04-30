/*
Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.

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

#include <kernels_rpp.h>
#include <vx_ext_rpp.h>
#include <stdio.h>

#include"internal_publishKernels.h"

#include "/home/neel/Mythreyi/forked_RPP/AMD-RPP/include/rpp.h"
#include "/home/neel/Mythreyi/forked_RPP/AMD-RPP/include/rppdefs.h"
#include "/home/neel/Mythreyi/forked_RPP/AMD-RPP/include/rppi_brightness_illumination_functions.h"

#define ENABLE_OPENCL 1

static vx_status VX_CALLBACK validateBrightnessAndContrast(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK processBrightnessAndContrast(vx_node node, const vx_reference * parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK initializeBrightnessAndContrast(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK uninitializeBrightnessAndContrast(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

vx_status publishBrightnessAndContrast(vx_context context)
{
    // add kernel to the context with callbacks
    return VX_SUCCESS;
}

vx_status BrightnessAndContrast_Register(vx_context context)
{
	vx_status status = VX_SUCCESS;

// add kernel to the context with callbacks
    vx_kernel kernel = vxAddUserKernel(context, "org.rpp.BrightnessAndContrast",
            VX_KERNEL_BRIGHTNESS_AND_CONTRAST,
            processBrightnessAndContrast,
            6,
            validateBrightnessAndContrast,
            initializeBrightnessAndContrast,
            uninitializeBrightnessAndContrast);

    ERROR_CHECK_OBJECT(kernel);
    #ifdef ENABLE_OPENCL
    // enable OpenCL buffer access since the kernel_f callback uses OpenCL buffers instead of host accessible buffers
    vx_bool enableBufferAccess = vx_true_e;
    ERROR_CHECK_STATUS(vxSetKernelAttribute(kernel, VX_KERNEL_ATTRIBUTE_AMD_OPENCL_BUFFER_ACCESS_ENABLE, &enableBufferAccess, sizeof(enableBufferAccess)));
    #endif

	if (kernel)
	{
		PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 0, VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED));
		PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 1, VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED));
		PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 2, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED));
		PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 3, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED));
        PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 4, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED));
        PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 5, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED));
		PARAM_ERROR_CHECK(vxFinalizeKernel(kernel));
	}

	if (status != VX_SUCCESS)
	{
	exit:	vxRemoveKernel(kernel); return VX_FAILURE;
	}

	return status;
}
