/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_listbox.c
 * Created    : not known (before 12/01/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:22:36
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"
#include "pcp_listbox.h"

#include "pcp_string.h"
#include "pcp_mem.h"

BOOL ListBox_DeleteAllStrings(HWND hwndListBox)
{
    BOOL bSuccess = FALSE;
    int iCount = ListBox_GetSelCount(hwndListBox);
    LPINT lpiItems = (LPINT)_alloca(iCount * sizeof(INT));
    ListBox_GetSelItems(hwndListBox, iCount, lpiItems);

    SetWindowRedraw(hwndListBox, FALSE);


    for ( ; iCount > 0; iCount--)
    {
        if (ListBox_DeleteString(hwndListBox, lpiItems[iCount - 1]) != LB_ERR)
            bSuccess = TRUE;
    }

    SetWindowRedraw(hwndListBox, TRUE);

    return (bSuccess);
}

int ListBox_AddNoDuplicateString(HWND hwndListBox, LPTSTR pszString)
{
    int iCount = ListBox_GetCount(hwndListBox), iIndex, iLen;
    LPTSTR pszBuffer;

    for (iIndex = 0; iIndex < iCount; iIndex++)
    {
        iLen = ListBox_GetTextLen(hwndListBox, iIndex);
        pszBuffer = (LPTSTR)Mem_AllocStr(iLen + SZ); /* SZ to be sure */
        if (ListBox_GetText(hwndListBox, iIndex, pszBuffer) != LB_ERR);
            if (_tcsicmp(pszString, pszBuffer) == 0)
                return (-1);

        Mem_Free(pszBuffer);
    }

    return (ListBox_AddString(hwndListBox, pszString));
}

int ListBox_MoveString(HWND hwndListBox, int iIndex, int iNewIndex, BOOL bRelativeToOld)
{
    int iCount = ListBox_GetCount(hwndListBox);
    int nExactNewIndex;

    if (iIndex == 0 && iNewIndex < 0)
        iNewIndex = 0;

    nExactNewIndex = bRelativeToOld ? (iIndex + iNewIndex) : iNewIndex;

    if ((bRelativeToOld && (iIndex + iNewIndex) >= iCount) ||
        (iNewIndex >= iCount))
    {
        return (LB_ERR);
    }
    else
    {
        LPTSTR pszBuffer = (LPTSTR)Mem_AllocStr(ListBox_GetTextLen(hwndListBox, iIndex) + SZ);
        LPVOID lpVoid = (LPVOID)ListBox_GetItemData(hwndListBox, iIndex);

        ListBox_GetText(hwndListBox, iIndex, pszBuffer);
        ListBox_DeleteString(hwndListBox, iIndex);
        ListBox_InsertString(hwndListBox, nExactNewIndex, pszBuffer);
        ListBox_SetItemData(hwndListBox, nExactNewIndex, lpVoid);
    
        Mem_Free(pszBuffer);
    }

    return (nExactNewIndex);
}

int ListBox_MoveStringUp(HWND hwndList, int nIndex)
{
    int nNewIndex;

    SetWindowRedraw(hwndList, FALSE);
    nNewIndex = ListBox_MoveString(hwndList, nIndex, -1, TRUE);
    ASSERT(nNewIndex != LB_ERR);
    if (ListBox_SetSel(hwndList, TRUE, nNewIndex) == LB_ERR)
    {
        // Try and use ListBox_SetCurSel() (for single selection listbox)
        ListBox_SetCurSel(hwndList, nNewIndex);
    }
    SetWindowRedraw(hwndList, TRUE);
    RedrawWindow(hwndList, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

    return (nNewIndex);
}

int ListBox_MoveStringDown(HWND hwndList, int nIndex)
{
    int nNewIndex;

    SetWindowRedraw(hwndList, FALSE);
    nNewIndex = ListBox_MoveString(hwndList, nIndex, 1, TRUE);
    ASSERT(nNewIndex != LB_ERR);
    if (ListBox_SetSel(hwndList, TRUE, nNewIndex) == LB_ERR)
    {
        // Try and use ListBox_SetCurSel() (for single selection listbox)
        ListBox_SetCurSel(hwndList, nNewIndex);
    }
    SetWindowRedraw(hwndList, TRUE);
    RedrawWindow(hwndList, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

    return (nIndex);
}

void ListBox_MoveAllStrings(HWND hwndListBox, int iAmount)
{
    int iCount = ListBox_GetSelCount(hwndListBox), i;
    LPINT lpiItems = (LPINT)Mem_Alloc(iCount * sizeof(INT));

    SetWindowRedraw(hwndListBox, FALSE);

    ListBox_GetSelItems(hwndListBox, iCount, lpiItems);

    if (iAmount < 0)
    {
        for (i = 0; i < iCount; i++)
        {
            int nIndex = ListBox_MoveString(hwndListBox, lpiItems[i], iAmount, TRUE);
            ASSERT(nIndex != LB_ERR);
        }
    }
    else
    {
        for (i = (iCount - 1); i > -1; i--)
        {
            int nIndex = ListBox_MoveString(hwndListBox, lpiItems[i], iAmount, TRUE);
            ASSERT(nIndex != LB_ERR);
        }
    }

    // Reselect all the items moved
    for (i = 0; i < iCount; i++)
        ListBox_SetSel(hwndListBox, TRUE, lpiItems[i] + iAmount);

    SetWindowRedraw(hwndListBox, TRUE);
    RedrawWindow(hwndListBox, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

    Mem_Free(lpiItems);
}

/*************************************************
 * ListBox_GetFirstSel
 *
 * hwndListBox  -   HWND to a listbox control
 *
 * returns index of first selected item in the listbox control
 * if no items are selected it returns LB_ERR
 *************************************************/

int ListBox_GetFirstSel(HWND hwndListBox)
{
    int iIndex;

    ListBox_GetSelItems(hwndListBox, 1, &iIndex);

    return (iIndex);
}
