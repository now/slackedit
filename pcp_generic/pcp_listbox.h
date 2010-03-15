/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_listbox.h
 * Created    : not known (before 12/01/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:22:31
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __LISTBOX_H
#define __LISTBOX_H

#include "pcp_generic.h"

FOREXPORT BOOL ListBox_DeleteAllStrings(HWND hwndListBox);
FOREXPORT int ListBox_AddNoDuplicateString(HWND hwndListBox, LPTSTR pszString);
FOREXPORT int ListBox_MoveString(HWND hwndListBox, int iIndex, int iNewIndex, BOOL bRelativeToOld);
FOREXPORT int ListBox_GetFirstSel(HWND hwndListBox);
FOREXPORT void ListBox_MoveAllStrings(HWND hwndListBox, int iAmount);

FOREXPORT int ListBox_MoveStringUp(HWND hwndList, int nIndex);
FOREXPORT int ListBox_MoveStringDown(HWND hwndList, int nIndex);


#endif /* __LISTBOX_H */
