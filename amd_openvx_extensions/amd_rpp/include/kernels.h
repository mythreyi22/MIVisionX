/*

MCW


*/



#ifndef _VX_EXT_RPP_H_
#define _VX_EXT_RPP_H_


#include <VX/vx.h>

enum rpp_library
{
  RPP_LIBRARY = 1,
};
enum user_kernel_e
{
    VX_KERNEL_BRIGHTNESS_AND_CONSTAST  = VX_KERNEL_BASE(VX_ID_AMD, RPP_LIBRARY) + 0x001,
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