/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_paint
 *
 * File       : pcp_paint_dll.h
 * Created    : 06/27/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 13:24:49
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_PAINT_DLL_H
#define __PCP_PAINT_DLL_H

#ifdef FOREXPORT
#undef FOREXPORT
#endif /* FOREXPORT */

#ifdef PCP_PAINT_EXPORTS
#define FOREXPORT __declspec(dllexport)
#else
#define FOREXPORT __declspec(dllimport)
#endif /* PCP_PAINT_EXPORTS */

FOREXPORT HINSTANCE PCPPaint_GetHandle(void);

#endif /* __PCP_PAINT_DLL_H */
