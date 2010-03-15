/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_mem.h
 * Created    : not known (before 08/21/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:23:14
 * Comments   : limited as of now 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_MEM_H
#define __PCP_MEM_H

#include "pcp_generic.h"

FOREXPORT __inline LPVOID Mem_Alloc(DWORD dwSize);
FOREXPORT __inline LPVOID Mem_ReAlloc(LPVOID lpMem, DWORD dwSize);
FOREXPORT __inline DWORD Mem_Size(LPCVOID lpMem);
FOREXPORT __inline BOOL Mem_Validate(LPCVOID lpMem);
FOREXPORT __inline BOOL Mem_Free(LPVOID lpMem);

FOREXPORT __inline LPTSTR Mem_AllocStr(DWORD dwSize);
FOREXPORT __inline LPTSTR Mem_ReAllocStr(LPVOID lpMem, DWORD dwSize);
FOREXPORT __inline DWORD Mem_SizeStr(LPCVOID lpMem);

#endif /* __PP_MEM_H */
