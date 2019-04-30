/*

MCW


*/



#ifndef _VX_EXT_RPP_H_
#define _VX_EXT_RPP_H_

#include <VX/vx.h>
#include "kernels_rpp.h"


	/*!***********************************************************************************************************
						RPP VX_API_ENTRY C Function NODE
	*************************************************************************************************************/

extern "C" SHARED_PUBLIC vx_node VX_API_CALL vxExtrppNode_brightness(vx_graph graph, vx_array pSrc, vx_array pDst, vx_int32  height, vx_int32 width, vx_float32 alpha, vx_float32 beta);

#endif
