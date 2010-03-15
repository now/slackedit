/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_dpa.h
 * Created    : not known (before 12/01/99)
 * Owner      : pcppopper
 * Revised on : 06/28/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_DPA_H
#define __PCP_DPA_H

#include "pcp_generic.h"

typedef struct tagDPA
{
	int cCount;
	LPVOID *pp;
	HANDLE hHeap;
	int cAlloc;
	int cGrow;
} DPA, *HDPA;

FOREXPORT HDPA DPA_Create(int cGrow);
FOREXPORT HDPA DPA_CreateEx(int cGrow, HANDLE hHeap);
FOREXPORT BOOL DPA_Destroy(HDPA hdpa);
FOREXPORT HDPA DPA_Clone(HDPA hdpa, HDPA hdpaNew);
FOREXPORT LPVOID DPA_GetPtr(HDPA hdpa, int iIndex);
FOREXPORT int DPA_GetPtrIndex(HDPA hdpa, LPVOID p);
FOREXPORT BOOL DPA_Grow(HDPA pdpa, int cAlloc);
FOREXPORT BOOL DPA_SetPtr(HDPA hdpa, int iIndex, LPVOID p);
FOREXPORT int DPA_InsertPtr(HDPA hdpa, int iIndex, LPVOID p);
FOREXPORT LPVOID DPA_DeletePtr(HDPA hdpa, int iIndex);
FOREXPORT BOOL DPA_DeleteAllPtrs(HDPA hdpa);
FOREXPORT BOOL DPA_DeleteRange(HDPA hdpa, int iIndex, int iCount);
FOREXPORT int DPA_GetCount(HDPA pdpa);

#define DPA_GetPtrPtr(hdpa)      (*((LPVOID * *)((BYTE *)(hdpa) + sizeof(int))))
#define DPA_FastGetPtr(hdpa, i)  (DPA_GetPtrPtr(hdpa)[i])

typedef int (CALLBACK *LPFNDPACOMPARE)(LPVOID lpEin, LPVOID lpZwei, LPARAM lParam);

FOREXPORT BOOL DPA_Sort(HDPA hdpa, LPFNDPACOMPARE lpfnCompare, LPARAM lParam);

typedef struct _DSA * HDSA;

// Search array.  If DPAS_SORTED, then array is assumed to be sorted
// according to pfnCompare, and binary search algorithm is used.
// Otherwise, linear search is used.
//
// Searching starts at iStart (-1 to start search at beginning).
//
// DPAS_INSERTBEFORE/AFTER govern what happens if an exact match is not
// found.  If neither are specified, this function returns -1 if no exact
// match is found.  Otherwise, the index of the item before or after the
// closest (including exact) match is returned.
//
// Search option flags
//
#define DPAS_SORTED             0x0001
#define DPAS_INSERTBEFORE       0x0002
#define DPAS_INSERTAFTER        0x0004

FOREXPORT int DPA_Search(HDPA hdpa, LPVOID lpFind, int nStart,
                      LPFNDPACOMPARE lpfnCompare,
                      LPARAM lParam, UINT uOptions);

#endif /* __PCP_DPA_H */
