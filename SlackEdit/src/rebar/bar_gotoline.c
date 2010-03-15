/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : gotoline.c
 * Created    : not known (before 06/22/00)
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

/* SlackEdit */
#include "../slack_main.h"
#include "bar_gotoline.h"
#include "bar_main.h"
#include "../settings/settings.h"

/* pcp_generic */
#include <pcp_combobox.h>
#include <pcp_editctrl.h>
#include <pcp_mdi.h>
#include <pcp_mru.h>
#include <pcp_rebar.h>
#include <pcp_rect.h>
#include <pcp_string.h>

#define IDC_CBO_EDIT     0x00000440

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

static BOOL CALLBACK GL_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL GL_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void GL_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

HWND GotoLine_Create(HWND hwndOwner)
{
    HWND hwnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_GOTOLINE),
                        g_hwndRebarMain, GL_DlgProc);

    if (MDI_GetCount(g_hwndMDIClient) == 0)
        EnableWindow(hwnd, FALSE);

    return (hwnd);
}

HWND GotoLine_Destroy(HWND hwnd)
{
    if (EndDialog(hwnd, 0))
        return (NULL);
    else
        return (hwnd);
}

static BOOL CALLBACK GL_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_DLG_MSG(hwndDlg, WM_INITDIALOG,  GL_OnInitDialog);
        HANDLE_DLG_MSG(hwndDlg, WM_COMMAND,     GL_OnCommand);
    }
    return (FALSE);
}

static BOOL GL_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    RECT rc;
    HWND hwndComboEdit;

    GetWindowRect(hwnd, &rc);

    Rebar_MyInsertBand(String_LoadString(IDS_REBARTITLE_GOTOLINE), hwnd,
                IDC_DLG_GOTOLINE, Rect_Width(&rc), Rect_Height(&rc));

    hwndComboEdit = ComboBoxEx_GetEditControl(GetDlgItem(hwnd, IDC_COMBO_LINE));

    Edit_LimitText(hwndComboEdit, 10);
    SetWindowLong(hwndComboEdit, GWL_STYLE, GetWindowLong(hwndComboEdit, GWL_STYLE) | ES_NUMBER);

    MRU_SetComboBoxEx(GetDlgItem(hwnd, IDC_COMBO_LINE), _T("Line MRU"), g_MRUSettings.nLinesMax);

    return (TRUE);
}

static void GL_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode)
{
    switch (idCtl)
    {
        case IDOK:
        {
            int nLine = GetDlgItemInt(hwnd, IDC_COMBO_LINE, NULL, FALSE);
            HWND hwndEdit = MDI_MyGetActive(TRUE);
            TCHAR szBuffer[MAX_PATH];
            LPEDITINTERFACE lpInterface;

            lpInterface = PCP_Edit_GetInterface(hwndEdit);

            if (!PCP_Edit_GotoLine(lpInterface, nLine, IsDlgButtonChecked(hwnd, IDC_CHECK_EXTENDSELECTION)))
            {
                _stprintf(szBuffer, String_LoadString(IDS_FORMAT_GOTOLINE_LINEINDEXTOOLARGE), EditView_GetLineCount(hwndEdit));
                MessageBox(hwnd, szBuffer, _T("Oops"), MB_OK);

                SetFocus(hwnd);

                return;
            }
            else
            {
                _ltot(nLine, szBuffer, 10);

                MRU_Write(_T("Line MRU"), szBuffer, g_MRUSettings.nLinesMax);

                SetFocus(hwndEdit);
            }
        }
        break; //??? this nice ???
        //Fallthrough
        case IDCANCEL:
            Rebar_DeleteBand(g_hwndRebarMain, Rebar_IDToIndex(g_hwndRebarMain, IDC_DLG_GOTOLINE));
            g_hwndDlgGotoline = NULL;
            EndDialog(hwnd, 0);
        break;
    }
}
