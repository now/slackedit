/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_mdi_child_generic.h
 * Created    : 03/09/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:15:18
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __SE_MDI_CHILD_GENERIC_H
#define __SE_MDI_CHILD_GENERIC_H

BOOL MDI_Child_Generic_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void MDI_Child_Generic_OnSize(HWND hwnd, UINT state, int cx, int cy);
void MDI_Child_Generic_OnMDIActivate(HWND hwnd, BOOL fActive, HWND hwndActivate, HWND hwndDeactivate);
void MDI_Child_Generic_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT uNotifyCode);
void MDI_Child_Generic_OnClose(HWND hwnd);
BOOL MDI_Child_Generic_OnQueryEndSession(HWND hwnd);
LRESULT MDI_Child_Generic_OnNotify(HWND hwnd, int id, LPNMHDR pnmh);
void MDI_Child_Generic_OnMenuSelect(HWND hwnd, HMENU hMenu, int item, HMENU hMenuPopup, UINT flags);
void MDI_Child_Generic_OnTimer(HWND hwnd, UINT id);
BOOL MDI_Child_Generic_OnPreClose(HWND hwnd);
void MDI_Child_Generic_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);

void MDI_Child_Generic_EnableToolbarButtons(HWND hwnd);

int MDI_Child_Generic_GetFileType(HWND hwnd, BOOL bCareIfEdit);

#endif /* __SE_MDI_CHILD_GENERIC_H */
