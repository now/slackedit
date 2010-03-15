/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : main.h
 * Created    : not known
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:04:02
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __MAIN_H
#define __MAIN_H

#define IDC_MDICLIENT_MAIN		1999
#define IDC_EDIT_MDICHILD		2000
#define IDC_STATUSBAR_MAIN		2001
#define IDC_TAB_WINDOWBAR		2002
#define IDC_TOOLBAR_MAIN		2003
#define IDC_REBAR_MAIN			2004
#define IDC_COMBO_QUICKSEARCH	2005
#define IDC_TRAYICON_MAIN		2006
#define IDC_CLIPBOOK			2007
#define IDC_OUTPUTWINDOW		2008
#define IDC_DLG_GOTOLINE		2009

#define IDM_RECENTFILES			42000
#define IDM_MDI_FIRSTCHILD		60000
#define IDM_EDIT_PASTERECENT	38999

#define IDM_MAIN_FILE			0
#define IDM_MAIN_EDIT			1
#define IDM_MAIN_SEARCH			2
#define IDM_MAIN_VIEW			3
#define IDM_MAIN_OPTIONS		4
#define IDM_MAIN_WINDOW			5
#define IDM_MAIN_HELP			6

#define IDT_AUTOSAVE			0x00000001

#define TCM_SETTEXT				2001

#define MDISTATE_NORMAL		0
#define MDISTATE_TILEHORZ	1
#define MDISTATE_TILEVERT	2
#define MDISTATE_MAXIMIZED	3

#define WM_MYFONTCHANGE		(WM_USER + 346)

/* void Cls_OnMYFontChange(HWND hwnd, UINT idCtl, UINT codeNotify) */
#define HANDLE_WM_MYFONTCHANGE(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (LPLOGFONT)(lParam)), 0L)
#define FORWARD_WM_MYFONTCHANGE(hwnd, lplf, fn) \
	(void)(fn)((hwnd), WM_MYFONTCHANGE, (WPARAM)0, (LPARAM)(LPLOGFONT)(lplf))

HINSTANCE g_hInstance;
HWND g_hwndMain, g_hwndMDIClient, g_hwndToolbarMain, g_hwndTabWindowBar, 
	g_hwndRebarMain, g_hwndCboQuickSearch, g_hwndClipbook, g_hwndOutputWindow;
HWND g_hwndDlgGotoline;
HMENU g_hMenuRecentFiles;
HMENU g_hMainMenu;

extern const long magicDWord;
extern LPCTSTR g_pszAppName;
extern LPCTSTR g_pszEditChildClass;
extern LPCTSTR g_pszHexChildClass;

HMENU Main_GetMainSubMenu(HWND hwnd, int nPos);
void Main_ToggleChildWindow(UINT uID, BOOL bResize);

#endif /* __MAIN_H */
