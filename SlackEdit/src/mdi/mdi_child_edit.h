/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_mdi_child_edit.h
 * Created    : not known (before 03/09/00)
 * Owner      : pcppopper
 * Revised on : 07/16/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __SE_MDI_CHILD_EDIT_H
#define __SE_MDI_CHILD_EDIT_H

LRESULT CALLBACK MDI_Child_Edit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void MDI_Child_Edit_EnableToolbarButtons(HWND hwnd, BOOL bLast);
void MDI_Child_Edit_UpdateOverWriteMode(HWND hwnd);

#endif /* __SE_MDI_CHILD_EDIT_H */
