/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_mdi_child_edit.c
 * Created    : not known (before 06/22/00)
 * Owner      : pcppopper
 * Revised on : 07/16/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>
#include <commctrl.h>

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "mdi_child_generic.h"
#include "mdi_child_edit.h"
#include "child_tclcommandline.h"
#include "../dialogs/dlg_find.h"
#include "../parsing/file_actions.h"
#include "../settings/settings.h"
#include "../settings/settings_font.h"
#include "../subclasses/sub_pcp_edit.h"
#include "../windows/wnd_statusbar.h"

/* pcp_controls */
#include <pcp_statusbar.h>

/* pcp_generic */
#include <pcp_string.h>
#include <pcp_clipboard.h>
#include <pcp_rect.h>
#include <pcp_editctrl.h>
#include <pcp_mem.h>
#include <pcp_point.h>
#include <pcp_path.h>
#include <pcp_toolbar.h>

/* pcp_edit */

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

BOOL CALLBACK MDI_Child_Edit_SaveEnumProc(HWND hwnd, LPARAM lParam);

static BOOL MDI_Child_Edit_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void MDI_Child_Edit_OnSize(HWND hwnd, UINT state, int cx, int cy);
static void MDI_Child_Edit_OnMDIActivate(HWND hwnd, BOOL fActive, HWND hwndActivate, HWND hwndDeactivate);
static void MDI_Child_Edit_OnSetFocus(HWND hwnd, HWND hwndLoseFocus);
static void MDI_Child_Edit_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT uNotifyCode);
static void MDI_Child_Edit_OnClose(HWND hwnd);
static BOOL MDI_Child_Edit_OnQueryEndSession(HWND hwnd);
static LRESULT MDI_Child_Edit_OnNotify(HWND hwnd, int id, LPNMHDR pnmh);
static void MDI_Child_Edit_OnMenuSelect(HWND hwnd, HMENU hMenu, int item, HMENU hMenuPopup, UINT flags);
static void MDI_Child_Edit_OnTimer(HWND hwnd, UINT id);
static BOOL MDI_Child_Edit_OnPreClose(HWND hwnd);
static void MDI_Child_Edit_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);

void MDI_Child_Edit_Clipboard_AppendText(HWND hwnd, BOOL bPrepend, BOOL bCut);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

LRESULT CALLBACK MDI_Child_Edit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE,             MDI_Child_Edit_OnCreate);
        HANDLE_MSG(hwnd, WM_SIZE,               MDI_Child_Edit_OnSize);
        HANDLE_MSG(hwnd, WM_MDIACTIVATE,        MDI_Child_Edit_OnMDIActivate);
        HANDLE_MSG(hwnd, WM_SETFOCUS,           MDI_Child_Edit_OnSetFocus);
        HANDLE_MSG(hwnd, WM_COMMAND,            MDI_Child_Edit_OnCommand);
        HANDLE_MSG(hwnd, WM_CLOSE,              MDI_Child_Edit_OnClose);
        HANDLE_MSG(hwnd, WM_QUERYENDSESSION,    MDI_Child_Edit_OnQueryEndSession);
        HANDLE_MSG(hwnd, WM_NOTIFY,             MDI_Child_Edit_OnNotify);
        HANDLE_MSG(hwnd, WM_MENUSELECT,         MDI_Child_Edit_OnMenuSelect);
        HANDLE_MSG(hwnd, WM_TIMER,              MDI_Child_Edit_OnTimer);
        HANDLE_MSG(hwnd, WM_SYSCOMMAND,         MDI_Child_Edit_OnSysCommand);
    }

    return (DefMDIChildProc(hwnd, uMsg, wParam, lParam));
}

static BOOL MDI_Child_Edit_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HCURSOR hCursor = NULL;
    LPFILEINFO lpfi = (LPFILEINFO)((LPMDICREATESTRUCT)lpCreateStruct->lpCreateParams)->lParam;
    LPEDITINTERFACE lpInterface;

    EditView_Create(hwnd, (lpfi->dwFileType == FI_TYPE_TEXT) ? lpfi->szFileName : NULL);

    if (Path_FileExists(g_ImageSettings.szMarginCursorFile))
        hCursor = LoadCursorFromFile(g_ImageSettings.szMarginCursorFile);

    if (hCursor != NULL)
        DestroyCursor(TextView_SetMarginCursor(MDI_GetEditView(GetDlgItem(hwnd, IDC_EDIT_MDICHILD)), hCursor));

    Font_Initialize(GetDlgItem(hwnd, IDC_EDIT_MDICHILD));

    lpInterface = PCP_Edit_GetInterface(GetDlgItem(hwnd, IDC_EDIT_MDICHILD));

    EditChild_SetLineFeed(hwnd, PCP_TextEdit_GetCRLFMode(lpInterface));

    /* return the generic oncreate stuff */
    return (MDI_Child_Generic_OnCreate(hwnd, lpCreateStruct));
}

static void MDI_Child_Edit_OnSize(HWND hwnd, UINT state, int cx, int cy)
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

    // do the generic onsize stuff

    MDI_Child_Generic_OnSize(hwnd, state, cx, cy);
}

static void MDI_Child_Edit_OnMDIActivate(HWND hwnd, BOOL fActive, HWND hwndActivate, HWND hwndDeactivate)
{
    MDI_Child_Edit_EnableToolbarButtons(hwnd, (hwndDeactivate == hwnd && hwndActivate == NULL));
    
    MDI_Child_Edit_UpdateOverWriteMode(hwnd);

    EditChild_UpdateStatusbarInfo(hwndActivate);

    MDI_Child_Generic_OnMDIActivate(hwnd, fActive, hwndActivate, hwndDeactivate);
}

static void MDI_Child_Edit_OnSetFocus(HWND hwnd, HWND hwndLoseFocus)
{
    Window_SetFocusToDlgItem(hwnd, IDC_EDIT_MDICHILD);
}

static void MDI_Child_Edit_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT uNotifyCode)
{
    switch (id)
    {
        case IDM_EDIT_CUT_APPENDCUTTOCLIPBOARD:
            MDI_Child_Edit_Clipboard_AppendText(GetDlgItem(hwnd, IDC_EDIT_MDICHILD), FALSE, TRUE);
        return;
        case IDM_EDIT_CUT_APPENDCLIPBOARDTOCUT:
            MDI_Child_Edit_Clipboard_AppendText(GetDlgItem(hwnd, IDC_EDIT_MDICHILD), TRUE, TRUE);
        return;
        case IDM_EDIT_COPY_APPENDCOPYTOCLIPBOARD:
            MDI_Child_Edit_Clipboard_AppendText(GetDlgItem(hwnd, IDC_EDIT_MDICHILD), FALSE, FALSE);
        return;
        case IDM_EDIT_COPY_APPENDCLIPBOARDTOCOPY:
            MDI_Child_Edit_Clipboard_AppendText(GetDlgItem(hwnd, IDC_EDIT_MDICHILD), TRUE, FALSE);
        return;
        case IDM_SEARCH_FIND:
            Find_Edit_CreateDialog(g_hwndMain, FALSE);
        return;
        case IDM_SEARCH_REPLACE:
            Find_Edit_CreateDialog(g_hwndMain, TRUE);
        return;
        default:
            MDI_Child_Generic_OnCommand(hwnd, id, hwndCtl, uNotifyCode);
        break;
    }
}

static void MDI_Child_Edit_OnTimer(HWND hwnd, UINT id)
{
    MDI_Child_Generic_OnTimer(hwnd, id);
}

static LRESULT MDI_Child_Edit_OnNotify(HWND hwnd, int id, LPNMHDR pnmh)
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
        case PEN_SELCHANGE:
            TextView_UpdateStatusbar(MDI_GetEditView(GetDlgItem(hwnd, IDC_EDIT_MDICHILD)));
        return (0);
        case PEN_INCREMENTALSEARCHUPDATE:
            TextView_IncrementalSearchUpdateStatusbar((LPNMINCREMENTALSTATUS)pnmh);
        return (0);
        case PEN_OVERWRITEMODECHANGED:
            MDI_Child_Edit_UpdateOverWriteMode(hwnd);
        return (0);
    }

    return (MDI_Child_Generic_OnNotify(hwnd, id, pnmh));
}

static void MDI_Child_Edit_OnClose(HWND hwnd)
{
    MDI_Child_Edit_OnPreClose(hwnd);
}

static BOOL MDI_Child_Edit_OnQueryEndSession(HWND hwnd)
{
    return (MDI_Child_Edit_OnPreClose(hwnd));
}

static void MDI_Child_Edit_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
    MDI_Child_Generic_OnSysCommand(hwnd, cmd, x, y);
}

static BOOL MDI_Child_Edit_OnPreClose(HWND hwnd)
{
    HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT_MDICHILD);

    if (EditView_GetModify(hwndEdit))
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

void MDI_Child_Edit_EnableToolbarButtons(HWND hwnd, BOOL bLast)
{
    HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT_MDICHILD);
    BOOL bSave  = ((bLast) ? FALSE : TRUE);
    BOOL bUndo  = (!bLast && EditView_CanUndo(hwndEdit));
    BOOL bRedo  = (!bLast && EditView_CanRedo(hwndEdit));

    Toolbar_EnableButton(g_hwndToolbarMain, IDM_FILE_SAVE, bSave);
    Toolbar_EnableButton(g_hwndToolbarMain, IDM_EDIT_UNDO, bUndo);
    Toolbar_EnableButton(g_hwndToolbarMain, IDM_EDIT_REDO, bRedo);
}

static void MDI_Child_Edit_OnMenuSelect(HWND hwnd, HMENU hMenu, int item, HMENU hMenuPopup, UINT flags)
{
    PostMessage(g_hwndMain, WM_MENUSELECT, MAKEWPARAM(item, flags), (LPARAM)hMenu);
}

void MDI_Child_Edit_Clipboard_AppendText(HWND hwnd, BOOL bPrepend, BOOL bCut)
{
    LPTSTR pszSelection;

    EditView_GetSelText(hwnd, &pszSelection);

    Clipboard_AppendText(pszSelection, bPrepend);

    if (bCut)
        EditView_Cut(hwnd);
}

void MDI_Child_Edit_UpdateOverWriteMode(HWND hwnd)
{
    STATUSBARPANE sp;
    BOOL bOverwriteMode = EditView_GetOverWriteMode(GetDlgItem(hwnd, IDC_EDIT_MDICHILD));

    sp.fMask    = SF_CRBG | SF_CRFG | SF_TEXT | SF_STYLE;
    sp.nPane    = 3;

    if (!bOverwriteMode)
    {
        sp.pszText      = String_LoadString(IDS_STATUSBARPANE_INSERT);
        sp.crBg         = SB_DEFAULT_CRBG;
        sp.crFg         = SB_DEFAULT_CRFG;
        sp.uStyle       = 0;
    }
    else
    {
        sp.pszText      = String_LoadString(IDS_STATUSBARPANE_OVERWRITE);
        sp.crBg         = RGB(240, 0, 0);
        sp.crFg         = RGB(255, 255, 255);
        sp.uStyle       = SBT_POPOUT;
    }

    Statusbar_SetPane(g_hwndStatusbarMain, &sp);
}
