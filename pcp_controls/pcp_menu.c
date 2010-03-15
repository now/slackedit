/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_menu.c
 * Created    : not known (before 06/26/99)
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

/* pcp_controls */
#include "pcp_menu.h"

/* pcp_generic */
#include <pcp_mem.h>
#include <pcp_rect.h>
#include <pcp_string.h>

/* pcp_paint */
#include <pcp_paint.h>

#define CX_IMAGE        16
#define CY_IMAGE        16
#define CX_EXTRA        4
#define CY_EXTRA        2

typedef struct tagMENUWINDOW
{
    HWND    hwnd;
    BOOL    bMenubar;
    HMENU   *ahMenu;
    INT     nMenuCount;
    RECT    rc;
    WNDPROC lpfnOldWndProc;
} MENUWINDOW, *LPMENUWINDOW;

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

LRESULT CALLBACK Menu_CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK Menu_HookWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL Menu_InitMenuItem(HMENU hMenu, UINT uItem, BOOL fByPosition, int iImage, UINT fFlags);

static void Menu_AddMenuToWindow(LPMENUWINDOW lpmw, HMENU hMenu);
static void Menu_RemoveMenusFromWindow(LPMENUWINDOW lpmw);

static void Menu_MeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpMeasureItem);
static void Menu_DrawItem(HWND hwnd, LPDRAWITEMSTRUCT lpDrawItem);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

static  HIMAGELIST  hImageListHot;
static  HIMAGELIST  hImageListCold;
static  HHOOK       g_hMenuHook;
        LPTSTR      g_pszMenuSubclassProp = _T("MenuSubclassProp2");
        ATOM        g_hMenuAtom;

ToolbarFunc Menu_GetIconFromProcess = NULL;
ToolbarFunc Menu_GetSysMenuIconFromProcess = NULL;
ToolbarFunc Menu_GetOtherIconFromProcess = NULL;
/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

void Menu_Install(ToolbarFunc ProcessToolbarFunc, ToolbarFunc ProcessSysMenuFunc, ToolbarFunc ProcessOtherFunc)
{
    hImageListHot   = ImageList_Create(CX_IMAGE, CY_IMAGE, ILC_MASK | ILC_COLOR16, 0, 100);
    hImageListCold  = ImageList_Create(CX_IMAGE, CY_IMAGE, ILC_MASK | ILC_COLOR16, 0, 100);

    Menu_GetIconFromProcess = ProcessToolbarFunc;
    Menu_GetSysMenuIconFromProcess = ProcessSysMenuFunc;
    Menu_GetOtherIconFromProcess = ProcessOtherFunc;

    g_hMenuAtom = GlobalAddAtom(g_pszMenuSubclassProp);
    /* Set up the whole menu hook (i.e. all should be ownerdrawn) */
    g_hMenuHook =
            SetWindowsHookEx(WH_CALLWNDPROC, Menu_CallWndProc, NULL,
                    GetCurrentThreadId());
}

void Menu_AddImage(HBITMAP hBitmap, COLORREF crTrans)
{
    HBITMAP hbm, hbmGray;
    COLORREF crBack;

    hbm     = Paint_Bitmap_CopyBitmap(hBitmap);
    ImageList_AddMasked(hImageListHot, hbm, crTrans);
    DeleteObject(hbm);

    crBack  = crTrans;
    
    hbm     = Paint_Bitmap_CopyBitmap(hBitmap);
    hbmGray = Paint_Bitmap_GrayScale(hbm, 4.5, &crBack);

    ImageList_AddMasked(hImageListCold, hbmGray, crBack);

    /* Clear up the mess =\ */
    DeleteObject(hbm);
    DeleteObject(hbmGray);
}

void Menu_Uninstall(void)
{
    ImageList_Destroy(hImageListCold);
    ImageList_Destroy(hImageListHot);
    UnhookWindowsHookEx(g_hMenuHook);
    GlobalDeleteAtom(g_hMenuAtom);
}

HIMAGELIST Menu_GetImageList(BOOL bHot)
{
    return ((bHot) ? hImageListHot : hImageListCold);
}

int Menu_CreateMenu(HMENU hSubMenu, int *aImages)
{
    int i;
    int nMenuCount = GetMenuItemCount(hSubMenu);

    for (i = 0; i < nMenuCount; i++)
    {
        int nIndex;

        if ((int)aImages == IDP_TOOLBAR)
            nIndex = IDP_TOOLBAR;
        else if ((int)aImages == IDP_NONE)
            nIndex = IDP_NONE;
        else
            nIndex = aImages[i];

        if (!Menu_InitMenuItem(hSubMenu, i, TRUE, nIndex, 0))
        {
            MENUITEMINFO mii;

            INITSTRUCT(mii, TRUE);
            mii.fMask = MIIM_TYPE | MIIM_SUBMENU;

            GetMenuItemInfo(hSubMenu, i, TRUE, &mii);

            if (mii.hSubMenu != NULL)
            {
                Menu_CreateMenu(mii.hSubMenu, (int *)IDP_TOOLBAR);
            }
        }
    }

    return (nMenuCount);
}

BOOL Menu_InitMenuItem(HMENU hMenu, UINT uItem, BOOL fByPosition, int iImage, UINT fFlags)
{
    MENUITEMINFO mii;
    TCHAR szMenuText[MAX_PATH] = _T("");
    LPMENUITEMEX lpmix;
    BOOL bSet;

    INITSTRUCT(mii, TRUE);
    mii.fMask       = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA | MIIM_ID;
    mii.dwTypeData  = szMenuText;
    mii.cch         = MAX_PATH;

    GetMenuItemInfo(hMenu, uItem, fByPosition, &mii);

    if ((LPMENUITEMEX)mii.dwItemData != (LPMENUITEMEX)NULL)
        return (TRUE);
    //  Mem_Free((LPMENUITEMEX)mii.dwItemData);

    lpmix = (LPMENUITEMEX)Mem_Alloc(sizeof(MENUITEMEX));

    if ((mii.fType & MFT_STRING) == MFT_STRING)
    {
        lpmix->pszText  = String_Duplicate(szMenuText);
    }
    else
    {
        lpmix->fFlags   |= ODMFT_NOTEXT;
        lpmix->pszText  = (LPTSTR)mii.dwTypeData;
    }

    lpmix->fFlags   |= ((mii.fType & MFT_SEPARATOR) ? ODMFT_SEPARATOR : 0);
    lpmix->fFlags   |= ((mii.fType & MFT_RADIOCHECK) ? ODMFT_RADIOGROUPITEM : 0);
    lpmix->fType    = mii.fType;
    lpmix->wID      = mii.wID;

    if (fFlags != 0)
        lpmix->fFlags |= fFlags;

    if (iImage == IDP_TOOLBAR)
    {
        BOOL bSet = FALSE;

        if (Menu_GetIconFromProcess != NULL)
            bSet = ((lpmix->iImageIndex = Menu_GetIconFromProcess(mii.wID)) != IDP_NONE);

        if (!bSet && Menu_GetSysMenuIconFromProcess != NULL)
            bSet = ((lpmix->iImageIndex = Menu_GetSysMenuIconFromProcess(mii.wID)) != IDP_NONE);

        if (!bSet && Menu_GetOtherIconFromProcess != NULL)
            bSet = ((lpmix->iImageIndex = Menu_GetOtherIconFromProcess(mii.wID)) != IDP_NONE);

        // Unnecessary but adds clearity
        if (!bSet)
            lpmix->iImageIndex = IDP_NONE;
    }
    else
    {
        lpmix->iImageIndex  = iImage;
    }

    mii.fMask       = MIIM_TYPE | MIIM_DATA;
    mii.fType       = MFT_OWNERDRAW | lpmix->fType;
    mii.fType       &= ~(MFT_STRING | MFT_BITMAP);
    mii.dwTypeData  = (LPTSTR)NULL;
    mii.dwItemData  = (DWORD)lpmix;

    bSet = SetMenuItemInfo(hMenu, uItem, fByPosition, &mii);

    if (mii.hSubMenu != NULL)
        return (FALSE);
    else
        return (bSet);
}

void Menu_DestroyMenu(HMENU hSubMenu)
{
    int j = 0;
    int cCount = GetMenuItemCount(hSubMenu);
    MENUITEMINFO miiGet, miiSet;

    for (j = 0; j < cCount; j++)
    {
        INITSTRUCT(miiGet, TRUE);
        miiGet.fMask = MIIM_SUBMENU | MIIM_DATA | MIIM_TYPE;

        GetMenuItemInfo(hSubMenu, j, TRUE, &miiGet);

        if (miiGet.hSubMenu != NULL)
            Menu_DestroyMenu(miiGet.hSubMenu);

        if ((miiGet.fType & MFT_OWNERDRAW) != MFT_OWNERDRAW)
            continue;

        if ((LPMENUITEMEX)miiGet.dwItemData != NULL)
        {
            INITSTRUCT(miiSet, TRUE);
            miiSet.fMask        = MIIM_TYPE | MIIM_DATA;
            miiSet.fType        = ((LPMENUITEMEX)miiGet.dwItemData)->fType;
            miiSet.dwTypeData   = ((LPMENUITEMEX)miiGet.dwItemData)->pszText;
            miiSet.dwItemData   = (DWORD)NULL;

            SetMenuItemInfo(hSubMenu, j, TRUE, &miiSet);

            if (((LPMENUITEMEX)miiGet.dwItemData)->pszText != NULL)
                Mem_Free(((LPMENUITEMEX)miiGet.dwItemData)->pszText);

            Mem_Free((LPMENUITEMEX)miiGet.dwItemData);
        }
    }
}

void Menu_InsertMenuItem(HMENU hMenu, UINT uID, BOOL fByPosition, LPMENUITEMEX lpmix)
{
    MENUITEMINFO mii;
    LPMENUITEMEX lpmixSet = (LPMENUITEMEX)Mem_Alloc(sizeof(MENUITEMEX));

    *lpmixSet = *lpmix;
    lpmixSet->pszText = String_Duplicate(lpmix->pszText);

    INITSTRUCT(mii, TRUE);
    mii.fMask       = MIIM_TYPE | MIIM_DATA | MIIM_ID;
    mii.fType       = MFT_OWNERDRAW; // | lpmix->fType;
    mii.dwItemData  = (DWORD)lpmixSet;
    mii.wID         = lpmix->wID;

    if (lpmix->iImageIndex == IDP_TOOLBAR)
        if (Menu_GetIconFromProcess != NULL)
            Menu_GetIconFromProcess(lpmix->wID);

    InsertMenuItem(hMenu, uID, fByPosition, &mii);
}

void Menu_SetItemImage(HMENU hMenu, UINT uItem, BOOL fByPosition, int iImage)
{
    MENUITEMINFO mii;

    INITSTRUCT(mii, TRUE);
    mii.fMask   = MIIM_DATA;

    GetMenuItemInfo(hMenu, uItem, fByPosition, &mii);

    if ((LPMENUITEMEX)mii.dwItemData != NULL)
    {
        ((LPMENUITEMEX)mii.dwItemData)->iImageIndex = iImage;
    }
}

void Menu_SetItemText(HMENU hMenu, UINT uItem, BOOL fByPosition, LPCTSTR pszText)
{
    MENUITEMINFO mii;

    INITSTRUCT(mii, TRUE);
    mii.fMask   = MIIM_DATA;

    GetMenuItemInfo(hMenu, uItem, fByPosition, &mii);

    if ((LPMENUITEMEX)mii.dwItemData != NULL)
    {
        LPMENUITEMEX lpmix = (LPMENUITEMEX)mii.dwItemData;

        Mem_Free(lpmix->pszText);
        lpmix->pszText = Mem_AllocStr(_tcslen(pszText));
        _tcscpy(lpmix->pszText, pszText);
    }
}

void Menu_SetItemFlags(HMENU hMenu, UINT uItem, BOOL fByPosition, UINT fFlags)
{
    MENUITEMINFO mii;

    INITSTRUCT(mii, TRUE);
    mii.fMask   = MIIM_DATA;

    GetMenuItemInfo(hMenu, uItem, fByPosition, &mii);

    if ((LPMENUITEMEX)mii.dwItemData != NULL)
    {
        LPMENUITEMEX lpmix = (LPMENUITEMEX)mii.dwItemData;

        lpmix->fFlags = fFlags;
    }
}

void Menu_MeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpMeasureItem)
{
    LPMENUITEMEX lpmix = (LPMENUITEMEX)lpMeasureItem->itemData;

    if (lpmix->fFlags & ODMFT_SEPARATOR)
    {
        lpMeasureItem->itemWidth    = 0;
        lpMeasureItem->itemHeight   = GetSystemMetrics(SM_CYMENU) >> 1;
    }
    else
    {
        HDC hdc;
        HFONT hFont;
        HFONT hOldFont;
        NONCLIENTMETRICS nm;
        SIZE size;
        RECT rcText;

        INITSTRUCT(nm, TRUE);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0);
        hFont = CreateFontIndirect(&nm.lfMenuFont);

        hdc = GetDC(hwnd);
        hOldFont = SelectObject(hdc, hFont);

        // To trim down length of strings containing tabs (that get too long)
/*      if (lpmix->fFlags & ODMFT_DONTEXPANDTABS)
            String_ReplaceChar(pszText, _T('\t'), _T(' '));
*/
//      VERIFY(GetTextExtentPoint32(hdc, lpmix->pszText, _tcslen(lpmix->pszText), &size));
        VERIFY(DrawText(hdc, lpmix->pszText, _tcslen(lpmix->pszText), &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT));
        size.cx = Rect_Width(&rcText);
        
        DeleteObject(SelectObject(hdc, hOldFont));
        ReleaseDC(hwnd, hdc);

        if (lpmix->fFlags & ODMFT_MENUBAR)
        {
            lpMeasureItem->itemHeight = GetSystemMetrics(SM_CYMENU);
            lpMeasureItem->itemWidth = size.cx - ((size.cx / _tcslen(lpmix->pszText)) * 1);
        }
        else
        {
            lpMeasureItem->itemHeight = max(CY_IMAGE + (CY_EXTRA * 2),
                                    GetSystemMetrics(SM_CYMENU));
            lpMeasureItem->itemWidth = CX_IMAGE + (CX_EXTRA * 3) + size.cx;
        }
    }
}

void Menu_DrawItem(HWND hwnd, LPDRAWITEMSTRUCT lpDrawItem)
{
    RECT        rcMenu, rcIcon;
    LPMENUITEMEX lpmix = (LPMENUITEMEX)lpDrawItem->itemData;
    COLORREF    crBtnHilite     = GetSysColor(COLOR_BTNHILIGHT);
    COLORREF    crMenuText      = GetSysColor(COLOR_MENUTEXT);
    COLORREF    crGrayText      = GetSysColor(COLOR_GRAYTEXT);
    COLORREF    crHiliteText    = GetSysColor(COLOR_HIGHLIGHTTEXT);
    HBRUSH      hbBackground    = GetSysColorBrush(COLOR_MENU);
    HBRUSH      hbBtnHilite     = GetSysColorBrush(COLOR_BTNHILIGHT);
    HBRUSH      hbBtnLite       = GetSysColorBrush(COLOR_3DLIGHT);
    BOOL        bSelected       = (lpDrawItem->itemState & ODS_SELECTED);
    BOOL        bDisabled       = (lpDrawItem->itemState & ODS_GRAYED);
    BOOL        bChecked        = (lpDrawItem->itemState & ODS_CHECKED);
    BOOL        bRadioCheck     = (lpmix->fFlags & ODMFT_RADIOGROUPITEM);
    HDC         hdcItem         = lpDrawItem->hDC;
    COLORREF    crTextOld;
    int bkOld;
    DWORD dwLeftDTFlags = DT_VCENTER | DT_SINGLELINE | DT_LEFT;
    DWORD dwRightDTFlags = DT_VCENTER | DT_SINGLELINE | DT_RIGHT;
    DWORD dwCenterDTFlags = DT_VCENTER | DT_SINGLELINE | DT_CENTER;

    if (lpDrawItem->itemAction & ODA_FOCUS)
    {
        return;
    }

    if (lpmix->fFlags & ODMFT_SEPARATOR)
    {
        CopyRect(&rcMenu, &lpDrawItem->rcItem);
        rcMenu.top += Rect_Height(&rcMenu) >> 1;
        DrawEdge(hdcItem, &rcMenu, EDGE_ETCHED, BF_TOP);

        return;
    }

    bkOld = SetBkMode(hdcItem, TRANSPARENT);

    CopyRect(&rcMenu, &lpDrawItem->rcItem);
    CopyRect(&rcIcon, &lpDrawItem->rcItem);
    rcIcon.right    = CX_IMAGE + CX_EXTRA;
    InflateRect(&rcIcon, -1, -1);

    if (bDisabled)
    {
        int cx, cy;
        HDC hdcMem;
        HBITMAP hBitmap;
        HBITMAP hOldBitmap;

        crTextOld = SetTextColor(hdcItem, crMenuText);

        FillRect(hdcItem, &rcMenu, hbBackground);

        ImageList_GetIconSize(hImageListHot, &cx, &cy);
        
        hdcMem = CreateCompatibleDC(hdcItem);
        hBitmap = CreateCompatibleBitmap(hdcItem, cx, cy);
        hOldBitmap = SelectObject(hdcMem, hBitmap);

        PatBlt(hdcMem, 0, 0, cx, cy, WHITENESS);
        ImageList_Draw(hImageListHot, lpmix->iImageIndex, hdcMem, 0, 0, ((GetSysColor(COLOR_BTNTEXT) == RGB(255, 255, 255)) ? ILD_MASK : ILD_NORMAL));
        Paint_DitherBlt(hdcItem, rcIcon.left + 1, rcIcon.top + 1, cx, cy, hdcMem, 0, 0);

        DeleteObject(SelectObject(hdcMem, hOldBitmap));
        DeleteDC(hdcMem);
    }
    else
    {
        crTextOld   = SetTextColor(hdcItem, crHiliteText);

        FillRect(hdcItem, &rcMenu, hbBtnLite);

        if (bSelected)
        {
            DrawEdge(hdcItem, &rcMenu, BDR_SUNKENOUTER, BF_RECT);

            if (bChecked || lpmix->iImageIndex != IDP_NONE)
                DrawEdge(hdcItem, &rcIcon, BDR_RAISEDINNER, BF_RECT);
        }
        else if (bChecked)
        {
            DrawEdge(hdcItem, &rcIcon, BDR_SUNKENOUTER, BF_RECT);
        }

        if (bChecked && lpmix->iImageIndex == IDP_NONE)
        {
            if (bRadioCheck)
                Paint_DrawRadioDot(hdcItem, &rcIcon, crMenuText);
            else
                Paint_DrawCheckMark(hdcItem, &rcIcon, crMenuText);
        }

        if (lpmix->iImageIndex != IDP_NONE)
        {
            ImageList_Draw((bSelected || bChecked) ? hImageListHot : hImageListCold,
                    lpmix->iImageIndex, hdcItem, rcIcon.left + 1, rcIcon.top + 1, ILD_TRANSPARENT);
        }
    }

    if (!(lpmix->fFlags & ODMFT_MENUBAR))
    {
        rcMenu.left += CX_IMAGE + CX_EXTRA + 2;
        rcMenu.right -= CX_EXTRA * 3;
        rcMenu.top -= 1;
    }

    if (!(lpmix->fFlags & ODMFT_NOTEXT) && lpmix->pszText != (LPTSTR)NULL)
    {
        LPTSTR pszTab = NULL;
        int nTextLen;
        BOOL bTabbed = FALSE;

        pszTab = _tcschr(lpmix->pszText, _T('\t'));
        nTextLen = _tcslen(lpmix->pszText);

        if (pszTab != NULL && !(lpmix->fFlags & ODMFT_DONTEXPANDTABS))
        {
            bTabbed = TRUE;
            nTextLen -= _tcslen(pszTab);
            pszTab++;
        }

        if (bDisabled)
        {
            RECT rc;

            CopyRect(&rc, &rcMenu);

            if (!bSelected)
            {
                SetTextColor(hdcItem, crBtnHilite);

                DrawText(hdcItem, lpmix->pszText, nTextLen, &rc,
                    (lpmix->fFlags & ODMFT_MENUBAR) ? dwCenterDTFlags : dwLeftDTFlags);
                
                if (bTabbed)
                    DrawText(hdcItem, pszTab, _tcslen(pszTab), &rc, dwRightDTFlags);
            }

            rc.top -= 2;
            rc.left -= 1;
            rc.right -= 1;

            SetTextColor(hdcItem, crGrayText);

            DrawText(hdcItem, lpmix->pszText, nTextLen, &rc,
                    (lpmix->fFlags & ODMFT_MENUBAR) ? dwCenterDTFlags : dwLeftDTFlags);

            if (bTabbed)
                DrawText(hdcItem, pszTab, _tcslen(pszTab), &rc, dwRightDTFlags);
        }
        else
        {
            SetTextColor(hdcItem, crMenuText);

            DrawText(hdcItem, lpmix->pszText, nTextLen, &rcMenu,
                (lpmix->fFlags & ODMFT_MENUBAR) ? dwCenterDTFlags : dwLeftDTFlags);

            if (!bSelected)
                SetTextColor(hdcItem, RGB(0, 0, 130));

            if (bTabbed)
                DrawText(hdcItem, pszTab, _tcslen(pszTab), &rcMenu, dwRightDTFlags);
        }
    }

    SetTextColor(hdcItem, crTextOld);
    SetBkMode(hdcItem, bkOld);

    DeleteBrush(hbBackground);
    DeleteBrush(hbBtnHilite);
    DeleteBrush(hbBtnLite);
}

// Must be a better way =\...

BOOL Menu_TrackPopupMenu(HMENU hMenu, int x, int y, HWND hwndOwner)
{
    Menu_CreateMenu(hMenu, (int *)IDP_TOOLBAR);
    return (Menu_TrackCreatedPopupMenu(hMenu, x, y, hwndOwner));
}

BOOL Menu_TrackCreatedPopupMenu(HMENU hMenu, int x, int y, HWND hwndOwner)
{
    LPMENUWINDOW lpmw = (LPMENUWINDOW)GetProp(hwndOwner, (LPCTSTR)MAKEWORD(g_hMenuAtom, 0));

    ASSERT(lpmw != NULL);

    if (x == -1 && y == -1)
    {
        POINT p;

        GetCursorPos(&p);
        
        x = p.x;
        y = p.y;
    }

    return (TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                    x, y, 0, hwndOwner, NULL));
}

static void Menu_AddMenuToWindow(LPMENUWINDOW lpmw, HMENU hMenu)
{
    int i;

    if (lpmw->ahMenu == NULL)
        lpmw->ahMenu    = (HMENU *)Mem_Alloc(sizeof(HMENU));
    else
        lpmw->ahMenu    = (HMENU *)Mem_ReAlloc(lpmw->ahMenu, Mem_Size(lpmw->ahMenu) + sizeof(HMENU));

    for (i = 0; i < lpmw->nMenuCount; i++)
    {
        if (lpmw->ahMenu[i] == hMenu)
            return;
    }

    lpmw->ahMenu[lpmw->nMenuCount]  = hMenu;
    lpmw->nMenuCount++;
}

static void Menu_RemoveMenusFromWindow(LPMENUWINDOW lpmw)
{
    int i;

    for (i = 0; i < lpmw->nMenuCount; i++)
    {
        if (IsMenu(lpmw->ahMenu[i]))
            Menu_DestroyMenu(lpmw->ahMenu[i]);
    }

    Mem_Free(lpmw->ahMenu);
    lpmw->ahMenu        = NULL;
    lpmw->nMenuCount    = 0;
}

LRESULT CALLBACK Menu_CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        CWPSTRUCT *pcp = (CWPSTRUCT *)lParam;
        if (pcp->message == WM_CREATE)
        {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)pcp->lParam;
            TCHAR szClassName[256];

            if (GetClassName(pcp->hwnd, szClassName, 256))
                if (_tcsicmp(szClassName, WC_TABCONTROL) == 0)
                    lpcs->style |= TCS_HOTTRACK;

            if (lpcs->lpszClass != MAKEINTATOM(32768) &&
                IsWindow(pcp->hwnd) )
            {
                LPMENUWINDOW lpmw = (LPMENUWINDOW)Mem_Alloc(sizeof(MENUWINDOW));

                lpmw->hwnd              = pcp->hwnd;
                lpmw->ahMenu            = NULL;
                lpmw->lpfnOldWndProc    = (WNDPROC)GetWindowLong(pcp->hwnd, GWL_WNDPROC);

                SetProp(pcp->hwnd, (LPCTSTR)MAKEWORD(g_hMenuAtom, 0), (HANDLE)lpmw);
                SubclassWindow(pcp->hwnd, Menu_HookWndProc);
            }
        }
    }

    return (CallNextHookEx((HHOOK)g_hMenuHook, nCode, wParam, lParam));
}

static LRESULT CALLBACK Menu_HookWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPMENUWINDOW lpmw = (LPMENUWINDOW)GetProp(hwnd, (LPCTSTR)MAKEWORD(g_hMenuAtom, 0));

    if (lpmw == NULL)
        return (DefWindowProc(hwnd, uMsg, wParam, lParam));

    switch (uMsg)
    {
        case WM_NCDESTROY:
            CallWindowProc(lpmw->lpfnOldWndProc, hwnd, uMsg, wParam, lParam);
            SubclassWindow(hwnd, lpmw->lpfnOldWndProc);
            RemoveProp(hwnd, (LPCTSTR)MAKEWORD(g_hMenuAtom, 0));
            Mem_Free(lpmw);
        return (0);
        case WM_EXITMENULOOP:
            Menu_RemoveMenusFromWindow(lpmw);
        break;
        case WM_INITMENUPOPUP:
            CallWindowProc(lpmw->lpfnOldWndProc, hwnd, uMsg, wParam, lParam);
            Menu_CreateMenu((HMENU)wParam, (int *)IDP_TOOLBAR);
            Menu_AddMenuToWindow(lpmw, (HMENU)wParam);
        return (0);
        /*
#if (WINVER >= 0x500)
        case WM_UNINITMENUPOPUP:
            if ((BOOL)HIWORD(lParam)) //systemmenu
            {
                Menu_DeInitSubMenu((HMENU)wParam);

                return (0);
            }
        break;
#endif /* (WINVER >= 0x500) */
        case WM_MEASUREITEM:
            if (((LPMEASUREITEMSTRUCT)lParam)->CtlType == ODT_MENU)
            {
                Menu_MeasureItem(hwnd, (LPMEASUREITEMSTRUCT)lParam);

                return (TRUE);
            }
        break;
        case WM_DRAWITEM:
            if (((LPDRAWITEMSTRUCT)lParam)->CtlType == ODT_MENU)
            {
                LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

                Menu_DrawItem(hwnd, lpdis);

                return (TRUE);
            }
        break;
        case WM_NCHITTEST:
            if (lpmw->bMenubar)
            {
                HMENU hMenu = GetMenu(hwnd);
                POINT pt;
                UINT uMenuItem;
                RECT rc;

                POINTSTOPOINT(pt, lParam);
                
                uMenuItem = MenuItemFromPoint(hwnd, hMenu, pt);

                if (uMenuItem == -1)
                    break;

                GetMenuItemRect(hwnd, hMenu, uMenuItem, &rc);
                RedrawWindow(hwnd, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
            }
        break;
    }

    return (CallWindowProc(lpmw->lpfnOldWndProc, hwnd, uMsg, wParam, lParam));
}


/******************************************************************
 * Menubar functions
 */

int Menubar_Create(HWND hwnd, HMENU hMenu)
{
    int i;
    int nMenuCount = GetMenuItemCount(hMenu);
    LPMENUWINDOW lpmw = (LPMENUWINDOW)GetProp(hwnd, (LPCTSTR)MAKEWORD(g_hMenuAtom, 0));


    for (i = 0; i < nMenuCount; i++)
        Menu_InitMenuItem(hMenu, i, TRUE, IDP_NONE, ODMFT_MENUBAR);

    lpmw->bMenubar  = TRUE;

    return (nMenuCount);
}
