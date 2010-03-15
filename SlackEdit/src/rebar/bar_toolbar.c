/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : toolbar.c
 * Created    : not known (before 0g/27/00)
 * Owner      : pcppopper
 * Revised on : 07/02/00
 * Comments   : 
 *              
 *              
 *****************************************************************/
 
/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* windows */
#include <commctrl.h>

/* resources */
#include <slack_resource.h>
#include <slack_picdef.h>

/* SlackEdit */
#include "../slack_main.h"
#include "bar_main.h"
#include "bar_toolbar.h"
#include "../settings/settings.h"

/* pcp_generic */
#include <pcp_rebar.h>
#include <pcp_string.h>
#include <pcp_toolbar.h>

/* pcp_controls */
#include <pcp_menu.h>

/* pcp_paint */
#include <pcp_paint.h>

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/
 
/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

TBBUTTON g_tbbDefButtons[TOOLBARENTRIES];
TBBUTTON g_tbbAllButtons[TOOLBARENTRIES];

int g_nDefButtons;
int g_nAllButtons;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

HWND Toolbar_Create(HWND hwndParent)
{
    HIMAGELIST hilCold, hilHot;
    HBITMAP hbm, hbmGray;
    COLORREF crBack;
    TBSAVEPARAMS tbsp;

    HWND hwndToolbar = CreateWindowEx(
                                WS_EX_TOOLWINDOW |WS_EX_CONTROLPARENT,
                                TOOLBARCLASSNAME, "",
                                WS_CHILDWINDOW | WS_VISIBLE |
                                WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
                                CCS_NODIVIDER  | CCS_NOPARENTALIGN |
                                CCS_ADJUSTABLE | CCS_NORESIZE | CCS_TOP |
                                TBSTYLE_FLAT | TBSTYLE_TOOLTIPS |
                                TBSTYLE_TRANSPARENT | TBSTYLE_WRAPABLE,
                                0, 0, 0, 0,
                                hwndParent, (HMENU)IDC_TOOLBAR_MAIN,
                                g_hInstance, NULL);

    Toolbar_SetParent(hwndToolbar, hwndParent);
    Toolbar_ButtonStructSize(hwndToolbar, sizeof(TBBUTTON));
    Toolbar_SetMaxTextRows(hwndToolbar, 1);
    Toolbar_SetBitmapSize(hwndToolbar, 16, 16);
//  SendMessage(hwndToolbar, TB_SETBUTTONSIZE, 0, MAKELONG(22, 22));

    hilCold = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR16, 0, 39);
    hilHot  = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR16, 0, 39);
    
    hbm     = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_TOOLBAR));
    ImageList_AddMasked(hilHot, hbm, RGB(255, 0, 255));

    DeleteObject(hbm);

    //might wanna try a 16bit
    hbm     = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_TOOLBAR));
    crBack  = RGB(255, 0, 255);
    hbmGray = Paint_Bitmap_GrayScale(hbm, 4.5, &crBack);
    ImageList_AddMasked(hilCold, hbmGray, crBack);

    DeleteObject(hbm);
    DeleteObject(hbmGray);

    Toolbar_SetImageList(hwndToolbar, hilCold);
    Toolbar_SetHotImageList(hwndToolbar, hilHot);
    Toolbar_SetExtendedStyle(hwndToolbar, TBSTYLE_EX_DRAWDDARROWS);

    tbsp.hkr            = REG_ROOT;
    tbsp.pszSubKey      = (REG_SLACKEDIT _T("\\GUI Settings"));
    tbsp.pszValueName   = _T("Toolbar Buttons");

    Toolbar_SaveRestore(hwndToolbar, FALSE, &tbsp);

    // First time... (or if the registry got fuced)
    if (Toolbar_ButtonCount(hwndToolbar) == 0)
    {
        Toolbar_AddButtons(hwndToolbar, g_nDefButtons, g_tbbDefButtons);
    }

    return (hwndToolbar);
}

HWND Toolbar_Destroy(HWND hwndToolbar)
{
    TBSAVEPARAMS tbsp;

    tbsp.hkr            = REG_ROOT;
    tbsp.pszSubKey      = (REG_SLACKEDIT _T("\\GUI Settings"));
    tbsp.pszValueName   = _T("Toolbar Buttons");

    Toolbar_SaveRestore(hwndToolbar, TRUE, &tbsp);

    if (DestroyWindow(hwndToolbar))
        return (NULL);
    else
        return (hwndToolbar);
}

BOOL Toolbar_ToolTip(LPTOOLTIPTEXT lpttt)
{
    lpttt->hinst = g_hInstance;

    if ((lpttt->lpszText = MAKEINTRESOURCE(lpttt->hdr.idFrom)) == NULL)
        return (FALSE);
    else
        return (TRUE);
}

void Toolbar_SetButtonText(HWND hwnd, HWND hwndMenu)
{
    TBBUTTONINFO    tbbi;
    TBBUTTON        tbb;
    MENUITEMINFO    mii;
    TCHAR           szText[80];
    LPTSTR          lpTab;
    int             cnt;
    int             iItems;
    
    iItems = Toolbar_ButtonCount(hwnd);

    INITSTRUCT(tbbi, TRUE);
    tbbi.dwMask = TBIF_TEXT;

    mii.cbSize      = sizeof(MENUITEMINFO);
    mii.fMask       = MIIM_TYPE;
    mii.fType       = MFT_STRING;
    mii.dwTypeData  = szText;
    mii.cch         = sizeof(szText);

    for (cnt = 0; cnt < iItems; cnt++)
    {
        Toolbar_GetButton(hwnd, cnt, &tbb);

        if (tbb.fsStyle & TBSTYLE_SEP)
            continue;

        if (GetMenuItemInfo(GetMenu(hwndMenu), tbb.idCommand, FALSE, &mii))
        {   
            if ((lpTab = _tcschr((LPTSTR)mii.dwTypeData, _T('\t'))) != NULL)
            *lpTab = _T('\0');

            
            String_Trim((LPTSTR)szText, _T("&."));

            tbbi.pszText = szText;

            Toolbar_SetButtonInfo(hwnd, tbb.idCommand, &tbbi);
        }
    }
}

BOOL MyToolbar_GetButtonInfo(LPTBNOTIFY lptbn)
{
    MENUITEMINFO    mii;
    LPTSTR          lpTab;


    if (lptbn->iItem < g_nAllButtons)
    {
        lptbn->tbButton.iBitmap   = g_tbbAllButtons[lptbn->iItem].iBitmap;
        lptbn->tbButton.idCommand = g_tbbAllButtons[lptbn->iItem].idCommand;
        lptbn->tbButton.fsState   = g_tbbAllButtons[lptbn->iItem].fsState;
        lptbn->tbButton.fsStyle   = g_tbbAllButtons[lptbn->iItem].fsStyle;
        lptbn->tbButton.dwData    = 0;
        lptbn->tbButton.iString   = 0;

        mii.cbSize      = sizeof(MENUITEMINFO);
        mii.fMask       = MIIM_TYPE;
        mii.fType       = MFT_STRING;
        mii.dwTypeData  = lptbn->pszText;
        mii.cch         = lptbn->cchText;

        if (lptbn->tbButton.fsStyle & TBSTYLE_SEP)
            return (TRUE);

        if (!GetMenuItemInfo(GetMenu(g_hwndMain), g_tbbAllButtons[lptbn->iItem].idCommand, FALSE, &mii))
            return (TRUE);

        if ((lpTab = _tcschr((LPTSTR)mii.dwTypeData, _T('\t'))) != NULL)
            *lpTab = _T('\0');
        
        String_Trim((LPTSTR)mii.dwTypeData, _T("&."));

        return (TRUE);
    }

    return (FALSE);
}

void Toolbar_Reset(HWND hwnd)
{
    int i;

    for (i = (Toolbar_ButtonCount(hwnd) - 1); i >= 0; i--)
        Toolbar_DeleteButton(hwnd, i);

    Toolbar_InitDefButtons();

    Toolbar_AddButtons(hwnd, g_nDefButtons, &g_tbbDefButtons);
}

int Toolbar_GetIcon(UINT uID)
{
    int i;

    for (i = 0; i < g_nAllButtons; i++)
    {
        if (g_tbbAllButtons[i].idCommand == (int)uID)
            return (g_tbbAllButtons[i].iBitmap);
    }

    return (-1);
}

int Toolbar_GetSysMenuIcon(UINT uID)
{
    switch (uID)
    {
    case IDM_SYSMENU_RESTORE:
    return (IDP_RESTORE);
    case IDM_SYSMENU_MINIMIZE:
    return (IDP_MIN);
    case IDM_SYSMENU_MAXIMIZE:
    return (IDP_MAX);
    case IDM_SYSMENU_CLOSE:
    return (IDP_KILL);
    case IDM_SYSMENU_NEXT:
    return (IDP_NEXTWINDOW);
    }

    return (IDP_NONE);
}

int Toolbar_GetOtherIcon(UINT uID)
{
    switch (uID)
    {
    case IDM_TRAY_RESTORE:
    return (IDP_RESTORE);
    case IDM_TRAY_OPEN:
    return (IDP_OPEN);
    case IDM_TRAY_EXIT:
    return (IDP_EXIT);
    }

    return (IDP_NONE);
}

void Toolbar_InitDefButtons(void)
{
    LPTBBUTTON lptbb = g_tbbDefButtons;
    
    g_nDefButtons = 0;

    lptbb->iBitmap      = IDP_NEW;
    lptbb->idCommand    = IDM_FILE_NEW;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;

    lptbb->iBitmap      = IDP_NONE;
    lptbb->idCommand    = 0;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_SEP;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;

    lptbb->iBitmap      = IDP_OPEN;
    lptbb->idCommand    = IDM_FILE_OPEN;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;

    lptbb->iBitmap      = IDP_FAVORITES;
    lptbb->idCommand    = IDM_FILE_FAVORITEOPEN;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;

    lptbb->iBitmap      = IDP_SAVE;
    lptbb->idCommand    = IDM_FILE_SAVE;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;

    lptbb->iBitmap      = IDP_SAVEALL;
    lptbb->idCommand    = IDM_FILE_SAVEALL;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;

    lptbb->iBitmap      = IDP_NONE;
    lptbb->idCommand    = 0;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_SEP;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;

    lptbb->iBitmap      = IDP_UNDO;
    lptbb->idCommand    = IDM_EDIT_UNDO;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;
    
    lptbb->iBitmap      = IDP_REDO;
    lptbb->idCommand    = IDM_EDIT_REDO;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;

    lptbb->iBitmap      = IDP_NONE;
    lptbb->idCommand    = 0;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_SEP;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;
    
    lptbb->iBitmap      = IDP_FIND;
    lptbb->idCommand    = IDM_SEARCH_FIND;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;
    
    lptbb->iBitmap      = IDP_REPLACE;
    lptbb->idCommand    = IDM_SEARCH_REPLACE;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;

    lptbb->iBitmap      = IDP_NONE;
    lptbb->idCommand    = 0;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_SEP;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;

    lptbb->iBitmap      = IDP_EXIT;
    lptbb->idCommand    = IDM_FILE_EXIT;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nDefButtons++;
}

void Toolbar_InitAllButtons(void)
{
    LPTBBUTTON lptbb = g_tbbAllButtons;

    g_nAllButtons = 0;

    lptbb->iBitmap      = IDP_NEW;
    lptbb->idCommand    = IDM_FILE_NEW;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_OPEN;
    lptbb->idCommand    = IDM_FILE_OPEN;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_FAVORITES;
    lptbb->idCommand    = IDM_FILE_FAVORITEOPEN;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_SAVE;
    lptbb->idCommand    = IDM_FILE_SAVE;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_SAVEALL;
    lptbb->idCommand    = IDM_FILE_SAVEALL;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_READONLY;
    lptbb->idCommand    = IDM_FILE_READONLY;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_PRINT;
    lptbb->idCommand    = IDM_FILE_PRINT;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_EXIT;
    lptbb->idCommand    = IDM_FILE_EXIT;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_UNDO;
    lptbb->idCommand    = IDM_EDIT_UNDO;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;
    
    lptbb->iBitmap      = IDP_REDO;
    lptbb->idCommand    = IDM_EDIT_REDO;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_CUT;
    lptbb->idCommand    = IDM_EDIT_CUT;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_COPY;
    lptbb->idCommand    = IDM_EDIT_COPY;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_PASTE;
    lptbb->idCommand    = IDM_EDIT_PASTE;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_DELETE;
    lptbb->idCommand    = IDM_EDIT_DELETE;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_CLEARNUMBOOKMARKS;
    lptbb->idCommand    = IDM_EDIT_NUMBOOKMARKS_CLEARBOOKMARKS;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK1;
    lptbb->idCommand    = IDM_EDIT_NUMBOOKMARKS_BOOKMARK1;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK2;
    lptbb->idCommand    = IDM_EDIT_NUMBOOKMARKS_BOOKMARK2;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK3;
    lptbb->idCommand    = IDM_EDIT_NUMBOOKMARKS_BOOKMARK3;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK4;
    lptbb->idCommand    = IDM_EDIT_NUMBOOKMARKS_BOOKMARK4;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK5;
    lptbb->idCommand    = IDM_EDIT_NUMBOOKMARKS_BOOKMARK5;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK6;
    lptbb->idCommand    = IDM_EDIT_NUMBOOKMARKS_BOOKMARK6;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK7;
    lptbb->idCommand    = IDM_EDIT_NUMBOOKMARKS_BOOKMARK7;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK8;
    lptbb->idCommand    = IDM_EDIT_NUMBOOKMARKS_BOOKMARK8;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK9;
    lptbb->idCommand    = IDM_EDIT_NUMBOOKMARKS_BOOKMARK9;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK0;
    lptbb->idCommand    = IDM_EDIT_NUMBOOKMARKS_BOOKMARK0;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK1;
    lptbb->idCommand    = IDM_EDIT_GOTOBOOKMARKS_BOOKMARK1;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK2;
    lptbb->idCommand    = IDM_EDIT_GOTOBOOKMARKS_BOOKMARK2;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK3;
    lptbb->idCommand    = IDM_EDIT_GOTOBOOKMARKS_BOOKMARK3;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK4;
    lptbb->idCommand    = IDM_EDIT_GOTOBOOKMARKS_BOOKMARK4;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK5;
    lptbb->idCommand    = IDM_EDIT_GOTOBOOKMARKS_BOOKMARK5;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK6;
    lptbb->idCommand    = IDM_EDIT_GOTOBOOKMARKS_BOOKMARK6;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK7;
    lptbb->idCommand    = IDM_EDIT_GOTOBOOKMARKS_BOOKMARK7;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK8;
    lptbb->idCommand    = IDM_EDIT_GOTOBOOKMARKS_BOOKMARK8;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK9;
    lptbb->idCommand    = IDM_EDIT_GOTOBOOKMARKS_BOOKMARK9;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_BOOKMARK0;
    lptbb->idCommand    = IDM_EDIT_GOTOBOOKMARKS_BOOKMARK0;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_TOGGLEBOOKMARK;
    lptbb->idCommand    = IDM_EDIT_BOOKMARKS_TOGGLEBOOKMARK;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_NEXTBOOKMARK;
    lptbb->idCommand    = IDM_EDIT_BOOKMARKS_NEXTBOOKMARK;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_PREVIOUSBOOKMARK;
    lptbb->idCommand    = IDM_EDIT_BOOKMARKS_PREVIOUSBOOKMARK;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_CLEARBOOKMARKS;
    lptbb->idCommand    = IDM_EDIT_BOOKMARKS_CLEARALLBOOKMARKS;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_INDENT;
    lptbb->idCommand    = IDM_EDIT_INDENT;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_UNINDENT;
    lptbb->idCommand    = IDM_EDIT_UNINDENT;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_FIND;
    lptbb->idCommand    = IDM_SEARCH_FIND;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_FINDNEXT;
    lptbb->idCommand    = IDM_SEARCH_FINDNEXT;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_FINDPREVIOUS;
    lptbb->idCommand    = IDM_SEARCH_FINDPREVIOUS;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_REPLACE;
    lptbb->idCommand    = IDM_SEARCH_REPLACE;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_CLIPBOOK;
    lptbb->idCommand    = IDM_VIEW_CHILDREN_CLIPBOOK;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_CHECK;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_OUTPUTWINDOW;
    lptbb->idCommand    = IDM_VIEW_CHILDREN_OUTPUTWINDOW;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_CHECK;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_GOTOLINE;
    lptbb->idCommand    = IDM_VIEW_REBARITEMS_GOTOLINE;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_CHECK;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_CONFIG;
    lptbb->idCommand    = IDM_OPTIONS_SETTINGS;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_CLOSEWINDOW;
    lptbb->idCommand    = IDM_WINDOW_CLOSE;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_CLOSEALLWINDOWS;
    lptbb->idCommand    = IDM_WINDOW_CLOSEALL;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_NEXTWINDOW;
    lptbb->idCommand    = IDM_WINDOW_NEXT;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;
    
    lptbb->iBitmap      = IDP_PREVIOUSWINDOW;
    lptbb->idCommand    = IDM_WINDOW_PREVIOUS;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_CASCADE;
    lptbb->idCommand    = IDM_WINDOW_CASCADE;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_TILEHORIZONTAL;
    lptbb->idCommand    = IDM_WINDOW_TILEHORIZONTAL;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_TILEVERTICAL;
    lptbb->idCommand    = IDM_WINDOW_TILEVERTICAL;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_WINDOWLIST;
    lptbb->idCommand    = IDM_WINDOW_WINDOWLIST;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_TIPOFTHEDAY;
    lptbb->idCommand    = IDM_HELP_TIPOFDADAY;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;

    lptbb->iBitmap      = IDP_ABOUT;
    lptbb->idCommand    = IDM_HELP_ABOUT;
    lptbb->fsState      = TBSTATE_ENABLED;
    lptbb->fsStyle      = TBSTYLE_BUTTON;
    lptbb->dwData       = 0;
    lptbb->iString      = 0;
    lptbb++;
    g_nAllButtons++;
}
