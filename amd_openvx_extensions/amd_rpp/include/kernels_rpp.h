/*

MCW


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
    VX_KERNEL_BRIGHTNESS_AND_CONTRAST  = VX_KERNEL_BASE(VX_ID_MCW, VX_LIBRARY_RPP) + 0x001,
};

//////////////////////////////////////////////////////////////////////
//! \brief Common data shared across all nodes in a graph
/*struct NeuralNetworkCommonHandle {
    int count;
    miopenHandle_t  miopen_handle;
    cl_command_queue cmdq;
    bool exhaustiveSearch;
};
*/
//////////////////////////////////////////////////////////////////////
//! \brief The utility functions
vx_node createNode(vx_graph graph, vx_enum kernelEnum, vx_reference params[], vx_uint32 num);

#endif //__KERNELS_H__
