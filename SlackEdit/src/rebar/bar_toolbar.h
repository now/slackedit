/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : toolbar.h
 * Created    : not known (before 06/26/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:13:56
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __TOOLBAR_H
#define __TOOLBAR_H

#define TOOLBARHEIGHT				22
#define TOOLBARENTRIES				59

void Toolbar_InitDefButtons(void);
void Toolbar_InitAllButtons(void);

HWND Toolbar_Create(HWND hwndParent);
BOOL Toolbar_ToolTip(LPTOOLTIPTEXT lpttt);
void Toolbar_SetButtonText(HWND hwnd, HWND hwndMenu);
BOOL MyToolbar_GetButtonInfo(LPTBNOTIFY lptbn);
void Toolbar_Reset(HWND hwnd);
int Toolbar_GetIcon(UINT uID);
int Toolbar_GetSysMenuIcon(UINT uID);
int Toolbar_GetOtherIcon(UINT uID);
HWND Toolbar_Destroy(HWND hwndToolbar);

extern TBBUTTON g_tbbDefButtons[TOOLBARENTRIES];
extern TBBUTTON g_tbbAllButtons[TOOLBARENTRIES];

extern int g_nDefButtons;
extern int g_nAllButtons;

#endif /* __TOOLBAR_H */
