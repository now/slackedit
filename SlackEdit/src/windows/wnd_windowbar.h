/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_windowbar.h
 * Created    : not known (before 06/26/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:19:02
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __SE_WINDOWBAR_H
#define __SE_WINDOWBAR_H

HWND Windowbar_Create(HWND hwndParent);
HWND Windowbar_Destroy(HWND hwndTab);

BOOL Windowbar_ToolTip(LPTOOLTIPTEXT lpttt);

void WindowBar_Update(LPFILEVIEW lpFileView, UINT nAction);

#endif /* __SE_WINDOWBAR_H */
