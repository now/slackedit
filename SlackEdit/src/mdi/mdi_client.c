/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_mdi_client.c
 * Created    : not known (before 12/06/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:33:16
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "mdi_client.h"
#include "../dialogs/dlg_windowlist.h"
#include "../settings/settings.h"

/* pcp_generic */
#include <pcp_mdi.h>
#include <pcp_rect.h>
#include <pcp_path.h>
#include <pcp_string.h>

/* pcp_paint */
#include <pcp_paint.h>

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

static LRESULT CALLBACK MDIClient_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void MDI_Client_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT uNotifyCode);
static BOOL MDI_Client_OnEraseBkgnd(HWND hwnd, HDC hdc);

static BOOL CALLBACK MDI_Client_DestroyEnumProc(HWND hwnd, LPARAM lParam);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

static WNDPROC lpfnDefMDIClientProc;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

HWND MDI_Client_Create(HWND hwndParent)
{
    HWND hwndMDIClient;
    CLIENTCREATESTRUCT ccs;

    ccs.hWindowMenu  = NULL; // Main_GetMainSubMenu(hwndParent, IDM_MAIN_WINDOW);
    ccs.idFirstChild = IDM_MDI_FIRSTCHILD;

    hwndMDIClient = CreateWindowEx(WS_EX_CLIENTEDGE, _T("MDICLIENT"), NULL,
                                WS_CHILDWINDOW | WS_CLIPCHILDREN |
                                WS_VISIBLE | WS_OVERLAPPED |
                                WS_CLIPSIBLINGS,
                                0, 0, 0, 0,
                                hwndParent, (HMENU)IDC_MDICLIENT_MAIN, g_hInstance,
                                (LPVOID)&ccs);

    lpfnDefMDIClientProc = SubclassWindow(hwndMDIClient, MDIClient_WndProc);

    return (hwndMDIClient);
}

static LRESULT CALLBACK MDIClient_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_COMMAND,    MDI_Client_OnCommand);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, MDI_Client_OnEraseBkgnd);
    }

    return (CallWindowProc(lpfnDefMDIClientProc, hwnd, uMsg, wParam, lParam));
}

static void MDI_Client_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT uNotifyCode)
{
    switch (id)
    {
    case IDM_WINDOW_CLOSE:
        Window_Close(MDI_GetActive(hwnd, NULL));
    return;
    case IDM_WINDOW_CLOSEALL:
        MDI_Client_DestroyChildren();
    return;
    case IDM_WINDOW_NEXT:
        MDI_Next(hwnd, NULL, FALSE);
    return;
    case IDM_WINDOW_PREVIOUS:
        MDI_Next(hwnd, NULL, TRUE);
    return;
    case IDM_WINDOW_CASCADE:
        MDI_Cascade(hwnd, 0);
        g_WindowSettings.nMDIWinState = MDISTATE_NORMAL;
    return;
    case IDM_WINDOW_TILEHORIZONTAL:
        MDI_Tile(hwnd, MDITILE_HORIZONTAL);
        g_WindowSettings.nMDIWinState = MDISTATE_TILEHORZ;
    return;
    case IDM_WINDOW_TILEVERTICAL:
        MDI_Tile(hwnd, MDITILE_VERTICAL);
        g_WindowSettings.nMDIWinState = MDISTATE_TILEVERT;
    return;
    case IDM_WINDOW_WINDOWLIST:
        WindowList_CreateDialogBox(g_hwndMain);
    return;
    default:
    {
        HWND hwndChild = MDI_GetActive(hwnd, NULL);

        if (hwndChild)
            Window_Command(hwndChild, id, uNotifyCode, hwndCtl);
    }
    return;
    }
}

static BOOL MDI_Client_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    RECT rc;

    GetClientRect(hwnd, &rc);

    if (g_ImageSettings.bMDIImage)
    {
        BITMAP bmp;
        HBITMAP hBitmap;
        //HBITMAP hFlippedBitmap
        HBITMAP hOldBitmap;
        HDC hdcMem = CreateCompatibleDC(hdc);
        hBitmap = (HBITMAP)LoadImage(g_hInstance, g_ImageSettings.szMDIImage,
                                IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

        if (hBitmap == NULL)
            return (FALSE); // FIXME: or?

        //hFlippedBitmap = Paint_Bitmap_FlipBitmap(hdcMem, hBitmap);
        //DeleteObject(hFlippedBitmap);
        hOldBitmap = SelectObject(hdcMem, hBitmap);
        GetObject(hBitmap, sizeof(BITMAP), &bmp);
        
        if (g_ImageSettings.bMDITile)
        {
            int x, y;

            for (y = 0; y < Rect_Height(&rc); y += bmp.bmHeight)
            {
                for (x = 0; x < Rect_Width(&rc); x += bmp.bmWidth)
                {
                    BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hdcMem,
                            0, 0, SRCCOPY);
                }
            }
        }
        else
        {
            StretchBlt(hdc, 0, 0, Rect_Width(&rc), Rect_Height(&rc), hdcMem,
                0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
        }

        DeleteObject(SelectObject(hdcMem, hOldBitmap));
        DeleteDC(hdcMem);

        return (TRUE);
    }
    else if (g_ImageSettings.bMDIHatch)
    {
        int i, x, y;
        HBRUSH hbrGray = (HBRUSH)GetStockObject(GRAY_BRUSH);

        SetMapMode(hdc, MM_ANISOTROPIC);
        SetWindowExtEx(hdc, 100, 100, NULL);
        SetViewportExtEx(hdc, rc.right, rc.bottom, NULL);

        FillRect(hdc, &rc, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

        for (i = 0; i < 13; i++)
        {
            x = (i * 40) % 100;

            y = ((i * 40) / 100) * 20;
            SetRect(&rc, x, y, x + 20, y + 20);
            FillRect(hdc, &rc, hbrGray);
        }

        return (TRUE);
    }
    else if (g_ImageSettings.bMDITransparent)
    {
        int cx, cy;
        HWND hwndDesktop    = GetDesktopWindow();
        HDC hdcWin          = GetDC(hwnd);
        HDC hdcDesktop;

        hdcDesktop = GetDCEx(hwndDesktop, NULL, DCX_CACHE | DCX_LOCKWINDOWUPDATE);
        
        cx = GetSystemMetrics(SM_CXSCREEN) / 10;
        cy = GetSystemMetrics(SM_CYSCREEN) / 10;
        
        BitBlt(hdcWin, 0, 0, cx, cy, hdcDesktop, 0, 0, SRCCOPY);

        ReleaseDC(hwnd, hdcWin);
        DeleteDC(hdcDesktop);

        return (TRUE);
    }

    return (CallWindowProc(lpfnDefMDIClientProc, hwnd, WM_ERASEBKGND, (WPARAM)hdc, (LPARAM)0L));
}

BOOL MDI_Client_DestroyChildren(void)
{
    EnumChildWindows(g_hwndMDIClient, MDI_Client_DestroyEnumProc, 0);

    return ((MDI_GetActive(g_hwndMDIClient, NULL) == NULL) ? TRUE : FALSE);
}

static BOOL CALLBACK MDI_Client_DestroyEnumProc(HWND hwnd, LPARAM lParam)
{
    TCHAR szClass[80];

    if (GetWindow(hwnd, GW_OWNER))
        return (TRUE);

    GetClassName(hwnd, szClass, 80);

    if (String_Equal(szClass, g_szEditChild, FALSE) || String_Equal(szClass, g_szHexChild, FALSE))
    {
        if (!SendMessage(hwnd, WM_QUERYENDSESSION, 0, 0L))
            return (TRUE);

        if (IsWindow(hwnd))
            MDI_Destroy(g_hwndMDIClient, hwnd);

        if (IsWindow(hwnd) && GetParent(hwnd))
            return (FALSE);
    }

    return (TRUE);
}

void MDI_Client_ResizeChildren(HWND hwnd)
{
    RECT rc;
    int cx, cy;
    HWND hwndChild;
    int cWnds = 0;
    HDWP hdwp;

    if (!IsWindow(hwnd) || g_WindowSettings.nMDIWinState != MDISTATE_NORMAL)
        return;

    GetClientRect(hwnd, &rc);
    cx = (int)(rc.right * 0.90);
    cy = (int)(rc.bottom * 0.85);

    /*  this needs to be moved to the window list func
        to take care of other MDISTATEs... no prob just do it! */
    for (hwndChild = GetWindow(hwnd, GW_CHILD);
        hwndChild != NULL;
        hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
    {
        if (GetWindow(hwndChild, GW_OWNER) || IsMaximized(hwndChild)
            || IsMinimized(hwndChild))
        {
            continue;
        }
        else
        {
            TCHAR szClass[80];

            GetClassName(hwndChild, szClass, 80);

            if (String_Equal(szClass, g_szEditChild, FALSE) ||
                String_Equal(szClass, g_szHexChild, FALSE))
            {
                cWnds++;
            }
        }
    }

    hdwp = BeginDeferWindowPos(cWnds);

    for (hwndChild = GetWindow(hwnd, GW_CHILD);
        hwndChild != NULL;
        hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
    {
        if (GetWindow(hwndChild, GW_OWNER) || IsMaximized(hwndChild)
            || IsMinimized(hwndChild))
        {
            continue;
        }
        else
        {
            TCHAR szClass[80];
        
            GetClassName(hwndChild, szClass, 80);

            if (String_Equal(szClass, g_szEditChild, FALSE) ||
                String_Equal(szClass, g_szHexChild, FALSE))
            {
                hdwp = DeferWindowPos(hdwp, hwndChild, 0, 0, 0,
                    cx, cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
            }
        }
    }
    
    EndDeferWindowPos(hdwp);
}

void MDI_Client_UpdateWindowMenu(HMENU hMenu)
{
    MENUITEMINFO mii;
    MENUITEMINFO miiSeparator;
    HWND hwndChild;
    int i;
    TCHAR szItem[MAX_PATH];
    TCHAR szTemp[40];
    FILEINFO fi;

    INITSTRUCT(mii, TRUE);
    mii.fMask       = MIIM_TYPE | MIIM_ID;
    mii.wID         = IDM_MDI_FIRSTCHILD;

    INITSTRUCT(miiSeparator, TRUE);
    miiSeparator.fMask      = MIIM_ID;

    for (i = IDM_MDI_FIRSTCHILD; i < (IDM_MDI_FIRSTCHILD + 9); i++)
    {
        if (GetMenuItemInfo(hMenu, i, FALSE, &miiSeparator))
            DeleteMenu(hMenu, i, MF_BYCOMMAND);
    }

    if (GetMenuItemInfo(hMenu, IDM_WINDOW_WINDOWLIST_SEPARATOR2, FALSE, &miiSeparator))
        DeleteMenu(hMenu, IDM_WINDOW_WINDOWLIST_SEPARATOR2, MF_BYCOMMAND);

    for (
        hwndChild = GetWindow(g_hwndMDIClient, GW_CHILD), i = 1;
        hwndChild != NULL && i < 10;
        hwndChild = GetWindow(hwndChild, GW_HWNDNEXT)
        )
    {
        if (GetWindow(hwndChild, GW_OWNER) != NULL)
            continue;

        if (i == 1)
        {
            MENUITEMINFO miiTemp;

            INITSTRUCT(miiTemp, TRUE);
            miiTemp.fMask   = MIIM_TYPE | MIIM_ID;
            miiTemp.wID     = IDM_WINDOW_WINDOWLIST_SEPARATOR2;
            miiTemp.fType   = MFT_SEPARATOR;

            InsertMenuItem(hMenu, GetMenuItemCount(hMenu), TRUE, &miiTemp);
        }

        Main_GetFileInfo(hwndChild, &fi);

        Path_CompactPath(szTemp, fi.szFileName, 35);

        _stprintf(szItem, String_LoadString(IDS_FORMAT_GENERIC_LIST_MENUITEM), i, szTemp);

        mii.fType       = MFT_STRING;
        mii.dwTypeData  = szItem;

        InsertMenuItem(hMenu, GetMenuItemCount(hMenu), TRUE, &mii);

        i++;
        mii.wID++;
    }

    CheckMenuRadioItem(hMenu, IDM_MDI_FIRSTCHILD, mii.wID, IDM_MDI_FIRSTCHILD, MF_BYCOMMAND);

    if (GetMenuItemInfo(hMenu, IDM_WINDOW_WINDOWLIST_SEPARATOR, FALSE, &miiSeparator))
        DeleteMenu(hMenu, IDM_WINDOW_WINDOWLIST_SEPARATOR, MF_BYCOMMAND);

    mii.wID     = IDM_WINDOW_WINDOWLIST_SEPARATOR;
    mii.fType   = MFT_SEPARATOR;

    InsertMenuItem(hMenu, GetMenuItemCount(hMenu), TRUE, &mii);

    if (GetMenuItemInfo(hMenu, IDM_WINDOW_WINDOWLIST, FALSE, &miiSeparator))
        DeleteMenu(hMenu, IDM_WINDOW_WINDOWLIST, MF_BYCOMMAND);

    mii.wID     = IDM_WINDOW_WINDOWLIST;
    mii.fType   = MFT_STRING;
    _tcscpy(szItem, String_LoadString(IDS_WINDOW_WINDOWLIST_MENUITEM));
    mii.dwTypeData  = szItem;

    InsertMenuItem(hMenu, GetMenuItemCount(hMenu), TRUE, &mii);
}

void MDI_Client_ActivateChild(UINT uID)
{
    HWND hwndChild;
    UINT i;

    for (i = IDM_MDI_FIRSTCHILD, hwndChild = GetWindow(g_hwndMDIClient, GW_CHILD);
        hwndChild != NULL;
        hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
    {
        if (GetWindow(hwndChild, GW_OWNER) != NULL)
            continue;

        if (uID == i)
        {
            MDI_Activate(g_hwndMDIClient, hwndChild);

            break;
        }

        i++;
    }
}

