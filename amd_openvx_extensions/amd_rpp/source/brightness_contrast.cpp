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
#include "internal_rpp.h"

#include "internal_publishKernels.h"

#include "/home/neel/Mythreyi/forked_MivisionX/AMD-RPP/include/rpp.h"
#include "/home/neel/Mythreyi/forked_MivisionX/AMD-RPP/include/rppdefs.h"
#include "/home/neel/Mythreyi/forked_MivisionX/AMD-RPP/include/rppi_brightness_illumination_functions.h"

// Rpp_Brighten_3C8U_pln(Rpp8u *pSrc, RppiSize size, Rpp8u *pDst, Rpp32f alpha, Rpp32f beta)

struct BrightnessLocalData {
    Rpp8u *pSrc;
    RppiSize dimensions;
    Rpp8u *pDst;
    Rpp32f alpha;
    Rpp32f beta;

#if ENABLE_OPENCL
    cl_mem input;
    cl_mem output;
#endif

#if ENABLE_HIP
    Rpp8u *pSrc;
    Rpp8u *pDst;
#endif
};

static vx_status VX_CALLBACK validateBrightnessAndContrast(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    std::cout<<"\n Inside validateBrightnessAndContrast";

 // check scalar type
    vx_enum type, out_type;

    STATUS_ERROR_CHECK(vxQueryScalar((vx_scalar)parameters[2], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_FLOAT32) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: Paramter: #2 type=%d (must be size)\n", type);
    STATUS_ERROR_CHECK(vxQueryScalar((vx_scalar)parameters[3], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_FLOAT32) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: Paramter: #3 type=%d (must be size)\n", type);

    STATUS_ERROR_CHECK(vxQueryScalar((vx_scalar)parameters[4], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_INT32) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: Paramter: #4 type=%d (must be size)\n", type);
    STATUS_ERROR_CHECK(vxQueryScalar((vx_scalar)parameters[5], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_INT32) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: Paramter: #5 type=%d (must be size)\n", type);

    STATUS_ERROR_CHECK(vxQueryArray((vx_array)parameters[0], VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type)));
    if(type != VX_TYPE_CHAR) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: Paramter: #0 type=%d (must be size)\n", type);
    STATUS_ERROR_CHECK(vxQueryArray((vx_array)parameters[1], VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type)));
    if(type != VX_TYPE_CHAR) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: Paramter: #1 type=%d (must be size)\n", type);

    out_type = type;
    STATUS_ERROR_CHECK(vxSetMetaFormatAttribute(metas[1], VX_ARRAY_ITEMTYPE, &out_type, sizeof(out_type)));

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK processBrightnessAndContrast(vx_node node, const vx_reference * parameters, vx_uint32 num)
{

    std::cout<<"\n Inside processBrightnessAndContrast \n";
    BrightnessLocalData * data = NULL;
    STATUS_ERROR_CHECK(vxQueryNode(node, VX_NODE_LOCAL_DATA_PTR, &data, sizeof(data)));
/*
#ifdef ENABLE_OPENCL
    STATUS_ERROR_CHECK(vxQueryArray((vx_array)parameters[0], VX_ARRAY_BUFFER_OPENCL, &data->input, sizeof(data->input)));
    STATUS_ERROR_CHECK(vxQueryArray((vx_array)parameters[1], VX_ARRAY_BUFFER_OPENCL, &data->output, sizeof(data->output)));
#endif
*/
    STATUS_ERROR_CHECK(vxQueryArray((vx_array)parameters[0], VX_ARRAY_BUFFER, data->pSrc, sizeof(data->pSrc)));
    STATUS_ERROR_CHECK(vxQueryArray((vx_array)parameters[1], VX_ARRAY_BUFFER, data->pDst, sizeof(data->pDst)));

    //Rpp_Brighten_3C8U_pln((Rpp8u*)data->pSrc, data->dimensions, (Rpp8u*)data->pDst,  data->alpha, data->beta);
    rppi_brighten_1C8U_pln((Rpp8u*)data->pSrc, data->dimensions, (Rpp8u*)data->pDst,  data->alpha, data->beta);
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK initializeBrightnessAndContrast(vx_node node, const vx_reference *parameters, vx_uint32 num)
{

    BrightnessLocalData * data = new BrightnessLocalData;
    memset(data, 0, sizeof(*data));

    //Set Descriptors.
    vx_int32  height;
    vx_int32 width;
    vx_float32 alpha;
    vx_float32 beta;

    STATUS_ERROR_CHECK(vxCopyScalar((vx_scalar)parameters[2], &data->dimensions.height, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    STATUS_ERROR_CHECK(vxCopyScalar((vx_scalar)parameters[3], &data->dimensions.width, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    STATUS_ERROR_CHECK(vxCopyScalar((vx_scalar)parameters[4], &data->alpha, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    STATUS_ERROR_CHECK(vxCopyScalar((vx_scalar)parameters[5], &data->beta, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

/*
#ifdef ENABLE_OPENCL
    //Declare Memory.
    STATUS_ERROR_CHECK(vxQueryArray((vx_array)parameters[0], VX_ARRAY_BUFFER_OPENCL, &data->input, sizeof(data->input)));
    STATUS_ERROR_CHECK(vxQueryArray((vx_array)parameters[1], VX_ARRAY_BUFFER_OPENCL, &data->output, sizeof(data->output)))
#endif
*/

    STATUS_ERROR_CHECK(vxQueryArray((vx_array)parameters[0], VX_ARRAY_BUFFER, data->pSrc, sizeof(data->pSrc)));
    STATUS_ERROR_CHECK(vxQueryArray((vx_array)parameters[1], VX_ARRAY_BUFFER, data->pDst, sizeof(data->pDst)));

    STATUS_ERROR_CHECK(vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &data, sizeof(data)));
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK uninitializeBrightnessAndContrast(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    std::cout<<"\n Inside uninitializeBrightnessAndContrast \n";
    return VX_SUCCESS;
}

vx_status BrightnessAndContrast_Register(vx_context context)
{
    std::cout<<"\n Inside BrightnessAndContrast_Register \n";
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
    vx_bool enableBufferAccess = vx_false_e;
    STATUS_ERROR_CHECK(vxSetKernelAttribute(kernel, VX_KERNEL_ATTRIBUTE_AMD_OPENCL_BUFFER_ACCESS_ENABLE, &enableBufferAccess, sizeof(enableBufferAccess)));
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
