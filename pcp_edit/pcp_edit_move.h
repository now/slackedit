/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_move.h
 * Created    : not known (before 0124/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:42:20
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_EDIT_MOVE_H
#define __PCP_EDIT_MOVE_H

#include "pcp_edit.h"

BOOL Edit_View_GetFromClipboard(LPEDITVIEW lpev, LPTSTR *pszText);
BOOL Edit_View_PutToClipboard(LPEDITVIEW lpev, LPCTSTR pszText);
BOOL Edit_View_TextInClipboard(LPEDITVIEW lpev);
void Edit_View_OnRButtonDown(LPEDITVIEW lpev, UINT nFlags, POINT point);
void Edit_View_OnLButtonDblClk(LPEDITVIEW lpev, UINT nFlags, POINT point);
void Edit_View_OnTimer(LPEDITVIEW lpev, UINT nIDEvent);
void Edit_View_OnLButtonUp(LPEDITVIEW lpev, UINT nFlags, POINT point);
void Edit_View_OnMouseMove(LPEDITVIEW lpev, UINT nFlags, POINT point);
void Edit_View_OnLButtonDown(LPEDITVIEW lpev, UINT nFlags, POINT point);
void Edit_View_SelectAll(LPEDITVIEW lpev);
POINT Edit_Move_WordToLeft(LPEDITVIEW lpev, POINT pt);
POINT Edit_Move_WordToRight(LPEDITVIEW lpev, POINT pt);
void Edit_Move_ScrollRight(LPEDITVIEW lpev);
void Edit_Move_ScrollLeft(LPEDITVIEW lpev);
void Edit_Move_ScrollDown(LPEDITVIEW lpev);
void Edit_Move_ScrollUp(LPEDITVIEW lpev);
void Edit_Move_MoveCtrlEnd(LPEDITVIEW lpev, BOOL bSelect);
void Edit_Move_MoveCtrlHome(LPEDITVIEW lpev, BOOL bSelect);
void Edit_Move_MovePgDn(LPEDITVIEW lpev, BOOL bSelect);
void Edit_Move_MovePgUp(LPEDITVIEW lpev, BOOL bSelect);
void Edit_Move_MoveEnd(LPEDITVIEW lpev, BOOL bSelect);
void Edit_Move_MoveHome(LPEDITVIEW lpev, BOOL bSelect);
void Edit_Move_MoveDown(LPEDITVIEW lpev, BOOL bSelect);
void Edit_Move_MoveUp(LPEDITVIEW lpev, BOOL bSelect);
void Edit_Move_MoveWordRight(LPEDITVIEW lpev, BOOL bSelect);
void Edit_Move_MoveWordLeft(LPEDITVIEW lpev, BOOL bSelect);
void Edit_Move_MoveRight(LPEDITVIEW lpev, BOOL bSelect);
void Edit_Move_MoveLeft(LPEDITVIEW lpev, BOOL bSelect);

#endif /* __PCP_EDIT_MOVE_H */
