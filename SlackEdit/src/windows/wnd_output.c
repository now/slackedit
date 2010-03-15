/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : output.c
 * Created    : 01/24/00
 * Owner      : pcppopper
 * Revised on : 07/03/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* resources */
#include <slack_resource.h>
#include <slack_picdef.h>

/* SlackEdit */
#include "../slack_main.h"
#include "wnd_output.h"
#include "../settings/settings.h"
#include "../settings/settings_font.h"
#include "../subclasses/sub_pcp_edit.h"

/* pcp_generic */
#include <pcp_rect.h>
#include <pcp_toolbar.h>

/* pcp_controls */
#include <pcp_basebar.h>
#include <pcp_menu.h>

/* pcp_edit */
#include <pcp_edit.h>

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

LRESULT CALLBACK OutputWindow_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void OutputWindow_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode);
static void OutputWindow_OnSize(HWND hwnd, UINT state, int cx, int cy);
static void OutputWindow_OnWindowPosChanged(HWND hwnd, const LPWINDOWPOS lpwpos);

LRESULT CALLBACK OutputWindow_Edit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

WNDPROC lpfnOldBandWndProc;
WNDPROC lpfnOldEditWndProc;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

HWND OutputWindow_Create(HWND hwndParent)
{
    RECT rc;
    HWND hwndEdit;
    HWND hwndOutputWindow;
    LPEDITINTERFACE lpInterface;
    int cx;
    int cy;

    hwndOutputWindow = BaseBar_Create(hwndParent, _T("Output Window"), g_hInstance, g_WindowSettings.nOutputWindowHeight, IDC_OUTPUTWINDOW, g_WindowSettings.nOutputWindowAlignment);

    cx = g_WindowSettings.nOutputWindowWidth;
    cy = g_WindowSettings.nOutputWindowHeight;

    BandCtrl_SetMaxSize(hwndOutputWindow, cx, cy);
    BandCtrl_SetAllowableAligns(hwndOutputWindow, CCS_BOTTOM | CCS_TOP);
    BandCtrl_SetImageList(hwndOutputWindow, Menu_GetImageList(TRUE));
    BandCtrl_SetTitleImage(hwndOutputWindow, IDP_OUTPUTWINDOW);

    GetClientRect(hwndOutputWindow, &rc);

    hwndEdit    = CreateWindowEx(WS_EX_CLIENTEDGE, WC_PCP_TEXTEDIT, NULL,
                        WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL |
                        WS_OVERLAPPED,
                        1, 0, Rect_Width(&rc) + 2, Rect_Height(&rc),
                        hwndOutputWindow, (HMENU)IDC_OUTPUTWINDOW_EDIT, g_hInstance, NULL);

    BandCtrl_SetClientWindow(hwndOutputWindow, hwndEdit);

    Font_Initialize(hwndEdit);

    lpInterface = PCP_Edit_GetInterface(hwndEdit);

    PCP_TextEdit_SetSelectionMargin(lpInterface, FALSE);
    PCP_Edit_SetReadOnly(lpInterface, TRUE);

    lpfnOldBandWndProc = SubclassWindow(g_hwndOutputWindow, OutputWindow_WndProc);
    lpfnOldEditWndProc = SubclassWindow(hwndEdit, OutputWindow_Edit_WndProc);

    SetWindowPos(g_hwndOutputWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME);

    return (hwndOutputWindow);
}

HWND OutputWindow_Destroy(HWND hwndOutputWindow)
{
    if (DestroyWindow(hwndOutputWindow))
    {
        return (NULL);
    }
    else
    {
        return (hwndOutputWindow);
    }
}

LRESULT CALLBACK OutputWindow_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_COMMAND,            OutputWindow_OnCommand);
        HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGED,   OutputWindow_OnWindowPosChanged);
        case WM_SETFOCUS:
            SetFocus(GetDlgItem(hwnd, IDC_OUTPUTWINDOW_EDIT));
        return (0);
    }

    return (CallWindowProc(lpfnOldBandWndProc, hwnd, uMsg, wParam, lParam));
}

static void OutputWindow_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode)
{
    switch (idCtl)
    {
    /* process something or other */
    case IDC_OUTPUTWINDOW_EDIT:
    return;
    }

    CallWindowProc(lpfnOldBandWndProc, hwnd, WM_COMMAND, MAKEWPARAM(idCtl, uNotifyCode), (LPARAM)hwndCtl);
}

static void OutputWindow_OnWindowPosChanged(HWND hwnd, const LPWINDOWPOS lpwpos)
{
    g_WindowSettings.nOutputWindowXPos      = lpwpos->x;
    g_WindowSettings.nOutputWindowYPos      = lpwpos->y;
    g_WindowSettings.nOutputWindowWidth     = lpwpos->cx;
    g_WindowSettings.nOutputWindowHeight    = lpwpos->cy;
    g_WindowSettings.nOutputWindowAlignment = BandCtrl_GetAlignment(hwnd);

    FORWARD_WM_WINDOWPOSCHANGED(hwnd, (LPARAM)lpwpos, lpfnOldBandWndProc);
}

LRESULT CALLBACK OutputWindow_Edit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPEDITINTERFACE lpInterface = PCP_Edit_GetInterface(hwnd);

    switch (uMsg)
    {
    case WM_CONTEXTMENU:
        /* show some lame context menu, like with copy and hide and clear or something */
    return (0);
    case WM_LBUTTONDBLCLK:
        /* Do line go-to's or similar */
    return (0);
    case WM_CHAR:
        switch ((UINT)wParam)
        {
        case VK_ESCAPE:
            if (PCP_Edit_GetSelectionMode(lpInterface) == PE_SELECTION_MODE_NONE)
            {
                SetFocus(MDI_MyGetActive(TRUE));
                g_hwndOutputWindow = OutputWindow_Destroy(g_hwndOutputWindow); // g_hwndMain
            }
        break;
        }
    break;
    }

    return (CallWindowProc(lpfnOldEditWndProc, hwnd, uMsg, wParam, lParam));
}
