/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : tclcmd.c
 * Created    : not known (before 12/22/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:18:28
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* windows */
#include <commctrl.h>

/* SlackEdit */
#include "../slack_main.h"
#include "child_tclcommandline.h"
#include "../settings/settings.h"
#include "../tcl/tcl_main.h"

/* pcp_generic */
#include <pcp_combobox.h>
#include <pcp_mru.h>

LRESULT CALLBACK TclCmd_Cbo_Edit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

WNDPROC lpfnDefTclCmdCboEditProc;

BOOL TclCmd_Create(HWND hwndParent)
{
    HWND hwndCbo;

    hwndCbo = CreateWindowEx(0, WC_COMBOBOXEX, NULL,
                                WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL |
                                WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                                CCS_NORESIZE | CBS_AUTOHSCROLL | CBS_DROPDOWN,
                                0, 0, 150, 160,
                                hwndParent, (HMENU)IDC_CBO_TCL, g_hInstance, NULL);

    MRU_SetComboBoxEx(hwndCbo, _T("TclCmd MRU"), g_MRUSettings.nTclCmdMax);

    lpfnDefTclCmdCboEditProc = SubclassWindow(ComboBoxEx_GetEditControl(hwndCbo), TclCmd_Cbo_Edit_WndProc);

    return (TRUE);
}

BOOL TclCmd_EndEdit(PNMCBEENDEDIT pnmcbee)
{
    if (pnmcbee->iWhy == CBENF_RETURN)
    {
        TCHAR szTclCmd[MAX_PATH]; //what size here?
        COMBOBOXEXITEM cbexi;

        cbexi.mask          = CBEIF_TEXT;
        cbexi.iItem         = -1;
        cbexi.pszText       = szTclCmd;
        cbexi.cchTextMax    = MAX_PATH;

        ComboBoxEx_GetItem(pnmcbee->hdr.hwndFrom, &cbexi);

        MyTcl_Eval(NULL, szTclCmd, TRUE);

        MRU_Write(_T("TclCmd MRU"), szTclCmd, g_MRUSettings.nTclCmdMax);
        MRU_SetComboBoxEx(pnmcbee->hdr.hwndFrom, _T("TclCmd MRU"), g_MRUSettings.nTclCmdMax);

        ComboBox_SetText(pnmcbee->hdr.hwndFrom, "");

        return (TRUE);
    }

    return (FALSE);
}

LRESULT CALLBACK TclCmd_Cbo_Edit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CHAR:
            switch ((int)wParam)
            {
                case VK_ESCAPE:
                    SetFocus(MDI_MyGetActive(TRUE));
                break;
            }
        break;
    }

    return (CallWindowProc(lpfnDefTclCmdCboEditProc, hwnd, uMsg, wParam, lParam));
}
