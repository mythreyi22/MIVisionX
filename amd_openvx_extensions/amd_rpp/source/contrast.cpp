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

#include </opt/rocm/rpp/include/rpp.h>
#include </opt/rocm/rpp/include/rppdefs.h>
#include </opt/rocm/rpp/include/rppi_image_augumentation_functions.h>

#define C L_USE_DEPRECATED_OPENCL_1_2_APIS
#define LOCAL_SIZE 4
#define MAX_SOURCE_SIZE 2048

#if ENABLE_OPENCL
void exec(void *thequeue, cl_kernel thekernel, size_t *global, size_t *local ){
    // Execute the kernel over the entire range of the data set
    cl_int err;
    err = clEnqueueNDRangeKernel(*(cl_command_queue *)thequeue, thekernel, 2, NULL, global, local,
                                                              0, NULL, NULL);
    // Wait for the command queue to get serviced before reading back results
    clFinish(*(cl_command_queue *)thequeue);
}

void load_kerel_source(char * filename, char *source_str){
        size_t source_size;
        FILE *fp;
        fp = fopen(filename, "r");
        if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
        }
       source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
       fclose( fp );
}


void call_kernel(cl_command_queue queue, cl_mem d_a, cl_mem d_c, unsigned int height,
                                                unsigned int width){
    cl_int err;
    int n = height * width;
    unsigned int min = 0;
    unsigned int max = 255;
    unsigned int new_min = 100;
    unsigned int new_max = 200;
    cl_context context;
    err = clGetCommandQueueInfo(queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &context, NULL);

     char *source_str;
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    load_kerel_source("/home/neel/Mythreyi/kernel/contrast_str.cl",source_str);


    cl_program program;
    cl_kernel kernel;
    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1,
                            (const char **) & source_str, NULL, &err);

    // Build the program executable
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, "contrast_streach", &err);

    // Set the arguments to our compute kernel
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_c);
    err |= clSetKernelArg(kernel, 2, sizeof(int), &min);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &max);
    err |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &new_min);
    err |= clSetKernelArg(kernel, 5, sizeof(unsigned int), &new_max);
    err |= clSetKernelArg(kernel, 6, sizeof(unsigned int), &height);
    err |= clSetKernelArg(kernel, 7, sizeof(unsigned int), &width);

    size_t globalSize, localSize;

    size_t global[2];
    global[0] = height;
    global[1] = width;

    size_t local[2];
    local[0] = LOCAL_SIZE;
    local[1] = LOCAL_SIZE;


    // Number of work items in each local work group
    localSize = 64;

    // Number of total work items - localSize must be devisor
    globalSize = ceil(n/(float)localSize) * localSize;


    exec(&queue, kernel, global, local);

    clReleaseProgram(program);
    clReleaseKernel(kernel);

}
#endif

struct ContrastLocalData {

#if ENABLE_OPENCL
    RPPCommonHandle handle;
#endif
    RppiSize dimensions;
    void * pSrc;
    void * pDst;
    Rpp32u max;
    Rpp32u min;

#if ENABLE_OPENCL
    cl_mem cl_pSrc;
    cl_mem cl_pDst;
#endif

#if ENABLE_HIP
    void *hip_pSrc;
    void *hip_pDst;
#endif

};

static vx_status VX_CALLBACK validatecontrast(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[])
{
 // check scalar max and min type
    vx_status status = VX_SUCCESS;
    vx_enum type;
    vx_parameter param = vxGetParameterByIndex(node, 0);

    STATUS_ERROR_CHECK(vxQueryScalar((vx_scalar)parameters[2], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_INT32) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: Paramter: #2 type=%d (must be size)\n", type);
    STATUS_ERROR_CHECK(vxQueryScalar((vx_scalar)parameters[3], VX_SCALAR_TYPE, &type, sizeof(type)));
    if(type != VX_TYPE_INT32) return ERRMSG(VX_ERROR_INVALID_TYPE, "validate: Paramter: #3 type=%d (must be size)\n", type);

    vx_image image;
    vx_df_image df_image = VX_DF_IMAGE_VIRT;
    STATUS_ERROR_CHECK(vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &image, sizeof(vx_image)));
    STATUS_ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &df_image, sizeof(df_image)));
		if (df_image != VX_DF_IMAGE_U8 || df_image != VX_DF_IMAGE_RGB)
			status = VX_ERROR_INVALID_VALUE;
    STATUS_ERROR_CHECK(vxSetMetaFormatAttribute(metas[1], VX_IMAGE_FORMAT, &df_image, sizeof(df_image)));

     vx_uint32  height, width;
    STATUS_ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height)));
    STATUS_ERROR_CHECK(vxSetMetaFormatAttribute(metas[1], VX_IMAGE_HEIGHT, &height, sizeof(height)));

    STATUS_ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width)));
    STATUS_ERROR_CHECK(vxSetMetaFormatAttribute(metas[1], VX_IMAGE_WIDTH, &width, sizeof(width)));
	vxReleaseImage(&image);

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK processcontrast(vx_node node, const vx_reference * parameters, vx_uint32 num)
{
    ContrastLocalData * data = NULL;
    STATUS_ERROR_CHECK(vxQueryNode(node, VX_NODE_LOCAL_DATA_PTR, &data, sizeof(data)));

#if ENABLE_OPENCL

    cl_command_queue handle = data->handle.cmdq;
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_ATTRIBUTE_AMD_OPENCL_BUFFER, &data->cl_pSrc, sizeof(data->cl_pSrc)));
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[1], VX_IMAGE_ATTRIBUTE_AMD_OPENCL_BUFFER, &data->cl_pDst, sizeof(data->cl_pDst)));
    call_kernel(handle, data->cl_pSrc, data->cl_pDst, data->dimensions.height, data->dimensions.width );
    return VX_SUCCESS;

#else
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_ATTRIBUTE_BUFFER, &data->pSrc, sizeof(vx_uint8)));
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[1], VX_IMAGE_ATTRIBUTE_BUFFER, &data->pDst, sizeof(vx_uint8)));
    rppi_contrast_1C8U_pln_host(data->pSrc, data->dimensions ,data->pDst, data->min, data->max);
    return VX_SUCCESS;
#endif
}

static vx_status VX_CALLBACK initializecontrast(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    ContrastLocalData * data = new ContrastLocalData;
    memset(data, 0, sizeof(*data));

#if ENABLE_OPENCL
    STATUS_ERROR_CHECK(vxQueryNode(node, VX_NODE_ATTRIBUTE_AMD_OPENCL_COMMAND_QUEUE, &data->handle.cmdq, sizeof(data->handle.cmdq)));
#endif

    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_HEIGHT, &data->dimensions.height, sizeof(data->dimensions.height)));
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_WIDTH, &data->dimensions.width, sizeof(data->dimensions.width)));

    STATUS_ERROR_CHECK(vxCopyScalar((vx_scalar)parameters[2], &data->max, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    STATUS_ERROR_CHECK(vxCopyScalar((vx_scalar)parameters[3], &data->min, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

#if ENABLE_OPENCL
    STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_ATTRIBUTE_AMD_OPENCL_BUFFER, &data->cl_pSrc, sizeof(data->cl_pSrc)));
#else
    //STATUS_ERROR_CHECK(vxQueryImage((vx_image)parameters[0], VX_IMAGE_ATTRIBUTE_BUFFER, data->pSrc, sizeof(data->pSrc)));
#endif

    STATUS_ERROR_CHECK(vxSetNodeAttribute(node, VX_NODE_LOCAL_DATA_PTR, &data, sizeof(data)));

    return VX_SUCCESS;
}

static vx_status VX_CALLBACK uninitializecontrast(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

vx_status Contrast_Register(vx_context context)
{
	vx_status status = VX_SUCCESS;
// add kernel to the context with callbacks
    vx_kernel kernel = vxAddUserKernel(context, "org.rpp.Contrast",
            VX_KERNEL_CONTRAST,
            processcontrast,
            4,
            validatecontrast,
            initializecontrast,
            uninitializecontrast);

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
		PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 0, VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED));
		PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 1, VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED));
		PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 2, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED));
		PARAM_ERROR_CHECK(vxAddParameterToKernel(kernel, 3, VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED));
        PARAM_ERROR_CHECK(vxFinalizeKernel(kernel));
	}

	if (status != VX_SUCCESS)
	{
	exit:	vxRemoveKernel(kernel); return VX_FAILURE;
	}

	return status;
}
