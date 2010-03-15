/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : find.c
 * Created    : not known (before 07/22/99)
 * Owner      : pcppopper
 * Revised on : 07/09/00
 * Comments   : 
 *              
 *              
 ****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* resources */
#include <slack_resource.h>
#include <slack_picdef.h>

/* SlackEdit */
#include "../slack_main.h"
#include "dlg_find.h"
#include "../settings/settings.h"
#include "../subclasses/sub_pcp_edit.h"
#include "../windows/wnd_statusbar.h"

/* pcp_generic */
#include <pcp_mem.h>
#include <pcp_mru.h>
#include <pcp_point.h>
#include <pcp_rect.h>
#include <pcp_string.h>
#include <pcp_window.h>

/* pcp_controls */
#include <pcp_menu.h>
#include <pcp_statusbar.h>

/* pcp_edit */
#include <pcp_edit.h>

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

BOOL CALLBACK Find_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL Find_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void Find_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode);
static void Find_OnClose(HWND hwnd);
BOOL Find_DoHighlightText(LPEDITINTERFACE lpInterface, HWND hwnd);

void Find_ChangeMode(HWND hwnd, BOOL bMode);
void Find_GetSettings(HWND hwnd, BOOL bReplace);

static DWORD Find_GetSearchFlags(void);

static BOOL Find_Replace_Imp(LPEDITINTERFACE lpInterface);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

static BOOL s_bReplaceMode;
static POINT s_ptCurrentPos     = { 0, 0 };
static POINT s_ptBlockBegin     = { 0, 0 };
static POINT s_ptBlockEnd       = { 0, 0 };
static POINT s_ptFoundAt        = { 0, 0 };
static POINT s_ptSaveCursorPos  = { 0, 0 };
static POINT s_ptSaveSelStart   = { 0, 0 };
static POINT s_ptSaveSelEnd     = { 0, 0 };

static BOOL s_bFound = FALSE;
static BOOL s_bSelectionPushed = FALSE;

static TCHAR s_szSearchText[MAX_PATH];
static TCHAR s_szReplaceText[MAX_PATH];

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/
//remove hwnd
void Find_Edit_CreateDialog(HWND hwnd, BOOL bReplace)
{
    s_bReplaceMode = bReplace;
    DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_FIND),
                                    g_hwndMain, Find_DlgProc);
}

BOOL CALLBACK Find_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_DLG_MSG(hwndDlg, WM_INITDIALOG,  Find_OnInitDialog);
        HANDLE_DLG_MSG(hwndDlg, WM_COMMAND,     Find_OnCommand);
        HANDLE_DLG_MSG(hwndDlg, WM_CLOSE,       Find_OnClose);
    }

    return (FALSE);
}

static BOOL Find_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND hwndFindCombo  = GetDlgItem(hwnd, IDC_COMBO_FIND);
    LPEDITINTERFACE lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

    ComboBox_LimitText(hwndFindCombo, MAX_PATH);

    MRU_SetComboBox(hwndFindCombo, _T("Find MRU"), g_MRUSettings.nFindMax);

    if (PCP_Edit_IsSelection(lpInterface))
    {
        POINT ptSelStart, ptSelEnd;
        LPTSTR pszSelection = NULL;

        PCP_Edit_GetSelection(lpew, &ptSelStart, &ptSelEnd);

        if (ptSelStart.y == ptSelEnd.y)
            PCP_Edit_GetText(lpInterface, ptSelStart, ptSelEnd, &pszSelection);
        else
            pszSelection = String_Duplicate(PCP_Edit_GetLineChars(lpInterface, ptSelStart.y));

        if (String_IsSpace(pszSelection))
            ComboBox_SetCurSel(hwndFindCombo, 0);
        else
            SetWindowText(hwndFindCombo, pszSelection);

        Mem_Free(pszSelection);
    }
    else
    {
        ComboBox_SetCurSel(hwndFindCombo, 0);
    }

    if (s_bReplaceMode)
    {
        HWND hwndReplaceCombo = GetDlgItem(hwnd, IDC_COMBO_REPLACE);

        if (PCP_Edit_IsSelection(lpInterface))
        {
            UINT uDirection;

            s_ptSaveCursorPos = PCP_Edit_GetCursorPos(lpInterface);
            PCP_Edit_GetSelection(lpInterface, &s_ptSaveSelStart, &s_ptSaveSelEnd);
            s_bSelectionPushed = TRUE;

            uDirection = ((s_ptSaveSelStart.y == s_ptSaveSelEnd.y) ? IDC_RADIO_DIRECTION_DOWN : IDC_RADIO_DIRECTION_UP);

            CheckRadioButton(hwnd, IDC_RADIO_DIRECTION_UP, IDC_RADIO_DIRECTION_DOWN, uDirection);
            Window_Command(hwnd, uDirection, 1, NULL);

            s_ptBlockBegin  = s_ptSaveSelStart;
            s_ptBlockEnd    = s_ptSaveSelEnd;
        }
        else
        {
            CheckRadioButton(hwnd, IDC_RADIO_DIRECTION_UP, IDC_RADIO_DIRECTION_DOWN, IDC_RADIO_DIRECTION_DOWN);
            Window_EnableDlgItem(hwnd, IDC_RADIO_DIRECTION_UP, FALSE);

            s_ptCurrentPos = PCP_Edit_GetCursorPos(lpInterface);
        }

        ComboBox_LimitText(hwndReplaceCombo, MAX_PATH);
        MRU_SetComboBox(hwndReplaceCombo, _T("Replace MRU"), g_MRUSettings.nReplaceMax);
        ComboBox_SetCurSel(hwndReplaceCombo, 0);

        Window_SetBothIcons(hwnd, ImageList_ExtractIcon(NULL, Menu_GetImageList(TRUE), IDP_REPLACE));
    }
    else
    {
        Window_SetBothIcons(hwnd, ImageList_ExtractIcon(NULL, Menu_GetImageList(TRUE), IDP_FIND));

        CheckRadioButton(hwnd, IDC_RADIO_DIRECTION_UP, IDC_RADIO_DIRECTION_DOWN, g_FindSettings.nDirection);
    }

    Find_ChangeMode(hwnd, s_bReplaceMode);

    if (GetWindowTextLength(hwndFindCombo))
        Window_EnableDlgItem(hwnd, IDOK, TRUE);

    CheckDlgButton(hwnd, IDC_CHECK_MATCHCASE, g_FindSettings.bCase);
    CheckDlgButton(hwnd, IDC_CHECK_REGEXP, g_FindSettings.bRegExp);
    CheckDlgButton(hwnd, IDC_CHECK_WRAPSEARCH, g_FindSettings.bWrapSearch);
    
    if (g_FindSettings.bRegExp)
    {
        Window_EnableDlgItem(hwnd, IDC_CHECK_WHOLEWORD, FALSE);
        g_FindSettings.bWholeWords = FALSE;
    }
    else
    {
        CheckDlgButton(hwnd, IDC_CHECK_WHOLEWORD, g_FindSettings.bWholeWords);
    }

    return (TRUE);
}

static void Find_OnClose(HWND hwnd)
{
    RECT r;
    GetWindowRect(hwnd, &r);

    if (s_bReplaceMode)
    {
        g_WindowSettings.nReplaceXPos = r.left;
        g_WindowSettings.nReplaceYPos = r.top;
    }
    else
    {
        g_WindowSettings.nFindXPos = r.left;
        g_WindowSettings.nFindYPos = r.top;
    }

    if (s_bSelectionPushed)
    {
        LPEDITINTERFACE lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

        PCP_Edit_SetCursorPos(lpInterface, s_ptSaveCursorPos);
        PCP_Edit_SetSelection(lpInterface, s_ptSaveSelStart, s_ptSaveSelEnd);
        PCP_Edit_SetAnchor(lpInterface, s_ptSaveCursorPos);
        PCP_Edit_EnsureVisible(lpInterface, s_ptSaveCursorPos);

        s_bSelectionPushed = FALSE;
    }

    s_bFound = FALSE;

    EndDialog(hwnd, 0);
}

static void Find_OnCommand(HWND hwnd, int idCtl, HWND hwndCtl, UINT uNotifyCode)
{
    LPEDITINTERFACE lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

    switch (idCtl)
    {
    case IDCANCEL:
        Window_Close(hwnd);
    break;
    case IDOK:
    {
        Find_GetSettings(hwnd, s_bReplaceMode);

        s_ptCurrentPos = PCP_Edit_GetCursorPos(lpInterface);

        if (s_bReplaceMode)
        {
            EDITSEARCHINFO esi;

            if (!s_bFound)
            {
                s_ptFoundAt = s_ptCurrentPos;
                s_bFound = Find_DoHighlightText(lpInterface, hwnd);

                return;
            }

            PCP_Edit_GetSearchInfo(lpInterface, &esi);

            if (esi.nLastSearchLen == 0)
            {
                if (s_ptFoundAt.y + 1 < PCP_Edit_GetLineCount(lpInterface))
                {
                    s_ptFoundAt.x = 0;
                    s_ptFoundAt.y++;
                }
                else
                {
                    s_bFound = FALSE;

                    return;
                }
            }
            else
            {
                s_ptFoundAt.x += 1; // FIXME: ??
            }

            s_bFound = Find_DoHighlightText(lpInterface, hwnd);

            return;
        }

        if (!Find_FindText(hwnd, s_szSearchText, s_ptCurrentPos, TRUE))
        {
            s_ptCurrentPos.x = 0;
            s_ptCurrentPos.y = 0;

            return;
        }

        if (!s_bReplaceMode)
            Window_Close(hwnd);
    }
    break;
    case IDC_BUTTON_REPLACE:
    {
        Find_GetSettings(hwnd, s_bReplaceMode);

        if (!s_bFound)
        {
            s_ptFoundAt = s_ptCurrentPos;
            s_bFound = Find_DoHighlightText(lpInterface);

            return;
        }

        s_bFound = Find_Replace_Imp(lpInterface);

        MRU_Write(_T("Find MRU"), s_szSearchText, g_MRUSettings.nFindMax);
        MRU_Write(_T("Replace MRU"), s_szReplaceText, g_MRUSettings.nReplaceMax);
    }
    break;
    case IDC_BUTTON_REPLACEALL:
    {
        HCURSOR hOldCursor;

        SetCapture(g_hwndMain);
        hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

        Window_SetRedraw(lpew->hwnd, FALSE);

        s_ptCurrentPos.x = 0;
        s_ptCurrentPos.y = 0;

        Find_GetSettings(hwnd, s_bReplaceMode);

        if (!s_bFound)
        {
            s_ptFoundAt = s_ptCurrentPos;
            s_bFound = Find_DoHighlightText(lpInterface);
        }

        while (s_bFound)
            s_bFound = Find_Replace_Imp(lpInterface);

        Window_SetRedraw(lpew->hwnd, TRUE);
        RedrawWindow(lpew->hwnd, NULL, NULL, RDW_INVALIDATE);

        SetCursor(hOldCursor);
        ReleaseCapture();

        MRU_Write(_T("Find MRU"), s_szSearchText, g_MRUSettings.nFindMax);
        MRU_Write(_T("Replace MRU"), s_szReplaceText, g_MRUSettings.nReplaceMax);
    }
    break;
    case IDC_COMBO_FIND:
    {
        HWND hwndFindNext = GetDlgItem(hwnd, IDOK);
        
        if (uNotifyCode == CBN_EDITCHANGE)
        {
            BOOL bText = Window_GetDlgItemTextLength(hwnd, IDC_COMBO_FIND);

            if (bText)
                EnableWindow(hwndFindNext, TRUE);
            else
                EnableWindow(hwndFindNext, FALSE);
        }
        else if (uNotifyCode == CBN_SELENDOK)
        {
            EnableWindow(hwndFindNext, TRUE);
        }
    }
    break;
    case IDC_CHECK_REGEXP:
        /*
         * if conducting a regular expression search we need to disable
         * some other options since they are of no relevance to such a
         * search
         */
        if (IsDlgButtonChecked(hwnd, IDC_CHECK_REGEXP))
        {
            CheckDlgButton(hwnd, IDC_CHECK_WHOLEWORD, BST_UNCHECKED);
            Window_EnableDlgItem(hwnd, IDC_CHECK_WHOLEWORD, FALSE);
            g_FindSettings.bWholeWords = FALSE;
        }
        else
        {
            Window_EnableDlgItem(hwnd, IDC_CHECK_WHOLEWORD, TRUE);
        }
    break;
    case IDC_RADIO_DIRECTION_UP:
        if (s_bReplaceMode)
        {
            /*
             * search context == selection
             * in replace mode can't allow finding next (IDOK)
             * or replacing (IDC_BUTTON_REPLACE)
             */
            Window_EnableDlgItem(hwnd, IDOK, FALSE);
            Window_EnableDlgItem(hwnd, IDC_BUTTON_REPLACE, FALSE);
        }
    break;
    case IDC_RADIO_DIRECTION_DOWN:
        if (s_bReplaceMode)
        {
            /*
             * search context == whole file
             * in replace mode finding next (IDOK) and replacing
             * (IDC_BUTTON_REPLACE) may have been disabled by
             * settings search context to selection. Fix this by
             * re-enabling the buttons
             */
            Window_EnableDlgItem(hwnd, IDOK, TRUE);
            Window_EnableDlgItem(hwnd, IDC_BUTTON_REPLACE, TRUE);
        }
    break;
    }
}

void Find_ChangeMode(HWND hwnd, BOOL bReplace)
{
    if (bReplace)
    {
        RECT rCancel, rHelp, rReplace, rReplaceAll;
        HWND hwndCancel, hwndHelp, hwndReplace, hwndReplaceAll;
        POINT p;

        hwndCancel      = GetDlgItem(hwnd, IDCANCEL);
        hwndHelp        = GetDlgItem(hwnd, IDC_BUTTON_HELP);
        hwndReplace     = GetDlgItem(hwnd, IDC_BUTTON_REPLACE);
        hwndReplaceAll  = GetDlgItem(hwnd, IDC_BUTTON_REPLACEALL);

        GetWindowRect(hwndCancel, &rCancel);
        GetWindowRect(hwndHelp, &rHelp);
        GetWindowRect(hwndReplace, &rReplace);
        GetWindowRect(hwndReplaceAll, &rReplaceAll);

        p.x = rCancel.left;
        p.y = rCancel.top;
        ScreenToClient(hwnd, &p);
        SetWindowPos(hwndReplace, 0, p.x, p.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        p.x = rHelp.left;
        p.y = rHelp.top;
        ScreenToClient(hwnd, &p);
        SetWindowPos(hwndReplaceAll, 0, p.x, p.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        p.x = rReplace.left;
        p.y = rReplace.top;
        ScreenToClient(hwnd, &p);
        SetWindowPos(hwndCancel, 0, p.x, p.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        p.x = rReplaceAll.left;
        p.y = rReplaceAll.top;
        ScreenToClient(hwnd, &p);
        SetWindowPos(hwndHelp, 0, p.x, p.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        if (g_WindowSettings.bReplaceCentered)
        {
            Window_CenterWindow(hwnd);
        }
        else
        {
            SetWindowPos(hwnd, 0, g_WindowSettings.nReplaceXPos, g_WindowSettings.nReplaceYPos,
                    0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }

        Window_EnableDlgItem(hwnd, IDC_COMBO_REPLACE, TRUE);

        SetDlgItemText(hwnd, IDC_DIRECTION, _T("Context"));
        SetDlgItemText(hwnd, IDC_RADIO_DIRECTION_UP, _T("&Selection"));
        SetDlgItemText(hwnd, IDC_RADIO_DIRECTION_DOWN, _T("Wh&ole File"));
    }
    else
    {
        HWND hwndReplace, hwndReplaceAll, hwndReplaceTxt, hwndReplaceLbl;
        HWND hwndWholeWord, hwndCase, hwndRegExp, hwndDirection, hwndUp,
            hwndDown;
        HWND hwndWrapSearch;
        RECT r;
        POINT p;

        hwndReplace         = GetDlgItem(hwnd, IDC_BUTTON_REPLACE);
        hwndReplaceAll      = GetDlgItem(hwnd, IDC_BUTTON_REPLACEALL);
        hwndReplaceTxt      = GetDlgItem(hwnd, IDC_COMBO_REPLACE);
        hwndReplaceLbl      = GetDlgItem(hwnd, IDC_LABEL_REPLACE);

        hwndWholeWord       = GetDlgItem(hwnd, IDC_CHECK_WHOLEWORD);
        hwndCase            = GetDlgItem(hwnd, IDC_CHECK_MATCHCASE);
        hwndRegExp          = GetDlgItem(hwnd, IDC_CHECK_REGEXP);
        hwndWrapSearch      = GetDlgItem(hwnd, IDC_CHECK_WRAPSEARCH);
        hwndDirection       = GetDlgItem(hwnd, IDC_DIRECTION);
        hwndUp              = GetDlgItem(hwnd, IDC_RADIO_DIRECTION_UP);
        hwndDown            = GetDlgItem(hwnd, IDC_RADIO_DIRECTION_DOWN);

        ShowWindow(hwndReplace,    SW_HIDE);
        ShowWindow(hwndReplaceAll, SW_HIDE);
        ShowWindow(hwndReplaceTxt, SW_HIDE);
        ShowWindow(hwndReplaceLbl, SW_HIDE);

        GetWindowRect(hwndReplaceLbl, &r);
        p.x = r.left;
        p.y = r.top;
        ScreenToClient(hwnd, &p);
        SetWindowPos(hwndWholeWord, 0, p.x, p.y + 10, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        GetWindowRect(hwndWholeWord, &r);
        p.x = r.left;
        p.y = r.bottom;
        ScreenToClient(hwnd, &p);
        SetWindowPos(hwndCase, 0, p.x, p.y + 3, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        GetWindowRect(hwndCase, &r);
        p.x = r.left;
        p.y = r.bottom;
        ScreenToClient(hwnd, &p);
        SetWindowPos(hwndRegExp, 0, p.x, p.y + 3, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        GetWindowRect(hwndRegExp, &r);
        p.x = r.left;
        p.y = r.bottom;
        ScreenToClient(hwnd, &p);
        SetWindowPos(hwndWrapSearch, 0, p.x, p.y + 3, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        GetWindowRect(hwndWholeWord, &r);
        p.x = r.right;
        p.y = r.top;
        ScreenToClient(hwnd, &p);
        SetWindowPos(hwndDirection, 0, p.x + 30, p.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        GetWindowRect(hwndDirection, &r);
        p.x = r.left;
        p.y = r.top;
        ScreenToClient(hwnd, &p);
        SetWindowPos(hwndUp, 0, p.x + 9, p.y + 16, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        GetWindowRect(hwndUp, &r);
        p.x = r.left;
        p.y = r.bottom;
        ScreenToClient(hwnd, &p);
        SetWindowPos(hwndDown, 0, p.x, p.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        GetWindowRect(hwndReplaceTxt, &r);
        p.y = Rect_Height(&r);
        GetWindowRect(hwnd, &r);

        SetWindowPos(hwnd, 0, g_WindowSettings.nFindXPos, g_WindowSettings.nFindYPos,
            Rect_Width(&r), Rect_Height(&r) - (p.y + 4), SWP_NOZORDER);
        
        if (g_WindowSettings.bFindCentered)
            Window_CenterWindow(hwnd);
    }
}

void Find_GetSettings(HWND hwnd, BOOL bReplace)
{
    GetWindowText(GetDlgItem(hwnd, IDC_COMBO_FIND), s_szSearchText, MAX_PATH);

    g_FindSettings.bWholeWords  = IsDlgButtonChecked(hwnd, IDC_CHECK_WHOLEWORD);
    g_FindSettings.bCase        = IsDlgButtonChecked(hwnd, IDC_CHECK_MATCHCASE);
    g_FindSettings.bRegExp      = IsDlgButtonChecked(hwnd, IDC_CHECK_REGEXP);
    g_FindSettings.bWrapSearch  = IsDlgButtonChecked(hwnd, IDC_CHECK_WRAPSEARCH);

    if (bReplace)
    {
        if (IsDlgButtonChecked(hwnd, IDC_RADIO_DIRECTION_UP))
            g_FindSettings.nContext = IDC_RADIO_DIRECTION_UP;
        else if (IsDlgButtonChecked(hwnd, IDC_RADIO_DIRECTION_DOWN))
            g_FindSettings.nContext = IDC_RADIO_DIRECTION_DOWN;

        GetWindowText(GetDlgItem(hwnd, IDC_COMBO_REPLACE), s_szReplaceText, MAX_PATH);
    }
    else
    {
        if (IsDlgButtonChecked(hwnd, IDC_RADIO_DIRECTION_UP))
            g_FindSettings.nDirection = IDC_RADIO_DIRECTION_UP;
        else if (IsDlgButtonChecked(hwnd, IDC_RADIO_DIRECTION_DOWN))
            g_FindSettings.nDirection = IDC_RADIO_DIRECTION_DOWN;
    }
}

static DWORD Find_GetSearchFlags(void)
{
    DWORD dwFlags = 0;

    if (g_FindSettings.bCase)
        dwFlags |= PE_FIND_MATCH_CASE;
    if (g_FindSettings.bWholeWords)
        dwFlags |= PE_FIND_WHOLE_WORD;
    if (g_FindSettings.bRegExp)
        dwFlags |= PE_FIND_REG_EXP;
    if (g_FindSettings.nDirection == IDC_RADIO_DIRECTION_UP)
        dwFlags |= PE_FIND_DIRECTION_UP;
    if (g_FindSettings.bWrapSearch)
        dwFlags = PE_FIND_WRAP_SEARCH;

    return (dwFlags);
}

BOOL Find_FindText(HWND hwnd, LPCTSTR pszText, POINT ptCurrentPos, BOOL bVisual)
{
    POINT ptTextPos;
    LPEDITINTERFACE lpInterface;

    lpInterface = PCP_Edit_GetInterface(hwnd);

    if (!PCP_Edit_FindTextVisual(lpInterface, pszText, ptCurrentPos, Find_GetSearchFlags(), &ptTextPos))
    {
        if (bVisual)
        {
            TCHAR szBuffer[MAX_PATH * 2];

            _stprintf(szBuffer,
                String_LoadString((g_FindSettings.bRegExp) ?
                            IDS_FORMAT_SEARCH_FAILED_REGEXP :
                            IDS_FORMAT_SEARCH_FAILED_STRING),
                pszText);

            MessageBox(g_hwndMain, szBuffer, _T("Find"), MB_OK | MB_ICONEXCLAMATION);
        }

        return (FALSE);
    }

    if (Point_Equal(&ptCurrentPos, &ptTextPos) ||
        ((dwFlags & FIND_DIRECTION_UP) && ptTextPos.y > ptCurrentPos.y) ||
        (ptTextPos.y < ptCurrentPos.y))
    {
        if (bVisual)
        {
            STATUSBARPANE sp;
            TCHAR szStatus[MAX_PATH + 30];

            /* We seem to have no more items to find or we have wrapped
             * Better tell the user so that he doesn't keep retrying
             * without realizing this (tihi =)
             */

            _stprintf(szStatus, String_LoadString((g_FindSettings.bRegExp) ?
                                    IDS_FORMAT_SEARCH_WRAPPED_REGEXP :
                                    IDS_FORMAT_SEARCH_WRAPPED_STRING), pszText);

            INITSTRUCT(sp, FALSE);
            sp.fMask    = SF_TEXT | SF_CRFG | SF_CRBG;
            sp.pszText  = szStatus;
            sp.crBg     = RGB(0, 0, 127);
            sp.crFg     = RGB(255, 255, 255);
            sp.nPane    = 0;

            Statusbar_SetPane(g_hwndStatusbarMain, &sp);
        }

        return (FALSE);
    }
    else
    {
        if (bVisual)
        {
            STATUSBARPANE sp;
            TCHAR szStatus[MAX_PATH + 30];

            _stprintf(szStatus, String_LoadString((g_FindSettings.bRegExp) ?
                                    IDS_FORMAT_SEARCH_FOUND_REGEXP :
                                    IDS_FORMAT_SEARCH_FOUND_STRING), pszText, ptTextPos.y + 1, ptTextPos.x + 1);

            INITSTRUCT(sp, FALSE);
            sp.fMask    = SF_TEXT | SF_CRFG | SF_CRBG;
            sp.pszText  = szStatus;
            sp.crBg     = SB_DEFAULT_CRBG;
            sp.crFg     = SB_DEFAULT_CRFG;
            sp.nPane    = 0;

            Statusbar_SetPane(g_hwndStatusbarMain, &sp);
        }
    }

    MRU_Write(_T("Find MRU"), pszText, g_MRUSettings.nFindMax);

    return (TRUE);
}

static BOOL Find_Replace_Imp(LPEDITINTERFACE lpInterface)
{
    EDITSEARCHINFO esi;

    PCP_Edit_GetSearchInfo(lpInterface, &esi);

    VERIFY(PCP_Edit_FindReplaceSelection(lpInterface, s_szReplaceText, esi.dwLastSearchFlags));

    PCP_Edit_GetSearchInfo(lpInterface, &esi);

    //  Manually recalculate points
    if (g_FindSettings.nContext == IDC_RADIO_DIRECTION_UP)
    {
        if (s_ptBlockBegin.y == s_ptFoundAt.y && s_ptBlockBegin.x > s_ptFoundAt.x)
        {
            s_ptBlockBegin.x -= esi.nLastSearchLen;
            s_ptBlockBegin.x += esi.nLastReplaceLen;
        }
        if (s_ptBlockEnd.y == s_ptFoundAt.y && s_ptBlockEnd.x > s_ptFoundAt.x)
        {
            s_ptBlockEnd.x -= esi.nLastSearchLen;
            s_ptBlockEnd.x += esi.nLastReplaceLen;
        }
    }

    if (esi.nLastSearchLen == 0)
    {
        if (s_ptFoundAt.y + 1 < PCP_Edit_GetLineCount(lpInterface))
        {
            s_ptFoundAt.x = 0;
            s_ptFoundAt.y++;
        }
        else
        {
            s_bFound = FALSE;

            break;
        }
    }
    else
    {
        s_ptFoundAt = PCP_Edit_GetCursorPos(lpInterface);
    }

    return (Find_DoHighlightText(lpInterface));
}

BOOL Find_DoHighlightText(LPEDITINTERFACE lpInterface)
{
    static BOOL bFoundBefore = FALSE;
    BOOL bFound;
    DWORD dwFlags = Find_GetSearchFlags();

    if (g_FindSettings.nContext == IDC_RADIO_DIRECTION_UP)
    {
        //  Searching selection only
        bFound = PCP_Edit_FindTextInBlock(lpInterface, s_szSearchText, s_ptFoundAt, s_ptBlockBegin, s_ptBlockEnd,
            dwFlags, FALSE, &s_ptFoundAt);
    }
    else
    {
        //  Searching whole text
        bFound = PCP_Edit_FindText(lpInterface, s_szSearchText, s_ptFoundAt, dwFlags, FALSE, &s_ptFoundAt);
    }

    if (bFound)
        bFoundBefore = TRUE;

    /* warn if not found, but only if nothing has been found ever */
    /* FIXME: only warn if nothing found ever */
    if (!bFound && !bFoundBefore)
    {
        TCHAR szBuffer[MAX_PATH * 2];
        POINT ptTemp = { 0, 0 };

        _stprintf(szBuffer,
            String_LoadString((g_FindSettings.bRegExp) ?
                        IDS_FORMAT_SEARCH_FAILED_REGEXP :
                        IDS_FORMAT_SEARCH_FAILED_STRING),
            s_szSearchText);

        MessageBox(hwnd, szBuffer, _T("Find"), MB_OK | MB_ICONEXCLAMATION);

        s_ptCurrentPos = (g_FindSettings.nContext == IDC_RADIO_DIRECTION_UP) ? s_ptBlockBegin : ptTemp;

        bFoundBefore = FALSE;

        return (FALSE);
    }

    PCP_Edit_HighlightText(lpInterface, s_ptFoundAt, _tcslen(s_szSearchText), FALSE);

    return (TRUE);
}

void Find_AutoComplete(HWND hwnd)
{
    LPEDITINTERFACE lpInterface = PCP_Edit_GetInterface(hwnd);
    POINT ptCursorPos = PCP_Edit_GetCursorPos(lpInterface);
    int nLength = PCP_Edit_GetLineLength(lpInterface, ptCursorPos.y);
    LPCTSTR pszText = PCP_Edit_GetLineChars(lpInterface, ptCursorPos.y);
    LPCTSTR pszEnd = pszText + ptCursorPos.x;

    if (ptCursorPos.x > 0 && ptCursorPos.y > 0 && (nLength == ptCursorPos.x || !xisalnum(*pszEnd)) && xisalnum(pszEnd[-1]))
    {
        LPCTSTR pszBegin = pszEnd - 1;
        LPTSTR pszBuffer;
        POINT ptTextPos;
        BOOL bFound;
        EDITSEARCHINFO esi;

        while (pszBegin > pszText && xisalnum(*pszBegin))
            pszBegin--;

        if (!xisalnum(*pszBegin))
            pszBegin++;

        nLength = pszEnd - pszBegin;

        pszBuffer = (LPTSTR)Mem_AllocStr(nLength + 1 + SZ);
        *pszBuffer = _T('<');
        _tcsncpy(pszBuffer + 1, pszBegin, nLength);
        pszBuffer[nLength + 1] = _T('\0');
        
        ptCursorPos.x -= nLength;
        bFound = PCP_Edit_FindText(lpInterface, pszBuffer, ptCursorPos, PE_FIND_MATCH_CASE|PE_FIND_REG_EXP|PE_FIND_DIRECTION_UP, TRUE, &ptTextPos);

        if (!bFound)
        {
            ptCursorPos.x += nLength;
            bFound = PCP_Edit_FindText(lpInterface, pszBuffer, ptCursorPos, FIND_MATCH_CASE|FIND_REG_EXP, TRUE, &ptTextPos);
            ptCursorPos.x -= nLength;
        }

        if (bFound)
        {
            int nFound = PCP_Edit_GetLineLength(lpInterface, ptTextPos.y);
            LPTSTR psz;

            PCP_Edit_GetSearchInfo(lpInterface, &esi);

            pszText = PCP_Edit_GetLineChars(lpInterface, ptTextPos.y) + ptTextPos.x + lpInterface->iLastFindWhatLen;
            nFound -= ptTextPos.x + esi.nLastSearchLen;
            Mem_Free(pszBuffer);
            psz = pszBuffer = (LPTSTR)Mem_AllocStr(nFound + SZ);

            while (nFound-- && xisalnum(*pszText))
                *psz++ = *pszText++;

            *psz = _T('\0');

            if ((pszBuffer[0] != _T('\0')))
            {
                int x, y;

                PCP_Edit_BeginUndoGroup(lpInterface, FALSE);
                PCP_Edit_InsertText(lpInterface, ptCursorPos.y, ptCursorPos.x + nLength, pszBuffer, &y, &x, PE_ACTION_AUTOCOMPLETE);
                ptCursorPos.x = x;
                ptCursorPos.y = y;

                PCP_Edit_SetCursorPos(lpInterface, ptCursorPos);
                PCP_Edit_SetSelection(lpInterface, ptCursorPos, ptCursorPos);
                PCP_Edit_SetAnchor(lpInterface, ptCursorPos);
                PCP_Edit_EnsureVisible(lpInterface, ptCursorPos);
                PCP_Edit_FlushUndoGroup(lpInterface);
            }

            Mem_Free(pszBuffer);
        }
    }
}






            if (lpte->bIncrementalSearchForwards || lpte->bIncrementalSearchBackwards)
            {
                TextEdit_View_OnChar(lpte, nVirtKey, cRepeat, fFlags);

                return;
            }

