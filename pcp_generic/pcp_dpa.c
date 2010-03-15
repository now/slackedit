/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_dpa.c
 * Created    : not known (before 12/01/99)
 * Owner      : pcppopper
 * Revised on : 06/28/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"

#include "pcp_dpa.h"
#include "pcp_mem.h"

typedef struct
{
    LPVOID *lpPointers;
    LPFNDPACOMPARE lpfnCompare;
    LPARAM lParam;
    int cPointers;
    LPVOID *lpTempPointers;
} SORTPARAMS, *LPSORTPARAMS;

BOOL DPA_MergeSort(LPSORTPARAMS lppsp);
void DPA_MergeSort2(LPSORTPARAMS lppsp, int nFirst, int cItems);

//================== Dynamic pointer array implementation ===========


HANDLE g_hSharedHeap = NULL;

LPVOID DPA_Alloc(long cb)
{
    // I will assume that this is the only one that needs the checks to
    // see if the heap has been previously created or not

    if (g_hSharedHeap == NULL)
    {
        g_hSharedHeap = HeapCreate(0, 1, 0);

        // If still NULL we have problems!
        if (g_hSharedHeap == NULL)
            return(NULL);
    }

    return (HeapAlloc(g_hSharedHeap, HEAP_ZERO_MEMORY, cb));
}

LPVOID DPA_ReAlloc(void * pb, long cb)
{
    if (pb == NULL)
        return (DPA_Alloc(cb));

    return (HeapReAlloc(g_hSharedHeap, HEAP_ZERO_MEMORY, pb, cb));
}

BOOL DPA_Free(void * pb)
{
    return (HeapFree(g_hSharedHeap, 0, pb));
}

/**************************************************************************
 * DPA_Create [COMCTL32.328] Creates a dynamic pointer array
 *
 * PARAMS
 *     nGrow [I] number of items by which the array grows when it is filled
 *
 * RETURNS
 *     Success: handle (pointer) to the pointer array.
 *     Failure: NULL
 */

HDPA DPA_Create(int cGrow)
{
    HDPA hdpa = DPA_Alloc(sizeof(DPA));
    
    if (hdpa)
    {
        hdpa->cCount    = 0;
        hdpa->cAlloc    = 0;
        hdpa->cGrow     = max(8, cGrow);
        hdpa->pp        = NULL;
        hdpa->hHeap     = g_hSharedHeap;   // Defaults to use shared one (for now...)
    }

    return (hdpa);
}

/**************************************************************************
 * DPA_CreateEx [COMCTL32.340]
 *
 * Creates a dynamic pointer array using the specified size and heap.
 *
 * PARAMS
 *     nGrow [I] number of items by which the array grows when it is filled
 *     hHeap [I] handle to the heap where the array is stored
 *
 * RETURNS
 *     Success: handle (pointer) to the pointer array.
 *     Failure: NULL
 */

// Should nuke the standard DPA above...
HDPA DPA_CreateEx(int cGrow, HANDLE hHeap)
{
    HDPA hdpa;

    if (hHeap == NULL)
    {
        hdpa = DPA_Alloc(sizeof(DPA));
        hHeap = g_hSharedHeap;
    }
    else
    {
        hdpa = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(DPA));
    }
    
    if (hdpa != NULL)
    {
        hdpa->cCount    = 0;
        hdpa->cAlloc    = 0;
        hdpa->cGrow     = max(8, cGrow);
        hdpa->pp        = NULL;
        hdpa->hHeap     = hHeap;
    }

    return (hdpa);
}

/**************************************************************************
 * DPA_Destroy [COMCTL32.329] Destroys a dynamic pointer array
 *
 * PARAMS
 *     hdpa [I] handle (pointer) to the pointer array
 *
 * RETURNS
 *     Success: TRUE
 *     Failure: FALSE
 */

BOOL DPA_Destroy(HDPA hdpa)
{
    if (hdpa == NULL)
        return (TRUE);

    ASSERT(hdpa->hHeap);

    if (hdpa->pp && !HeapFree(hdpa->hHeap, 0, hdpa->pp))
        return (FALSE);

    return (HeapFree(hdpa->hHeap, 0, hdpa));
}

/**************************************************************************
 * DPA_Clone [COMCTL32.331]
 *
 * Copies a pointer array to an other one or creates a copy
 *
 * PARAMS
 *     hdpa    [I] handle (pointer) to the existing (source) pointer array
 *     hdpaNew [O] handle (pointer) to the destination pointer array
 *
 * RETURNS
 *     Success: pointer to the destination pointer array.
 *     Failure: NULL
 *
 * NOTES
 *     - If the 'hdpaNew' is a NULL-Pointer, a copy of the source pointer
 *       array will be created and it's handle (pointer) is returned.
 *     - If 'hdpa' is a NULL-Pointer, the original implementation crashes,
 *       this implementation just returns NULL.
 */

HDPA DPA_Clone(HDPA hdpa, HDPA hdpaNew)
{
    BOOL fAlloc = FALSE;

    /* The want to copy an empty DPA? */
    if (hdpa == NULL)
        return (NULL); /* OMG! They thought they could get away with it! */

    if (hdpaNew == NULL)
    {
        hdpaNew = DPA_CreateEx(hdpa->cGrow, hdpa->hHeap);
        
        if (hdpaNew == NULL)
            return (NULL);

        fAlloc = TRUE;
    }

    if (!DPA_Grow(hdpaNew, hdpa->cAlloc))
    {
        if (!fAlloc)
            DPA_Destroy(hdpaNew);
    
        return (NULL);
    }

    hdpaNew->cCount = hdpa->cCount;
    memcpy(hdpaNew->pp, hdpa->pp, (hdpa->cCount * sizeof(LPVOID)));

    return (hdpaNew);
}

/**************************************************************************
 * DPA_GetPtr [COMCTL32.332]
 *
 * Retrieves a pointer from a dynamic pointer array
 *
 * PARAMS
 *     hdpa   [I] handle (pointer) to the pointer array
 *     nIndex [I] array index of the desired pointer
 *
 * RETURNS
 *     Success: pointer
 *     Failure: NULL
 */

LPVOID DPA_GetPtr(HDPA hdpa, int iIndex)
{
    if (hdpa == NULL || hdpa->pp == NULL)
        return (NULL);

    if (iIndex < 0 || iIndex >= hdpa->cCount)
        return (NULL);

    return (hdpa->pp[iIndex]);
}

/**************************************************************************
 * DPA_GetPtrIndex [COMCTL32.333]
 *
 * Retrieves the index of the specified pointer
 *
 * PARAMS
 *     hdpa   [I] handle (pointer) to the pointer array
 *     p      [I] pointer
 *
 * RETURNS
 *     Success: index of the specified pointer
 *     Failure: -1
 */

int DPA_GetPtrIndex(HDPA hdpa, LPVOID p)
{
    LPVOID *pp;
    LPVOID *ppMax;

    if (hdpa->pp != NULL)
    {
        pp = hdpa->pp;
        ppMax = pp + hdpa->cCount;
    
        for ( ; pp < ppMax; pp++)
        {
            if (*pp == p)
                return (pp - hdpa->pp);
        }
    }

    return (-1);
}

/**************************************************************************
 * DPA_Grow [COMCTL32.330]
 *
 * Sets the growth amount.
 *
 * PARAMS
 *     hdpa  [I] handle (pointer) to the existing (source) pointer array
 *     cGrow [I] number of items, the array grows, when it's too small
 *
 * RETURNS
 *     Success: TRUE
 *     Failure: FALSE
 */

BOOL DPA_Grow(HDPA hdpa, int cAlloc)
{
    if (cAlloc > hdpa->cAlloc)
    {
        LPVOID *ppNew;

        cAlloc = ((cAlloc + hdpa->cGrow - 1) / hdpa->cGrow) * hdpa->cGrow;

        if (hdpa->pp)
            ppNew = (LPVOID *)HeapReAlloc(hdpa->hHeap, HEAP_ZERO_MEMORY, hdpa->pp, (cAlloc * sizeof(LPVOID)));
        else
            ppNew = (LPVOID *)HeapAlloc(hdpa->hHeap, HEAP_ZERO_MEMORY, (cAlloc * sizeof(LPVOID)));

        if (ppNew == NULL)
            return (FALSE);

        hdpa->pp = ppNew;
        hdpa->cAlloc = cAlloc;
    }

    return (TRUE);
}

/**************************************************************************
 * DPA_SetPtr [COMCTL32.335]
 *
 * Sets a pointer in the pointer array
 *
 * PARAMS
 *     hdpa [I] handle (pointer) to the pointer array
 *     i    [I] index of the pointer that will be set
 *     p    [I] pointer to be set
 *
 * RETURNS
 *     Success: TRUE
 *     Failure: FALSE
 */

BOOL DPA_SetPtr(HDPA hdpa, int iIndex, LPVOID p)
{
    if (iIndex < 0)
        return (FALSE);

    if (iIndex >= hdpa->cCount)
    {
        if (!DPA_Grow(hdpa, iIndex + 1))
            return (FALSE);

        hdpa->cCount = iIndex + 1;
    }

    hdpa->pp[iIndex] = p;

    return (TRUE);
}

/**************************************************************************
 * DPA_InsertPtr [COMCTL32.334]
 *
 * Inserts a pointer into a dynamic pointer array
 *
 * PARAMS
 *     hdpa [I] handle (pointer) to the array
 *     i    [I] array index
 *     p    [I] pointer to insert
 *
 * RETURNS
 *     Success: index of the inserted pointer
 *     Failure: -1
 */

int DPA_InsertPtr(HDPA hdpa, int iIndex, LPVOID p)
{
    if (hdpa == NULL)
        return (-1);

    if ((iIndex > hdpa->cCount) || (iIndex == -1))
        iIndex = hdpa->cCount;

    // Make sure we have room for one more item
    //
    if ((hdpa->cCount + 1) > hdpa->cAlloc)
    {
        if (!DPA_Grow(hdpa, hdpa->cCount + 1))
            return (-1);
    }

    // If we are inserting, we need to slide everybody up
    //
    if (iIndex < hdpa->cCount)
    {
        // originally memcpy() was called here but memcpy()
        // can't handle overlapping memory so can't be used successfully!
        // memmove() needs to be used in this case...
        memmove(&hdpa->pp[iIndex + 1], &hdpa->pp[iIndex],
            (hdpa->cCount - iIndex) * sizeof(LPVOID));
    }

    hdpa->pp[iIndex] = p;
    hdpa->cCount++;

    return (iIndex);
}

/**************************************************************************
 * DPA_DeletePtr [COMCTL32.336]
 *
 * Removes a pointer from the pointer array.
 *
 * PARAMS
 *     hdpa [I] handle (pointer) to the pointer array
 *     i    [I] index of the pointer that will be deleted
 *
 * RETURNS
 *     Success: deleted pointer
 *     Failure: NULL
 */

LPVOID DPA_DeletePtr(HDPA hdpa, int iIndex)
{
    LPVOID p;

    if ((hdpa == NULL) || (iIndex < 0) || (iIndex >= hdpa->cCount))
    {
        return (NULL);
    }

    p = hdpa->pp[iIndex];

    if (iIndex < (hdpa->cCount - 1))
    {
        memmove(&hdpa->pp[iIndex], &hdpa->pp[iIndex + 1],
            (hdpa->cCount - (iIndex + 1)) * sizeof(LPVOID));
    }

    hdpa->cCount--;

    if ((hdpa->cAlloc - hdpa->cCount) > hdpa->cGrow)
    {
        LPVOID *ppNew;

        ppNew = HeapReAlloc(hdpa->hHeap, HEAP_ZERO_MEMORY, hdpa->pp,
                        (hdpa->cAlloc - hdpa->cGrow) * sizeof(LPVOID));

        ASSERT(ppNew);

        hdpa->pp = ppNew;
        hdpa->cAlloc -= hdpa->cGrow;
    }

    return (p);
}

BOOL DPA_DeleteRange(HDPA hdpa, int iIndex, int iCount)
{
    int iMoveCount = hdpa->cCount - (iIndex + iCount);

    if ((hdpa == NULL) || (iIndex < 0) || (iIndex >= hdpa->cCount))
    {
        return (FALSE);
    }

    if (iMoveCount)
    {
        memmove(&hdpa->pp[iIndex], &hdpa->pp[iIndex + iCount],
                iMoveCount * sizeof (LPVOID));
    }

    hdpa->cCount -= iCount;

    // check this!
    if ((hdpa->cAlloc - hdpa->cCount) > hdpa->cGrow)
    {
        LPVOID *ppNew;

        ppNew = HeapReAlloc(hdpa->hHeap, HEAP_ZERO_MEMORY, hdpa->pp,
                        (hdpa->cAlloc - hdpa->cGrow) * sizeof(LPVOID));

        ASSERT(ppNew);

        hdpa->pp = ppNew;
        hdpa->cAlloc -= hdpa->cGrow;
    }

    return (TRUE);
}   
/**************************************************************************
 * DPA_DeleteAllPtrs [COMCTL32.337]
 *
 * Removes all pointers and reinitializes the array.
 *
 * PARAMS
 *     hdpa [I] handle (pointer) to the pointer array
 *
 * RETURNS
 *     Success: TRUE
 *     Failure: FALSE
 */

BOOL DPA_DeleteAllPtrs(HDPA hdpa)
{
    if (hdpa == NULL || hdpa->pp == NULL || !HeapFree(hdpa->hHeap, 0, hdpa->pp))
        return (FALSE);

    hdpa->pp        = NULL;
    hdpa->cCount    = hdpa->cAlloc = 0;
    
    return (TRUE);
}

BOOL DPA_Sort(HDPA hdpa, LPFNDPACOMPARE lpfnCompare, LPARAM lParam)
{
    SORTPARAMS sp;

    sp.cPointers = hdpa->cCount;
    sp.lpPointers = hdpa->pp;
    sp.lpfnCompare = lpfnCompare;
    sp.lParam = lParam;

    return (DPA_MergeSort(&sp));
}

#define SortCompare(lpsp, lpp1, i1, lpp2, i2) \
    (lpsp->lpfnCompare(lpp1[i1], lpp2[i2], lpsp->lParam))

//
//  This function merges two sorted lists and makes one sorted list.
//   psp->pp[iFirst, iFirst+cItes/2-1], psp->pp[iFirst+cItems/2, iFirst+cItems-1]
//
void DPA_MergeThem(LPSORTPARAMS lpsp, int nFirst, int cItems)
{
    //
    // Notes:
    //  This function is separated from DPA_MergeSort2() to avoid comsuming
    // stack variables. Never inline this.
    //
    int cHalf = (cItems / 2);
    int iIn1, iIn2, iOut;
    LPVOID *lppvSrc = &lpsp->lpPointers[nFirst];

    // Copy the first part to temp storage so we can write directly into
    // the final buffer.  Note that this takes at most lpsp->cp/2 DWORD's
    memcpy(lpsp->lpTempPointers, lppvSrc, cHalf * sizeof(LPVOID));

    for (iIn1 = 0, iIn2 = cHalf, iOut = 0; ; )
    {
        if (SortCompare(lpsp, lpsp->lpTempPointers, iIn1, lppvSrc, iIn2) <= 0) {
            lppvSrc[iOut++] = lpsp->lpTempPointers[iIn1++];

            if (iIn1 == cHalf)
            {
                // We used up the first half; the rest of the second half
                // should already be in place
                break;
            }
        }
        else
        {
            lppvSrc[iOut++] = lppvSrc[iIn2++];

            if (iIn2 == cItems)
            {
                // We used up the second half; copy the rest of the first half
                // into place
                memcpy(&lppvSrc[iOut], &lpsp->lpTempPointers[iIn1], (cItems - iOut) * sizeof(LPVOID));

                break;
            }
        }
    }
}

//
//  This function sorts a give list (lpsp->lpp[iFirst,iFirst - cItems - 1]).
//
void DPA_MergeSort2(LPSORTPARAMS lpsp, int nFirst, int cItems)
{
    //
    // Notes:
    //   This function is recursively called. Therefore, we should minimize
    //  the number of local variables and parameters. At this point, we
    //  use one local variable and three parameters.
    //
    int cHalf;

    switch(cItems)
    {
    case 1:
    return;
    case 2:
        // Swap them, if they are out of order.
        if (SortCompare(lpsp, lpsp->lpPointers, nFirst, lpsp->lpPointers, nFirst + 1) > 0)
        {
            lpsp->lpTempPointers[0] = lpsp->lpPointers[nFirst];
            lpsp->lpPointers[nFirst] = lpsp->lpPointers[nFirst + 1];
            lpsp->lpPointers[nFirst + 1] = lpsp->lpTempPointers[0];
        }
    break;
    default:
        cHalf = (cItems / 2);
        // Sort each half
        DPA_MergeSort2(lpsp, nFirst, cHalf);
        DPA_MergeSort2(lpsp, nFirst + cHalf, cItems - cHalf);
        // Then, merge them.
        DPA_MergeThem(lpsp, nFirst, cItems);
    break;
    }
}

BOOL DPA_MergeSort(LPSORTPARAMS lpsp)
{
    if (lpsp->cPointers == 0)
        return (TRUE);

    // Note that we divide by 2 below; we want to round down
    lpsp->lpTempPointers = Mem_Alloc((lpsp->cPointers / 2) * sizeof(LPVOID));

    if (lpsp->lpTempPointers == NULL)
        return (FALSE);

    DPA_MergeSort2(lpsp, 0, lpsp->cPointers);
    Mem_Free(lpsp->lpTempPointers);

    return (TRUE);
}

// Search function
//
int DPA_Search(HDPA pdpa, LPVOID lpFind, int nStart,
            LPFNDPACOMPARE lpfnCompare, LPARAM lParam, UINT uOptions)
{
    int cPointers = DPA_GetCount(pdpa);

    ASSERT(lpfnCompare != NULL);
    ASSERT(0 <= nStart);

    // Only allow these wierd flags if the list is sorted
    ASSERT((uOptions & DPAS_SORTED) || !(uOptions & (DPAS_INSERTBEFORE | DPAS_INSERTAFTER)));

    if (!(uOptions & DPAS_SORTED))
    {
        // Not sorted: do lisearch.
        int i;

        for (i = nStart; i < cPointers; i++)
        {
            if (0 == lpfnCompare(lpFind, DPA_FastGetPtr(pdpa, i), lParam))
                return (i);
        }

        return (-1);
    }
    else
    {
        // Search the array using binary search.  If several adjacent 
        // elements match the target element, the index of the first
        // matching element is returned.

        int nRet = -1;      // assume no match
        BOOL bFound = FALSE;
        int nCmp = 0;
        int nLow = 0;       // Don't bother using iStart for binary search
        int nMid = 0;
        int nHigh = cPointers - 1;

        // (OK for cp == 0)
        while (nLow <= nHigh)
        {
            nMid = (nLow + nHigh) / 2;

            nCmp = lpfnCompare(lpFind, DPA_FastGetPtr(pdpa, nMid), lParam);

            if (0 > nCmp)
            {
                nHigh = nMid - 1;       // First is smaller
            }
            else if (0 < nCmp)
            {
                nLow = nMid + 1;        // First is larger
            }
            else
            {
                // Match; search back for first match
                bFound = TRUE;

                while (0 < nMid)
                {
                    if (0 != lpfnCompare(lpFind, DPA_FastGetPtr(pdpa, nMid - 1), lParam))
                        break;
                    else
                        nMid--;
                }

                break;
            }
        }

        if (bFound)
        {
            ASSERT(0 <= nMid);
            nRet = nMid;
        }

        // Did the search fail AND
        // is one of the strange search flags set?
        if (!bFound && (uOptions & (DPAS_INSERTAFTER | DPAS_INSERTBEFORE)))
        {
            // Yes; return the index where the target should be inserted
            // if not found
            if (0 < nCmp)       // First is larger
                nRet = nLow;
            else
                nRet = nMid;
            // (We don't distinguish between the two flags anymore)
        }
        else if ( !(uOptions & (DPAS_INSERTAFTER | DPAS_INSERTBEFORE)) )
        {
            // Sanity check with lisearch
            ASSERT(DPA_Search(pdpa, lpFind, nStart, lpfnCompare, lParam, uOptions & ~DPAS_SORTED) == nRet);
        }

        return (nRet);
    }
}

//===========================================================================
//
// String ptr management routines
//
// Copy as much of *psz to *pszBuf as will fit
//
int Str_GetPtr(LPCTSTR psz, LPTSTR pszBuf, int cchBuf)
{
    int cch = 0;

    // if pszBuf is NULL, just return length of string.
    //
    if (pszBuf == NULL && psz)
        return (_tcslen(psz));

    if (cchBuf)
    {
        if (psz != NULL)
        {
            cch = _tcslen(psz);

            if (cch > cchBuf - 1)
                cch = cchBuf - 1;

            memcpy(pszBuf, psz, cch);
        }

        pszBuf[cch] = 0;
    }

    return (cch);
}

BOOL Str_Set(LPTSTR *ppsz, LPCTSTR psz)
{
    if (psz == NULL)
    {
        if (*ppsz != NULL)
        {
            Mem_Free(*ppsz);
            *ppsz = NULL;
        }
    }
    else
    {
        LPTSTR pszNew;
        UINT cbSize = _tcslen(psz) + 1;

        if (*ppsz != NULL)
            pszNew = Mem_ReAlloc(*ppsz, cbSize);
        else
            pszNew = Mem_Alloc(cbSize);

        if (pszNew == NULL)
            return (FALSE);

        _tcscpy(pszNew, psz);
        *ppsz = pszNew;
    }

    return (TRUE);
}

// Set *ppsz to a copy of psz, reallocing as needed
//
BOOL Str_SetPtr(LPTSTR *ppsz, LPCTSTR psz)
{
    if (psz == NULL)
    {
        if (*ppsz != NULL)
        {
            DPA_Free(*ppsz);
            *ppsz = NULL;
        }
    }
    else
    {
        LPTSTR pszNew = (LPTSTR)DPA_ReAlloc(*ppsz, lstrlen(psz) + 1);

        if (pszNew == NULL)
            return (FALSE);

        _tcscpy(pszNew, psz);
        *ppsz = pszNew;
    }

    return (TRUE);
}

int DPA_GetCount(HDPA pdpa)
{
    if (pdpa == NULL)
        return (0);

    return ((*(int *)(pdpa)));
}
