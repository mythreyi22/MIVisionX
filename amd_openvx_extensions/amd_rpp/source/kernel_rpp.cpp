#include "kernels_rpp.h"

VX_API_ENTRY vx_node VX_API_CALL vxExtrppNode_brightness(vx_graph graph, vx_array pSrc, vx_array pDst, vx_int32  height, vx_int32 width, vx_float32 alpha, vx_float32 beta)
{
    vx_node node = NULL;
    vx_context context = vxGetContext((vx_reference)graph);
    if(vxGetStatus((vx_reference)context) == VX_SUCCESS) {
        vx_scalar ALPHA = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &alpha);
        vx_scalar BETA = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &beta);
        vx_scalar HEIGHT = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &height);
        vx_scalar WIDTH = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &width);

            vx_reference params[] = {
                (vx_reference) pSrc,
                (vx_reference) pDst,
                (vx_reference) HEIGHT,
                (vx_reference) WIDTH,
                (vx_reference) ALPHA,
                (vx_reference) BETA
        };
            node = createNode(graph, VX_KERNEL_BRIGHTNESS_AND_CONTRAST, params, 6);
    }
    return node;
}

// utility functions
vx_node createNode(vx_graph graph, vx_enum kernelEnum, vx_reference params[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vx_node node = 0;
    vx_context context = vxGetContext((vx_reference)graph);
    if(vxGetStatus((vx_reference)context) != VX_SUCCESS) {
        return NULL;
    }
    vx_kernel kernel = vxGetKernelByEnum(context, kernelEnum);
    if(vxGetStatus((vx_reference)kernel) == VX_SUCCESS) {
        node = vxCreateGenericNode(graph, kernel);
        if (node) {
            vx_uint32 p = 0;
            for (p = 0; p < num; p++) {
                if (params[p]) {
                    status = vxSetParameterByIndex(node, p, params[p]);
                    if (status != VX_SUCCESS) {
                        char kernelName[VX_MAX_KERNEL_NAME];
                        vxQueryKernel(kernel, VX_KERNEL_NAME, kernelName, VX_MAX_KERNEL_NAME);
                        vxAddLogEntry((vx_reference)graph, status, "createNode: vxSetParameterByIndex(%s, %d, 0x%p) => %d\n", kernelName, p, params[p], status);
                        vxReleaseNode(&node);
                        node = 0;
                        break;
                    }
                }
            }
        }
        else {
            vxAddLogEntry((vx_reference)graph, VX_ERROR_INVALID_PARAMETERS, "createNode: failed to create node with kernel enum %d\n", kernelEnum);
            status = VX_ERROR_NO_MEMORY;
        }
        vxReleaseKernel(&kernel);
    }
    else {
        vxAddLogEntry((vx_reference)graph, VX_ERROR_INVALID_PARAMETERS, "createNode: failed to retrieve kernel enum %d\n", kernelEnum);
        status = VX_ERROR_NOT_SUPPORTED;
    }
    return node;
}
