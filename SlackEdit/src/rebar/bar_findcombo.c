/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : findcombo.c
 * Created    : not known (before 06/22/00)
 * Owner      : pcppopper
 * Revised on : 07/05/00
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
#include "bar_toolbar.h"
#include "bar_main.h"
#include "../dialogs/dlg_find.h"
#include "../settings/settings.h"
#include "../subclasses/sub_pcp_edit.h"

/* pcp_generic */
#include <pcp_combobox.h>
#include <pcp_mdi.h>
#include <pcp_mem.h>
#include <pcp_mru.h>
#include <pcp_rebar.h>
#include <pcp_string.h>

/* pcp_edit */
#include <pcp_edit.h>

LRESULT CALLBACK QuickSearch_Edit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

WNDPROC lpfnDefQuickSearchEditProc;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

HWND FindCombo_Create(HWND hwndParent)
{
    HWND hwndFindCombo = CreateWindowEx(0, WC_COMBOBOXEX, NULL,
                                WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL |
                                WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                                CCS_NORESIZE | CCS_NODIVIDER |
                                CCS_NOMOVEY | CBS_AUTOHSCROLL | CBS_DROPDOWN,
                                0, 0, 150, 160,
                                hwndParent, (HMENU)IDC_COMBO_QUICKSEARCH, g_hInstance, NULL);

    MRU_SetComboBoxEx(hwndFindCombo, _T("Find MRU"), g_MRUSettings.nFindMax);

    Rebar_MyInsertBand(String_LoadString(IDS_REBARTITLE_QUICKSEARCH), hwndFindCombo,
                    IDC_COMBO_QUICKSEARCH, 40, TOOLBARHEIGHT);

    if (MDI_GetCount(g_hwndMDIClient) == 0)
        EnableWindow(hwndFindCombo, FALSE);

    lpfnDefQuickSearchEditProc = SubclassWindow(ComboBoxEx_GetEditControl(hwndFindCombo), QuickSearch_Edit_WndProc);

    return (hwndFindCombo);
}

HWND FindCombo_Destroy(HWND hwndCombo)
{
    if (DestroyWindow(hwndCombo))
        return (NULL);
    else
        return (hwndCombo);
}

LRESULT CALLBACK QuickSearch_Edit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

    return (CallWindowProc(lpfnDefQuickSearchEditProc, hwnd, uMsg, wParam, lParam));
}

void QuickSearch_EndEdit(PNMCBEENDEDIT pnmcb)
{
    POINT ptCurrentPos;
    TCHAR szSearchText[MAX_PATH];
    LPEDITINTERFACE lpInterface;

    lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

    ptCurrentPos = PCP_Edit_GetCursorPos(lpInterface);
    GetWindowText(pnmcb->hdr.hwndFrom, szSearchText, MAX_PATH);

    Find_FindText(hwnd, szSearchText, ptCurrentPos, TRUE);
}
