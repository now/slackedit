/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_dll.h
 * Created    : 07/23/00
 * Owner      : pcppopper
 * Revised on : 07/23/00
 * Comments   : 
 *              
 *              
 ****************************************************************/

#ifndef __PCP_EDIT_DLL_H
#define __PCP_EDIT_DLL_H

#ifdef FOREXPORT
#undef FOREXPORT
#endif /* FOREXPORT */

#ifdef PCP_EDIT_EXPORTS
#define FOREXPORT __declspec(dllexport)
#else
#define FOREXPORT __declspec(dllimport)
#endif /* PCP_EDIT_EXPORTS */

FOREXPORT HINSTANCE PCP_Edit_Dll_GetHandle(void);

#endif /* __PCP_EDIT_DLL_H */
