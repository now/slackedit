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

BOOL TextEdit_View_GetFromClipboard(LPTEXTEDITVIEW lpte, LPTSTR *pszText);
BOOL TextEdit_View_PutToClipboard(LPTEXTEDITVIEW lpte, LPCTSTR pszText);
BOOL TextEdit_View_TextInClipboard(LPTEXTEDITVIEW lpte);
void TextEdit_View_OnRButtonDown(LPTEXTEDITVIEW lpte, UINT nFlags, POINT point);
void TextEdit_View_OnLButtonDblClk(LPTEXTEDITVIEW lpte, UINT nFlags, POINT point);
void TextEdit_View_OnTimer(LPTEXTEDITVIEW lpte, UINT nIDEvent);
void TextEdit_View_OnLButtonUp(LPTEXTEDITVIEW lpte, UINT nFlags, POINT point);
void TextEdit_View_OnMouseMove(LPTEXTEDITVIEW lpte, UINT nFlags, POINT point);
void TextEdit_View_OnLButtonDown(LPTEXTEDITVIEW lpte, UINT nFlags, POINT point);
void TextEdit_View_SelectAll(LPTEXTEDITVIEW lpte);
POINT TextEdit_Move_WordToLeft(LPTEXTEDITVIEW lpte, POINT pt);
POINT TextEdit_Move_WordToRight(LPTEXTEDITVIEW lpte, POINT pt);
void TextEdit_Move_ScrollRight(LPTEXTEDITVIEW lpte);
void TextEdit_Move_ScrollLeft(LPTEXTEDITVIEW lpte);
void TextEdit_Move_ScrollDown(LPTEXTEDITVIEW lpte);
void TextEdit_Move_ScrollUp(LPTEXTEDITVIEW lpte);
void TextEdit_Move_MoveCtrlEnd(LPTEXTEDITVIEW lpte, BOOL bSelect);
void TextEdit_Move_MoveCtrlHome(LPTEXTEDITVIEW lpte, BOOL bSelect);
void TextEdit_Move_MovePgDn(LPTEXTEDITVIEW lpte, BOOL bSelect);
void TextEdit_Move_MovePgUp(LPTEXTEDITVIEW lpte, BOOL bSelect);
void TextEdit_Move_MoveEnd(LPTEXTEDITVIEW lpte, BOOL bSelect);
void TextEdit_Move_MoveHome(LPTEXTEDITVIEW lpte, BOOL bSelect);
void TextEdit_Move_MoveDown(LPTEXTEDITVIEW lpte, BOOL bSelect);
void TextEdit_Move_MoveUp(LPTEXTEDITVIEW lpte, BOOL bSelect);
void TextEdit_Move_MoveWordRight(LPTEXTEDITVIEW lpte, BOOL bSelect);
void TextEdit_Move_MoveWordLeft(LPTEXTEDITVIEW lpte, BOOL bSelect);
void TextEdit_Move_MoveRight(LPTEXTEDITVIEW lpte, BOOL bSelect);
void TextEdit_Move_MoveLeft(LPTEXTEDITVIEW lpte, BOOL bSelect);

#endif /* __PCP_EDIT_MOVE_H */
