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

#include "kernels.h"
#include "vx_ext_rpp.h"

// Rpp_Brighten_3C8U_pln(Rpp8u *pSrc, RppiSize size, Rpp8u *pDst, Rpp32f alpha, Rpp32f beta) 
struct Brightness {
    Rpp8u *pSrc, 
    RppiSize size,
    Rpp8u *pDst,
    Rpp32f alpha, 
    Rpp32f beta,
#if ENABLE_OPENCL
    cl_mem input_mem;
    cl_mem output_mem;
#endif

#if ENABLE_HIP
    Rpp8u *pSrc,
    Rpp8u *pDst
#endif
};

static vx_status VX_CALLBACK validate(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
    // check scalar type
    vx_enum type, out_type;
    ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)parameters[1], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_ENUM) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #1 type=%d (must be enum)\n", type);
    ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)parameters[2], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_SIZE) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #2 type=%d (must be size)\n", type);
    ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)parameters[3], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_SIZE) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #3 type=%d (must be size)\n", type);
    ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)parameters[4], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_SIZE) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #4 type=%d (must be size)\n", type);
    ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)parameters[5], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_SIZE) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #5 type=%d (must be size)\n", type);
    ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)parameters[6], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_ENUM) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #6 type=%d (must be enum)\n", type);

    // check tensor dimensions
    vx_size num_dims;
    vx_size input_dims[4], output_dims[4];
    ERROR_CHECK_STATUS(vxQueryTensor((vx_tensor)parameters[0], VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(num_dims)));
    ERROR_CHECK_STATUS(vxQueryTensor((vx_tensor)parameters[0], VX_TENSOR_DATA_TYPE, &type, sizeof(type)));
    if (num_dims != 4) return ERRMSG(VX_ERROR_INVALID_DIMENSION, "validate: POOL: #0 num_dims=%ld (must be 4)\n", num_dims);
    if ((type != VX_TYPE_FLOAT32) && (type != VX_TYPE_FLOAT16)) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #0 type=%d (must be float)\n", type);
    ERROR_CHECK_STATUS(vxQueryTensor((vx_tensor)parameters[0], VX_TENSOR_DIMS, input_dims, sizeof(input_dims)));
    ERROR_CHECK_STATUS(vxQueryTensor((vx_tensor)parameters[7], VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(num_dims)));
    ERROR_CHECK_STATUS(vxQueryTensor((vx_tensor)parameters[7], VX_TENSOR_DATA_TYPE, &out_type, sizeof(out_type)));
    if (num_dims != 4) return ERRMSG(VX_ERROR_INVALID_DIMENSION, "validate: POOL: #7 num_dims=%ld (must be 4)\n", num_dims);
    if ((out_type != VX_TYPE_FLOAT32) && (out_type != VX_TYPE_FLOAT16)) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #7 type=%d (must be float)\n", type);
    ERROR_CHECK_STATUS(vxQueryTensor((vx_tensor)parameters[7], VX_TENSOR_DIMS, output_dims, sizeof(output_dims)));
    if (output_dims[3] != input_dims[3] || output_dims[2] != input_dims[2])
        return ERRMSG(VX_ERROR_INVALID_DIMENSION, "validate: POOL: dims input[%ld,%ld,%ld,%ld] output[%ld,%ld,%ld,%ld]\n",
                    input_dims[0], input_dims[1], input_dims[2], input_dims[3],
                    output_dims[0], output_dims[1], output_dims[2], output_dims[3]);
    out_type = type;        // has to be same as input

    vx_enum pad_border_mode = 0;
    if(parameters[8]) {
        ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)parameters[8], VX_SCALAR_TYPE, &type, sizeof(type)));
        if(type != VX_TYPE_ENUM) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #8 type=%d (must be enum)\n", type);
        ERROR_CHECK_STATUS(vxCopyScalar((vx_scalar)parameters[8], &pad_border_mode, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        if(pad_border_mode != 0 && pad_border_mode != 1) {
            return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #8 pad_border_mode=%d (must be 0 or 1)\n", pad_border_mode);
        }
    }
    if(parameters[9]) {
        ERROR_CHECK_STATUS(vxQueryScalar((vx_scalar)parameters[9], VX_SCALAR_TYPE, &type, sizeof(type)));
        if(type != VX_TYPE_INT32) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #8 type=%d (must be VX_TYPE_INT32)\n", type);
        vx_int32 activation_mode = 0;
        ERROR_CHECK_STATUS(vxCopyScalar((vx_scalar)parameters[9], &activation_mode, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        if(activation_mode != 0 && activation_mode != 1) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: POOL: #5 activation_mode=%d (must be 0 or 1)\n", activation_mode);
    }

    // output tensor configuration
    num_dims = 4;
    ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(metas[7], VX_TENSOR_DATA_TYPE, &out_type, sizeof(out_type)));
    ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(metas[7], VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(num_dims)));
    ERROR_CHECK_STATUS(vxSetMetaFormatAttribute(metas[7], VX_TENSOR_DIMS, output_dims, sizeof(output_dims)));
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK processPoolingLayer(vx_node node, const vx_reference * parameters, vx_uint32 num)
{
    PoolingLayerLocalData * data = NULL;
    ERROR_CHECK_STATUS(vxQueryNode(node, VX_NODE_LOCAL_DATA_PTR, &data, sizeof(data)));
    miopenHandle_t miopenHandle = data->handle->miopen_handle;

    ERROR_CHECK_STATUS(vxQueryTensor((vx_tensor)parameters[0], VX_TENSOR_BUFFER_OPENCL, &data->input_mem, sizeof(data->input_mem)));
    ERROR_CHECK_STATUS(vxQueryTensor((vx_tensor)parameters[7], VX_TENSOR_BUFFER_OPENCL, &data->output_mem, sizeof(data->output_mem)));

    ERROR_CHECK_MIOPEN_STATUS(miopenPoolingForward(miopenHandle, data->pool_desc, &data->alpha, data->input_desc, data->input_mem, &data->beta, data->output_desc, data->output_mem, false, nullptr, 0));

    // activation (in-place in output_mem)
    if(parameters[9]) {
        float alpha = 1.0f, beta = 0.0f;
        ERROR_CHECK_MIOPEN_STATUS(miopenActivationForward(data->handle->miopen_handle, data->activation_desc, &alpha, data->output_desc, data->output_mem, &beta, data->output_desc, data->output_mem));
    }

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK initializeBrightnessAndContrast(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    Brightness * data = new Brightness;
    memset(data, 0, sizeof(*data));

    //Set Descriptors.
    vx_int32  height;
    vx_int32 width;
    vx_float32 alpha; 
    vx_float32 beta;

    ERROR_CHECK_STATUS(vxCopyScalar((vx_scalar)parameters[2], &height, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    ERROR_CHECK_STATUS(vxCopyScalar((vx_scalar)parameters[3], &width, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    ERROR_CHECK_STATUS(vxCopyScalar((vx_scalar)parameters[4], &alpha, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    ERROR_CHECK_STATUS(vxCopyScalar((vx_scalar)parameters[5], &beta, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

#if ENABLE_OPENCL
    //Declare Memory.
    ERROR_CHECK_STATUS(vxQueryTensor((vx_tensor)parameters[0], VX_TENSOR_BUFFER_OPENCL, &data->pSrc, sizeof(data->pSrc)));
    ERROR_CHECK_STATUS(vxQueryTensor((vx_tensor)parameters[1], VX_TENSOR_BUFFER_OPENCL, &data->pDst, sizeof(data->pDst)));
#endif

    ERROR_CHECK_STATUS(vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &data, sizeof(data)));
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


VX_API_ENTRY vx_node VX_API_CALL vxExtrppNode_brightness(vx_graph graph, vx_array pSrc, vx_array pDst, vx_int32  height, vx_int32 width, vx_float32 alpha, vx_float32 beta){

    vx_node node = NULL;
    vx_context context = vxGetContext((vx_reference)graph);
    if(vxGetStatus((vx_reference)context) == VX_SUCCESS) {
        vx_scalar alpha = vxCreateScalarWithSize(context, VX_TYPE_FLOAT32, &alpha, sizeof(alpha));
        vx_scalar beta = vxCreateScalarWithSize(context, VX_TYPE_FLOAT32, &beta, sizeof(beta));
        vx_scalar height = vxCreateScalarWithSize(context, VX_TYPE_INT32, &height, sizeof(height));
        vx_scalar width = vxCreateScalarWithSize(context, VX_TYPE_INT32, &width, sizeof(width));

        if( vxGetStatus((vx_reference)alpha) == VX_SUCCESS &&
                vxGetStatus((vx_reference)beta) == VX_SUCCESS)
                vxGetStatus((vx_reference)height) == VX_SUCCESS)
                vxGetStatus((vx_reference)width) == VX_SUCCESS)
        {
            vx_reference params[] = {
                (vx_reference) psrc,
                (vx_reference) pDst,
                (vx_reference) height,
                (vx_reference) width,
                (vx_reference) alpha,
                (vx_reference) beta
        };
            node = createNode(graph, VX_KERNEL_BRIGHTNESS_AND_CONSTAST, params, sizeof(params)/sizeof(params[0]));
            vxReleaseScalar(&alpha);
            vxReleaseScalar(&beta);
            vxReleaseScalar(&height);
            vxReleaseScalar(&width);
        }
    }
    return node;
}