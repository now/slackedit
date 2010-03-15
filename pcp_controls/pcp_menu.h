/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_menu.h
 * Created    : not known (before 06/26/00)
 * Owner      : pcppopper
 * Revised on : 07/02/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __MENU_H
#define __MENU_H

/* windows */
#include <commctrl.h>

/* pcp_edit */
#include "pcp_controls.h"

#define IDM_SYSMENU_SIZE                61440
#define IDM_SYSMENU_MOVE                61456
#define IDM_SYSMENU_MINIMIZE            61472
#define IDM_SYSMENU_MAXIMIZE            61488
#define IDM_SYSMENU_NEXT                61504
#define IDM_SYSMENU_CLOSE               61536
#define IDM_SYSMENU_RESTORE             61728

#define ODMFT_NOTEXT			0x00000001
#define ODMFT_SEPARATOR			0x00000002
#define ODMFT_MENUBAR			0x00000004
#define ODMFT_DONTEXPANDTABS	0x00000008
#define ODMFT_RADIOGROUPITEM	0x00000010

/*
 * These define standard icons (either it's to be looked up (IDP_TOOLBAR)
 * or it is an empty icon (IDP_SEP or IDP_NONE
 */
#define IDP_TOOLBAR				-2
#define IDP_SEPARATOR			-1
#define IDP_NONE				-1

typedef struct tagMENUITEMEX
{
	UINT		fType;
	UINT		fFlags;
	INT			iImageIndex;
	LPTSTR		pszText;
	WORD		wID;
} MENUITEMEX, FAR *LPMENUITEMEX;

typedef int (*ToolbarFunc)(UINT);

#define Menu_EnableMenuItem(hMenu, wIDEnableItem, fEnable) \
    EnableMenuItem((hMenu), (wIDEnableItem), (fEnable) ? MF_ENABLED : MF_GRAYED)

#define Menu_CheckMenuItem(hMenu, wIDCheckItem, fCheck) \
    CheckMenuItem((hMenu), (wIDCheckItem), (fCheck) ? MF_CHECKED : MF_UNCHECKED)

FOREXPORT void Menu_Install(ToolbarFunc ProcessToolbarFunc, ToolbarFunc ProcessSysMenuFunc, ToolbarFunc ProcessOtherFunc);
FOREXPORT void Menu_Uninstall(void);
FOREXPORT int Menubar_Create(HWND hwnd, HMENU hMenu);
FOREXPORT void Menu_AddImage(HBITMAP hBitmap, COLORREF crTrans);
FOREXPORT BOOL Menu_TrackPopupMenu(HMENU hMenu, int x, int y, HWND hwndOwner);
FOREXPORT BOOL Menu_TrackCreatedPopupMenu(HMENU hMenu, int x, int y, HWND hwndOwner);
FOREXPORT void Menu_DestroyMenu(HMENU hMenu);

FOREXPORT void Menu_InsertMenuItem(HMENU hMenu, UINT uID, BOOL fByPosition, LPMENUITEMEX lpmix);
FOREXPORT void Menu_SetItemImage(HMENU hMenu, UINT uItem, BOOL fByPosition, int iImage);
FOREXPORT void Menu_SetItemText(HMENU hMenu, UINT uItem, BOOL fByPosition, LPCTSTR pszText);
FOREXPORT int Menu_CreateMenu(HMENU hSubMenu, int *aImages);
FOREXPORT void Menu_SetItemFlags(HMENU hMenu, UINT uItem, BOOL fByPosition, UINT fFlags);

FOREXPORT HIMAGELIST Menu_GetImageList(BOOL bHot);

#endif /* __MENU_H */
