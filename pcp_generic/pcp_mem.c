/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_mem.c
 * Created    : not known (before 08/21/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:23:22
 * Comments   : limited as of now 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"
#include "pcp_mem.h"

__inline LPVOID Mem_Alloc(DWORD dwSize)
{
    return (HeapAlloc(GetProcessHeap(),
                    HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
                    dwSize));
}

__inline LPTSTR Mem_AllocStr(DWORD dwSize)
{
    return (HeapAlloc(GetProcessHeap(),
                    HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
                    (dwSize + 1) * sizeof(TCHAR)));
}

__inline LPVOID Mem_ReAlloc(LPVOID lpMem, DWORD dwSize)
{
    return (HeapReAlloc(GetProcessHeap(),
                    HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
                    lpMem,
                    dwSize));
}

__inline LPTSTR Mem_ReAllocStr(LPVOID lpMem, DWORD dwSize)
{
    return (HeapReAlloc(GetProcessHeap(),
                    HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
                    lpMem,
                    (dwSize + 1) * sizeof(TCHAR)));
}

__inline DWORD Mem_Size(LPCVOID lpMem)
{
    return (HeapSize(GetProcessHeap(), 0, lpMem));
}

__inline DWORD Mem_SizeStr(LPCVOID lpMem)
{
    return (HeapSize(GetProcessHeap(), 0, lpMem) / sizeof(TCHAR));
}

__inline BOOL Mem_Validate(LPCVOID lpMem)
{
    return (HeapValidate(GetProcessHeap(), 0, lpMem));
}

__inline BOOL Mem_Free(LPVOID lpMem)
{
    return (HeapFree(GetProcessHeap(), 0, lpMem));
}
