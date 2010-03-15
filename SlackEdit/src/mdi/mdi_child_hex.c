/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_mdi_child_hex.c
 * Created    : 03/09/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:15:59
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* windows */
#include <commctrl.h>

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "mdi_child_generic.h"
#include "mdi_child_hex.h"
#include "child_tclcommandline.h"
#include "../controls/pcp_hexedit.h"
#include "../parsing/file_actions.h"
#include "../subclasses/sub_pcp_hex.h"

/* pcp_generic */
#include <pcp_toolbar.h>
#include <pcp_path.h>
#include <pcp_string.h>
#include <pcp_rect.h>

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

BOOL CALLBACK MDI_Child_Hex_SaveEnumProc(HWND hwnd, LPARAM lParam);

static BOOL MDI_Child_Hex_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void MDI_Child_Hex_OnSize(HWND hwnd, UINT state, int cx, int cy);
static void MDI_Child_Hex_OnMDIActivate(HWND hwnd, BOOL fActive, HWND hwndActivate, HWND hwndDeactivate);
static void MDI_Child_Hex_OnSetFocus(HWND hwnd, HWND hwndLoseFocus);
static void MDI_Child_Hex_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT uNotifyCode);
static void MDI_Child_Hex_OnClose(HWND hwnd);
static BOOL MDI_Child_Hex_OnQueryEndSession(HWND hwnd);
static LRESULT MDI_Child_Hex_OnNotify(HWND hwnd, int id, LPNMHDR pnmh);
static void MDI_Child_Hex_OnMenuSelect(HWND hwnd, HMENU hMenu, int item, HMENU hMenuPopup, UINT flags);
static void MDI_Child_Hex_OnTimer(HWND hwnd, UINT id);
static BOOL MDI_Child_Hex_OnPreClose(HWND hwnd);
static void MDI_Child_Hex_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

LRESULT CALLBACK MDI_Child_Hex_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE,             MDI_Child_Hex_OnCreate);
        HANDLE_MSG(hwnd, WM_SIZE,               MDI_Child_Hex_OnSize);
        HANDLE_MSG(hwnd, WM_MDIACTIVATE,        MDI_Child_Hex_OnMDIActivate);
        HANDLE_MSG(hwnd, WM_SETFOCUS,           MDI_Child_Hex_OnSetFocus);
        HANDLE_MSG(hwnd, WM_COMMAND,            MDI_Child_Hex_OnCommand);
        HANDLE_MSG(hwnd, WM_CLOSE,              MDI_Child_Hex_OnClose);
        HANDLE_MSG(hwnd, WM_QUERYENDSESSION,    MDI_Child_Hex_OnQueryEndSession);
        HANDLE_MSG(hwnd, WM_NOTIFY,             MDI_Child_Hex_OnNotify);
        HANDLE_MSG(hwnd, WM_MENUSELECT,         MDI_Child_Hex_OnMenuSelect);
        HANDLE_MSG(hwnd, WM_TIMER,              MDI_Child_Hex_OnTimer);
        HANDLE_MSG(hwnd, WM_SYSCOMMAND,         MDI_Child_Hex_OnSysCommand);
    }

    return (DefMDIChildProc(hwnd, uMsg, wParam, lParam));
}

static BOOL MDI_Child_Hex_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    LPFILEINFO lpfi = (LPFILEINFO)((LPMDICREATESTRUCT)lpCreateStruct->lpCreateParams)->lParam;

    HexEdit_Create(hwnd, lpfi->szFileName);

    return (MDI_Child_Generic_OnCreate(hwnd, lpCreateStruct));
}

static void MDI_Child_Hex_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    if (state != SIZE_MINIMIZED)
    {
        RECT rcTclCmd;
        HWND hwndTclCmd = GetDlgItem(hwnd, IDC_CBO_TCL);
        HWND hwndEdit   = GetDlgItem(hwnd, IDC_EDIT_MDICHILD);
        HDWP hdwp = BeginDeferWindowPos(2); //EDIT and TCLCMD

        GetWindowRect(hwndTclCmd, &rcTclCmd);

        hdwp = DeferWindowPos(hdwp, hwndTclCmd, 0,
                    0, cy - Rect_Height(&rcTclCmd),
                    cx, Rect_Height(&rcTclCmd), SWP_NOZORDER);

        hdwp = DeferWindowPos(hdwp, hwndEdit, 0,
                    0, 0,
                    cx, cy - Rect_Height(&rcTclCmd) - 2, SWP_NOZORDER);

        EndDeferWindowPos(hdwp);
    }
    else
    {
        Main_UpdateWindowBar(hwnd, SIZE_MINIMIZED, 0L);
    }

    MDI_Child_Generic_OnSize(hwnd, state, cx, cy);
}

static void MDI_Child_Hex_OnMDIActivate(HWND hwnd, BOOL fActive, HWND hwndActivate, HWND hwndDeactivate)
{
    MDI_Child_Hex_EnableToolbarButtons(hwnd, (hwndDeactivate == hwnd && hwndActivate == NULL));

    MDI_Child_Generic_OnMDIActivate(hwnd, fActive, hwndActivate, hwndDeactivate);
}

static void MDI_Child_Hex_OnSetFocus(HWND hwnd, HWND hwndLoseFocus)
{
    Window_SetFocusToDlgItem(hwnd, IDC_EDIT_MDICHILD);
}

static void MDI_Child_Hex_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT uNotifyCode)
{
    MDI_Child_Generic_OnCommand(hwnd, id, hwndCtl, uNotifyCode);
}

static void MDI_Child_Hex_OnClose(HWND hwnd)
{
    MDI_Child_Hex_OnPreClose(hwnd);
}

static BOOL MDI_Child_Hex_OnQueryEndSession(HWND hwnd)
{
    return (MDI_Child_Hex_OnPreClose(hwnd));
}

static LRESULT MDI_Child_Hex_OnNotify(HWND hwnd, int id, LPNMHDR pnmh)
{
    switch (pnmh->code)
    {
        case CBEN_ENDEDIT:
            switch (pnmh->idFrom)
            {
                case IDC_CBO_TCL:
                return (TclCmd_EndEdit((PNMCBEENDEDIT)pnmh));
            }
        break;
//      case PEN_SELCHANGE:
//          TextView_UpdateStatusbar(MDI_GetEditView(GetDlgItem(hwnd, IDC_EDIT_MDICHILD)));
//      return (0);
//      case PEN_INCREMENTALSEARCHUPDATE:
//          TextView_IncrementalSearchUpdateStatusbar(MDI_GetEditView(GetDlgItem(hwnd, IDC_EDIT_MDICHILD)));
//      return (0);
    }

    return (MDI_Child_Generic_OnNotify(hwnd, id, pnmh));

}

static void MDI_Child_Hex_OnMenuSelect(HWND hwnd, HMENU hMenu, int item, HMENU hMenuPopup, UINT flags)
{
    PostMessage(g_hwndMain, WM_MENUSELECT, MAKEWPARAM(item, flags), (LPARAM)hMenu);
}

static void MDI_Child_Hex_OnTimer(HWND hwnd, UINT id)
{
    MDI_Child_Generic_OnTimer(hwnd, id);
}

static BOOL MDI_Child_Hex_OnPreClose(HWND hwnd)
{
    HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT_MDICHILD);

    if (FALSE) //HexEdit_GetModify(hwndEdit))
    {
        TCHAR szMsg[MAX_PATH + 18];
        int nOpt;
        FILEINFO fi;

        Main_GetFileInfo(hwnd, &fi);

        _stprintf(szMsg, String_LoadString(IDS_FORMAT_SAVE_NOTSAVED), Path_GetFileName(fi.szFileName));

        nOpt = MessageBox(g_hwndMain, szMsg, g_szAppName,
                        MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL);

        switch (nOpt)
        {
            case IDYES:
                Window_Command(g_hwndMain, IDM_FILE_SAVE, 0, 0);
                File_Close(hwnd);
            break;
            case IDNO:
                File_Close(hwnd);
            break;
            case IDCANCEL:
            return (FALSE);
        }
    }
    else
    {
        File_Close(hwnd);
    }

    return (TRUE);
}

static void MDI_Child_Hex_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
    MDI_Child_Generic_OnSysCommand(hwnd, cmd, x, y);
}

void MDI_Child_Hex_EnableToolbarButtons(HWND hwnd, BOOL bLast)
{
    HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT_MDICHILD);
    BOOL bSave  = ((bLast) ? FALSE : TRUE);
    BOOL bUndo  = (!bLast && FALSE); // HexEdit_CanUndo(hwndEdit);
    BOOL bRedo  = (!bLast && FALSE); // HexEdit_CanRedo(hwndEdit);

    Toolbar_EnableButton(g_hwndToolbarMain, IDM_FILE_SAVE, bSave);
    Toolbar_EnableButton(g_hwndToolbarMain, IDM_EDIT_UNDO, bUndo);
    Toolbar_EnableButton(g_hwndToolbarMain, IDM_EDIT_REDO, bRedo);
}
