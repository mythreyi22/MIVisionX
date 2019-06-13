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
#include <iostream>
#include "internal_rpp.h"

#include "internal_publishKernels.h"

#include </opt/rocm/rpp/include/rpp.h>
#include </opt/rocm/rpp/include/rppdefs.h>
#include </opt/rocm/rpp/include/rppi_arithmetic_and_logical_functions.h>

struct Accumulate_weightedLocalData {

#if ENABLE_OPENCL
    RPPCommonHandle handle;
#endif
    RppiSize dimensions;
    RppPtr_t pSrc1;
    RppPtr_t pSrc2;
    double alpha;

#if ENABLE_OPENCL
    cl_mem cl_pSrc1;
    cl_mem cl_pSrc2;
#endif

#if ENABLE_HIP
    void *hip_pSrc2;
    void *hip_pSrc1;
    double alpha;
#endif

};

static vx_status VX_CALLBACK validateAccumulate_weighted(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
 // check scalar alpha and beta type
    vx_status status = VX_SUCCESS;
    vx_enum type;
    vx_parameter param = vxGetParameterByIndex(node, 0);
    STATUS_ERROR_CHECK(vxQueryScalar((vx_scalar)parameters[2], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_FLOAT64) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: Paramter: #2 type=%d (must be size)\n", type);
    // Assuming both images will be of same type.
    vx_image image;
    vx_df_image df_image = VX_DF_IMAGE_VIRT;
    STATUS_ERROR_CHECK(vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &image, sizeof(vx_image)));
    STATUS_ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &df_image, sizeof(df_image)));
    if (df_image != VX_DF_IMAGE_U8 || df_image != VX_DF_IMAGE_RGB)
            status = VX_ERROR_INVALID_VALUE;
    STATUS_ERROR_CHECK(vxSetMetaFormatAttribute(metas[0], VX_IMAGE_FORMAT, &df_image, sizeof(df_image)));
     vx_uint32  height, width;
    STATUS_ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height)));
    STATUS_ERROR_CHECK(vxSetMetaFormatAttribute(metas[0], VX_IMAGE_HEIGHT, &height, sizeof(height)));
    STATUS_ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width)));
    STATUS_ERROR_CHECK(vxSetMetaFormatAttribute(metas[0], VX_IMAGE_WIDTH, &width, sizeof(width)));
    vxReleaseImage(&image);
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK processAccumulate_weighted(vx_node node, const vx_reference * parameters, vx_uint32 num)
{
    Accumulate_weightedLocalData * data = NULL;
    STATUS_ERROR_CHECK(vxQueryNode(node, VX_NODE_LOCAL_DATA_PTR, &data, sizeof(data)));
    vx_df_image df_image = VX_DF_IMAGE_VIRT;
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_ATTRIBUTE_FORMAT, &df_image, sizeof(df_image)));
    vx_df_image df_image2 = VX_DF_IMAGE_VIRT;
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[1], VX_IMAGE_ATTRIBUTE_FORMAT, &df_image2, sizeof(df_image2)));

#if ENABLE_OPENCL

    cl_command_queue handle = data->handle.cmdq;
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_ATTRIBUTE_AMD_OPENCL_BUFFER, &data->cl_pSrc1, sizeof(data->cl_pSrc1)));
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[1], VX_IMAGE_ATTRIBUTE_AMD_OPENCL_BUFFER, &data->cl_pSrc2, sizeof(data->cl_pSrc2)));
    if (df_image == VX_DF_IMAGE_U8 ){
        rppi_accumulate_weighted_u8_pln1_gpu((void *)data->cl_pSrc1, (void *)data->cl_pSrc2, data->dimensions, data->alpha, (void *)handle);
    }
    else if(df_image == VX_DF_IMAGE_RGB) {
        rppi_accumulate_weighted_u8_pkd3_gpu((void *)data->cl_pSrc1, (void *)data->cl_pSrc2, data->dimensions, data->alpha, (void *)handle);
    }
    return VX_SUCCESS;

#else

    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_ATTRIBUTE_BUFFER, &data->pSrc1, sizeof(vx_uint8)));
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[1], VX_IMAGE_ATTRIBUTE_BUFFER, &data->pSrc2, sizeof(vx_uint8)));
    if (df_image == VX_DF_IMAGE_U8 ){
        rppi_accumulate_weighted_u8_pln1_host((void *)data->pSrc1, (void *)data->pSrc2,
                                                data->dimensions, data->alpha);
    }
    else if(df_image == VX_DF_IMAGE_RGB) {
        rppi_accumulate_weighted_u8_pkd3_host((void *)data->pSrc1, (void *)data->pSrc2,
                                                 data->dimensions, data->alpha);
    }
    return VX_SUCCESS;

#endif // ENABLE_OPENCL

  return VX_SUCCESS;
}

static vx_status VX_CALLBACK initializeAccumulate_weighted(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    Accumulate_weightedLocalData * data = new Accumulate_weightedLocalData;
    memset(data, 0, sizeof(*data));

#if ENABLE_OPENCL
    STATUS_ERROR_CHECK(vxQueryNode(node, VX_NODE_ATTRIBUTE_AMD_OPENCL_COMMAND_QUEUE, &data->handle.cmdq, sizeof(data->handle.cmdq)));
#endif
    // Assuming both images are of same height and width
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_HEIGHT, &data->dimensions.height, sizeof(data->dimensions.height)));
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_WIDTH, &data->dimensions.width, sizeof(data->dimensions.width)));
    STATUS_ERROR_CHECK(vxCopyScalar((vx_scalar)parameters[2], &data->alpha, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

#if ENABLE_OPENCL
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_ATTRIBUTE_AMD_OPENCL_BUFFER, &data->cl_pSrc1, sizeof(data->cl_pSrc1)));
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[1], VX_IMAGE_ATTRIBUTE_AMD_OPENCL_BUFFER, &data->cl_pSrc2, sizeof(data->cl_pSrc2)));
#else
    //STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_ATTRIBUTE_BUFFER, data->pSrc, sizeof(data->pSrc)));
#endif

    STATUS_ERROR_CHECK(vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &data, sizeof(data)));

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK uninitializeAccumulate_weighted(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

vx_status AccumulateWeighted_Register(vx_context context)
{
    vx_status status = VX_SUCCESS;
// Add kernel to the context with callbacks
    vx_kernel kernel = vxAddUserKernel(context, "org.rpp.AccumulateWeighted",
            VX_KERNEL_RPP_ACCUMULATEWEIGHTED,
            processAccumulate_weighted,
            3,
            validateAccumulate_weighted,
            initializeAccumulate_weighted,
            uninitializeAccumulate_weighted);

    ERROR_CHECK_OBJECT(kernel);
    #if ENABLE_OPENCL
    // enable OpenCL buffer access since the kernel_f callback uses OpenCL buffers instead of host accessible buffers
    vx_bool enableBufferAccess = vx_true_e;
    STATUS_ERROR_CHECK(vxSetKernelAttribute(kernel, VX_KERNEL_ATTRIBUTE_AMD_OPENCL_BUFFER_ACCESS_ENABLE, &enableBufferAccess, sizeof(enableBufferAccess)));
    #else
    vx_bool enableBufferAccess = vx_false_e;
    #endif
    if (kernel)
    {
        PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 0, VX_BIDIRECTIONAL, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED));
        PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 1, VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED));
        PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 2, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED));
        PARAM_ERROR_CHECK(vxFinalizeKernel(kernel));
    }
    if (status != VX_SUCCESS)
    {
    exit:   vxRemoveKernel(kernel); return VX_FAILURE;
    }

    return status;
}