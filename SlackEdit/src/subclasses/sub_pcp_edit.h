/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : PhastPad
 *
 * File       : pcp_edit_child.h
 * Created    : 01/23/00
 * Owner      : pcppopper
 * Revised on : 07/02/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_EDIT_CHILD_H
#define __PCP_EDIT_CHILD_H

#include "../pcp_edit/pcp_edit_view.h"

void EditView_Create(HWND hwndOwner, LPTSTR pszFile);
void TextView_UpdateStatusbar(LPEDITINTERFACE lpInterface);
void TextView_UpdateEditMenuInt(LPEDITINTERFACE lpInterface, HMENU hMenu);
void TextView_UpdateSearchMenuInt(LPEDIINTERFACE lpInterface, HMENU hMenu);

void EditChild_SetLineFeed(HWND hwnd, int nType);
void EditChild_UpdateStatusbarInfo(HWND hwnd);

#endif /* __PCP_EDIT_CHILD_H */
