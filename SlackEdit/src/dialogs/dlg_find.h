/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : find.h
 * Created    : not known (before 06/22/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:10:29
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __DLG_FIND_H
#define __DLG_FIND_H

void Find_Edit_CreateDialog(HWND hwnd, BOOL bReplace);
BOOL Find_FindText(HWND hwnd, LPCTSTR pszText, POINT ptCurrentPos, BOOL bVisual);
void Find_AutoComplete(HWND hwnd);

#endif /* __DLG_FIND_H */
