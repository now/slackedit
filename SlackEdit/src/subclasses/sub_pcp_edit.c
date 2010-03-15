/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pcp_edit_child.c
 * Created    : 01/24/00
 * Owner      : pcppopper
 * Revised on : 07/02/00
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
#include "sub_pcp_edit.h"
#include "../dialogs/dlg_find.h"
#include "../mdi/child_tclcommandline.h"
#include "../settings/settings.h"
#include "../windows/wnd_statusbar.h"

/* pcp_generic */
#include <pcp_clipboard.h>
#include <pcp_mdi.h>
#include <pcp_mru.h>
#include <pcp_point.h>
#include <pcp_rect.h>
#include <pcp_string.h>

/* pcp_controls */
#include <pcp_statusbar.h>
#include <pcp_menu.h>

/* pcp_edit */
#include <pcp_edit.h>

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

LRESULT CALLBACK Sub_TextEdit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Sub_TextEdit_OnPageSetup(LPEDITINTERFACE lpInterface);
void Sub_TextEdit_UpdateEditUndo(LPEDITINTERFACE lpInterface, HMENU hMenu);
void Sub_TextEdit_UpdateEditRedo(LPEDITINTERFACE lpInterface, HMENU hMenu);
void Sub_TextEdit_OnChar(LPEDITINTERFACE lpInterface, HWND hwnd, UINT nChar, UINT nRepCnt, UINT nFlags) ;
void Sub_TextEdit_OnCommand(LPEDITINTERFACE lpInterface, HWND hwnd, WORD wNotifyCode, WORD wID, HWND hwndCtl);
void Sub_TextEdit_IncrementalSearchUpdateStatusbar(LPEDITSEARCHINFO lpesi);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

WNDPROC lpfnOldWndProc;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

void Sub_TextEdit_Create(HWND hwndOwner, LPTSTR pszFile)
{
    HWND hwndEdit;
    RECT rc;

    GetClientRect(hwndOwner, &rc);

    hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, WC_PCP_TEXTEDIT, _T(""),
        WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_OVERLAPPED,
        0, 0, Rect_Width(&rc), Rect_Height(&rc),
        hwndOwner, (HMENU)IDC_EDIT_MDICHILD, g_hInstance, pszFile);

    ASSERT(hwndEdit != NULL);

    lpfnOldWndProc = SubclassWindow(hwndEdit, Sub_TextEdit_WndProc);
}

LRESULT CALLBACK Sub_TextEdit_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPEDITINTERFACE lpInterface = PCP_Edit_GetInterface(hwnd);

    switch (uMsg)
    {
    case WM_COMMAND:
        Sub_TextEdit_OnCommand(lpInterface, hwnd, (WORD)HIWORD(wParam), (WORD)LOWORD(wParam), (HWND)lParam);
    return (0);
    case WM_CHAR:
        Sub_TextEdit_OnChar(lpInterface, hwnd, (UINT)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
    break;  
    case WM_SETFOCUS:
        Sub_TextEdit_UpdateStatusbar(lpInterface);
    break;
    case WM_MYFONTCHANGE:
        PCP_Edit_SetFont(lpInterface, (LPLOGFONT)lParam);
    return (0);
    }

    return (CallWindowProc(lpfnOldWndProc, hwnd, uMsg, wParam, lParam));
}

void Sub_TextEdit_OnChar(LPEDITINTERFACE lpInterface, HWND hwnd, UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    if (PCP_Edit_IsIncrementalSearchActive(lpInterface))
    {
        EDITSEARCHINFO esi;

        if (nChar == VK_ESCAPE)
        {
            PCP_Edit_GetSearchInfo(lpInterface, &esi);

            if (!esi.bLastSearchSuccessful)
            {
                PCP_Edit_FindIncrementalRemoveChar(lpInterface);
                PCP_Edit_GetSearchInfo(lpInterface, &esi);

                if (_tcslen(esi.pszSearchString) == 0)
                {
                    PCP_Edit_FindIncrementalEnd(lpInterface, TRUE);
                    esi.bIncrementalSearchActive = FALSE;
                }
                else
                {
                    PCP_Edit_FindTextVisual(lpInterface, esi.pszSearchString);
                }

                Sub_TextEdit_IncrementalSearchUpdateStatusbar(&esi);

                return;
            }

            PCP_Edit_FindIncrementalEnd(lpInterface, TRUE);

            Sub_TextEdit_IncrementalSearchUpdateStatusbar(&esi);

            return;
        }
        else if (nChar == VK_RETURN)
        {
            PCP_Edit_FindIncrementalEnd(lpInterface, FALSE);

            Sub_TextEdit_IncrementalSearchUpdateStatusbar(&esi);

            return;
        }
        else if (nChar == VK_DELETE || nChar == VK_BACK)
        {
            PCP_Edit_FindIncrementalRemoveChar(lpInterface);
            PCP_Edit_GetSearchInfo(lpInterface, &esi);

            if (_tcslen(esi.pszSearchString) == 0)
            {
                PCP_Edit_FindIncrementalEnd(lpInterface, TRUE);
            }
            else
            {
                PCP_Edit_FindTextVisual(lpInterface, esi.pszSearchString);
            }

            Sub_TextEdit_IncrementalSearchUpdateStatusbar(&esi);

            return;
        }

        PCP_Edit_FindIncrementalAddChar(lpInterface, nChar);
        PCP_Edit_GetSearchInfo(lpInterface, &esi);
        PCP_Edit_FindTextVisual(lpInterface, esi.pszSearchString);

        Sub_TextEdit_IncrementalSearchUpdateStatusbar(&esi);

        return;
    }

    switch (nChar)
    {
    case VK_ESCAPE:
    {
        if (PCP_Edit_GetSelectionMode(lpInterface) == PE_SELECTION_MODE_NONE)
            SetFocus(GetDlgItem(GetParent(hwnd), IDC_CBO_TCL));
    }
    break;
    }
}

void Sub_TextEdit_UpdateSearchMenuInt(HWND hwnd, HMENU hMenu)
{
    LPINTERFACE lpInterface = PCP_Edit_GetInterface(hwnd);
    POINT ptCursorPos = PCP_Edit_GetCursorPos(lpInterface);
    int nLines = PCP_Edit_GetLineCount(lpInterface);
    int nChars = PCP_Edit_GetLineLength(lpInterface, ptCursorPos.y);
    EDITSEARCHINFO esi;

    PCP_Edit_GetSearchInfo(lpInterface, &esi);

    Menu_EnableMenuItem(hMenu, IDM_SEARCH_FINDNEXT, esi.bLastSearchSuccessful);
    Menu_EnableMenuItem(hMenu, IDM_SEARCH_FINDPREVIOUS, esi.bLastSearchSuccessful);
    Menu_EnableMenuItem(hMenu, IDM_SEARCH_QUICKFIND_NEXT, nChars);
    Menu_EnableMenuItem(hMenu, IDM_SEARCH_QUICKFIND_PREVIOUS, nChars);
}

void Sub_TextEdit_UpdateEditMenuInt(HWND hwnd, HMENU hMenu)
{
    int i;
    LPINTERFACE lpInterface = PCP_Edit_GetInterface(hwnd);
    POINT ptCursorPos = PCP_Edit_GetCursorPos(lpInterface);
    POINT ptLastChangePos = PCP_Edit_GetLastChangePos(lpInterface);
    POINT ptSelStart, ptSelEnd;
    int nLength = PCP_Edit_GetLineLength(lpInterface, ptCursorPos.y);
    LPCTSTR pszText = PCP_Edit_GetLineChars(lpInterface, ptCursorPos.y) + ptCursorPos.x;
    BOOL fEnabled;

    Sub_TextEdit_UpdateEditUndo(lpew, hMenu);
    Sub_TextEdit_UpdateEditRedo(lpew, hMenu);

    /* Do we have a selection? toggle items that depend on it */

    PCP_Edit_GetSelection(lpInterface, &ptSelStart, &ptSelEnd);

    fEnabled = !Point_Equal(&ptSelStart, &ptSelEnd);

    Menu_EnableMenuItem(hMenu, 3, MF_BYPOSITION | fEnabled);
    Menu_EnableMenuItem(hMenu, 4, MF_BYPOSITION | fEnabled);
    Menu_EnableMenuItem(hMenu, IDM_EDIT_DELETE, fEnabled);

    Menu_EnableMenuItem(hMenu, IDM_EDIT_PASTE, Clipboard_GetText(NULL));

    /* Do we have any bookmarks? toggle items that depend on it */
    fEnabled = lpew->bBookmarkExist;

    Menu_EnableMenuItem(hMenu, IDM_EDIT_NUMBOOKMARKS_CLEARBOOKMARKS, fEnabled);
    
    for (i = IDM_EDIT_GOTOBOOKMARKS_BOOKMARK0; i <= IDM_EDIT_GOTOBOOKMARKS_BOOKMARK9; i++)
        Menu_EnableMenuItem(hMenu, i, fEnabled);
    
    Menu_EnableMenuItem(hMenu, IDM_EDIT_BOOKMARKS_NEXTBOOKMARK, fEnabled);
    Menu_EnableMenuItem(hMenu, IDM_EDIT_BOOKMARKS_PREVIOUSBOOKMARK, fEnabled);
    Menu_EnableMenuItem(hMenu, IDM_EDIT_BOOKMARKS_CLEARALLBOOKMARKS, fEnabled);

    Menu_EnableMenuItem(hMenu, IDM_EDIT_AUTOCOMPLETE, (ptCursorPos.x > 0 && ptCursorPos.y > 0 && (nLength == ptCursorPos.x || !xisalnum(*pszText)) && xisalnum(pszText[-1])));
    Menu_EnableMenuItem(hMenu, IDM_EDIT_MATCHBRACE, (ptCursorPos.x < nLength && (bracetype(*pszText) || ptCursorPos.x > 0 && bracetype(pszText[-1])) || ptCursorPos.x > 0 && bracetype(pszText[-1])));
    Menu_EnableMenuItem(hMenu, IDM_EDIT_GOTOLASTCHANGE, (ptLastChangePos.x > 0 && ptLastChangePos.y > -1));
}

void Sub_TextEdit_UpdateEditUndo(LPEDITINTERFACE lpInterface, HMENU hMenu)
{
    TCHAR szText[MAX_PATH];
    BOOL bCanUndo = PCP_Edit_CanUndo(lpInterface);
    MENUITEMINFO mii;
    TCHAR szMenuText[MAX_PATH] = _T("");

    if (bCanUndo)
    {
        TCHAR szDesc[MAX_PATH];

        PCP_Edit_GetUndoDescription(lpInterface, szDesc, 0);
        _stprintf(szText, String_LoadString(IDS_MENU_UNDO_FORMAT), szDesc);
    }
    else
    {
        _tcscpy(szText, String_LoadString(IDS_MENU_UNDO_DEFAULT));
    }

    INITSTRUCT(mii, TRUE);
    mii.fMask       = MIIM_DATA;
    mii.dwTypeData  = szMenuText;
    mii.cch         = MAX_PATH;

    if (!GetMenuItemInfo(hMenu, IDM_EDIT_UNDO, FALSE, &mii) || (mii.fType & MFT_STRING) != MFT_STRING)
        return;

    if (String_CountSubChars(szMenuText, _T('\t')) > 0)
        _tcscpy(szText, _tcschr(szMenuText, _T('\t')));

    mii.dwTypeData = szText;

    SetMenuItemInfo(hMenu, IDM_EDIT_UNDO, FALSE, &mii);
    Menu_EnableMenuItem(hMenu, IDM_EDIT_UNDO, bCanUndo);
}

void Sub_TextEdit_UpdateEditRedo(LPEDITINTERFACE lpInterface, HMENU hMenu)
{
    TCHAR szText[MAX_PATH];
    BOOL bCanRedo = PCP_Edit_CanRedo(lpInterface);
    MENUITEMINFO mii;
    TCHAR szMenuText[MAX_PATH];

    if (bCanRedo)
    {
        TCHAR szDesc[MAX_PATH];

        PCP_Edit_GetRedoDescription(lpInterfaec, szDesc, 0);
        _stprintf(szText, String_LoadString(IDS_MENU_REDO_FORMAT), szDesc);
    }
    else
    {
        _tcscpy(szText, String_LoadString(IDS_MENU_REDO_DEFAULT));
    }

    INITSTRUCT(mii, TRUE);
    mii.fMask       = MIIM_DATA;
    mii.dwTypeData  = szMenuText;
    mii.cch         = MAX_PATH;

    if (!GetMenuItemInfo(hMenu, IDM_EDIT_REDO, FALSE, &mii) || (mii.fType & MFT_STRING) != MFT_STRING)
        return;

    if (String_CountSubChars(szMenuText, _T('\t')) > 0)
        _tcscpy(szText, _tcschr(szMenuText, _T('\t')));

    mii.dwTypeData = szText;

    SetMenuItemInfo(hMenu, IDM_EDIT_REDO, FALSE, &mii);
    Menu_EnableMenuItem(hMenu, IDM_EDIT_REDO, bCanRedo);
}

void Sub_TextEdit_OnCommand(LPEDITINTERFACE lpInterface, HWND hwnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    switch (wID)
    {
        case IDM_FILE_PAGESETUP:
            Sub_TextEdit_OnPageSetup(lpInterface);
        return;
        case IDM_FILE_READONLY:
        {
            FILEINFO fi;
            DWORD dwFileAttributes;

            PCP_Edit_SetReadOnly(lpInterface, !PCP_Edit_GetReadOnly(lpInterface));
            EditChild_UpdateStatusbarInfo(GetParent(hwnd));

            Main_GetFileInfo(lpew->hwnd, &fi);
            dwFileAttributes = GetFileAttributes(fi.szFileName);

            if (!(fi.dwFileType & FI_TYPE_MASK_EDIT) && dwFileAttributes != (DWORD)-1)
            {
                if (TextBuffer_GetReadOnly(lpew->lpes))
                    dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
                else
                    dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;

                SetFileAttributes(fi.szFileName, dwFileAttributes);
            }
        }
        return;
        case IDM_EDIT_UNDO:
            PCP_Edit_Undo(lpInterface);
        return;
        case IDM_EDIT_REDO:
            PCP_Edit_Redo(lpInterface);
        return;
        case IDM_EDIT_SELECTALL:
            PCP_Edit_SetSelectionPos(lpInterface, 0, -1);
        return;
        case IDM_EDIT_NUMBOOKMARKS_CLEARBOOKMARKS:
            PCP_Edit_ClearAllBookmarks(lpInterface);
        return;
        case IDM_EDIT_BOOKMARKS_TOGGLEBOOKMARK:
            PCP_Edit_ToggleBookmark(lpInterface);
        return;
        case IDM_EDIT_BOOKMARKS_NEXTBOOKMARK:
            PCP_Edit_NextBookmark(lpInterface);
        return;
        case IDM_EDIT_BOOKMARKS_PREVIOUSBOOKMARK:
            PCP_Edit_PreviousBookmark(lpInterface);
        return;
        case IDM_EDIT_BOOKMARKS_CLEARALLBOOKMARKS:
            PCP_TextEdit_ClearNumBookmarks(lpInterface);
        return;
        case IDM_EDIT_AUTOCOMPLETE:
            PCP_TextEdit_AutoComplete(lpInterface);
        return;
        case IDM_EDIT_MATCHBRACE:
        {
            POINT ptFoundPos;
            
            ptFoundPos = PCP_TextEdit_MatchBrace(lpInterface, PCP_Edit_GetCursorPos(lpInterface));

            if (ptFoundPos.x != -1 && ptFoundPos.y != -1)
            {
                PCP_Edit_SetCursorPos(lpInterface, ptFoundPos);
                PCP_Edit_SetSelection(lpInterface, ptFoundPos, ptFoundPos);
                PCP_Edit_SetAnchor(lpInterface, ptFoundPos);
                PCP_Edit_EnsureVisible(lpInterface, ptFoundPos);
            }
        }
        return;
        case IDM_EDIT_GOTOLASTCHANGE:
            Sub_TextEdit_GotoLastChange(lpInterface);
        return;
        case IDM_EDIT_INDENT:
            PCP_TextEdit_Tab(lpInterface);
        return;
        case IDM_EDIT_UNINDENT:
            PCP_TextEdit_UnTab(lpInterface);
        return;
        case IDM_SEARCH_FINDNEXT:
            if (!PCP_Edit_FindNext(lpInterface))
            {
                TCHAR szBuffer[MAX_PATH * 2];

                _stprintf(szBuffer, String_LoadString(g_FindSettings.bRegExp ? IDS_FORMAT_SEARCH_FAILED_REGEXP : IDS_FORMAT_SEARCH_FAILED_STRING), lpew->pszLastFindWhat);
                MessageBox(g_hwndMain, szBuffer, _T("Find"), MB_OK | MB_ICONEXCLAMATION);
            }
        return;
        case IDM_SEARCH_FINDPREVIOUS:
            if (!PCP_Edit_FindPrevious(lpInterface))
            {
                TCHAR szBuffer[MAX_PATH * 2];

                _stprintf(szBuffer, String_LoadString(g_FindSettings.bRegExp ? IDS_FORMAT_SEARCH_FAILED_REGEXP : IDS_FORMAT_SEARCH_FAILED_STRING), lpew->pszLastFindWhat);
                MessageBox(g_hwndMain, szBuffer, _T("Find"), MB_OK | MB_ICONEXCLAMATION);
            }
        return;
        case IDM_SEARCH_QUICKFIND_NEXT:
        {
            EDITSEARCHINFO esi;

            if (!PCP_Edit_FindSelection(hwnd, TRUE))
            {
                Find_Edit_CreateDialog(g_hwndMain, FALSE);

                return;
            }

            PCP_Edit_GetSearchInfo(lpInterface, &esi);

            MRU_Write(_T("Find MRU"), esi.pszSearchString, g_MRUSettings.nFindMax);
        }
        return;
        case IDM_SEARCH_QUICKFIND_PREVIOUS:
        {
            EDITSEARCHINFO esi;

            if (!PCP_Edit_FindSelection(hwnd, FALSE))
            {
                Find_Edit_CreateDialog(g_hwndMain, FALSE);

                return;
            }

            PCP_Edit_GetSearchInfo(lpInterface, &esi);

            MRU_Write(_T("Find MRU"), esi.pszLastSearch, g_MRUSettings.nFindMax);
        }
        return;
        case IDM_SEARCH_INCREMENTALFORWARD:
            PCP_Edit_FindIncrementalStart(lpInterface, TRUE);
        return;
        case IDM_SEARCH_INCREMENTALBACKWARD:
            PCP_EditFindIncrementalStart(lpInterface, FALSE);
        return;
        case IDA_SELECT_CHARMODE:
            if (PCP_Edit_GetSelectionMode(lpInterface) != PE_SELECTION_MODE_NONE)
                PCP_Edit_SetSelectionMode(lpInterface, PE_SELECTION_MODE_CHAR);
            else
                PCP_Edit_SetSelectionMode(lpInterface, PE_SELECTION_MODE_NONE);
        return;
        case IDM_VIEW_LINEFEEDMODE_DOS:
            EditChild_SetLineFeed(hwnd, CRLF_STYLE_DOS);
        return;
        case IDM_VIEW_LINEFEEDMODE_UNIX:
            EditChild_SetLineFeed(hwnd, CRLF_STYLE_UNIX);
        return;
        case IDM_VIEW_LINEFEEDMODE_MAC:
            EditChild_SetLineFeed(hwnd, CRLF_STYLE_MAC);
        return;
    }

    if (wID >= IDM_EDIT_GOTOBOOKMARKS_BOOKMARK0 && wID <= IDM_EDIT_GOTOBOOKMARKS_BOOKMARK9)
        PCP_TextEdit_GotoNumBookmark(lpInterface, wID - IDM_EDIT_GOTOBOOKMARKS_BOOKMARK0);
    else if (wID >= IDM_EDIT_NUMBOOKMARKS_BOOKMARK0 && wID <= IDM_EDIT_NUMBOOKMARKS_BOOKMARK9)
        PCP_TextEdit_ToggleBookmarkNum(lpInterface, wID - IDM_EDIT_GOTOBOOKMARKS_BOOKMARK0);
}

void Sub_TextEdit_IncrementalSearchUpdateStatusbar(LPEDITSEARCHINFO lpesi)
{
    static BOOL bUpdatedAtLastCall = FALSE;
    TCHAR szText[SB_MAX_TEXT];

    ASSERT(IsWindow(g_hwndStatusbarMain));

    if (!lpesi->bIncrementalSearchActive)
    {
        if (bUpdatedAtLastCall)
            MyStatusbar_Ready(g_hwndStatusbarMain);

        bUpdatedAtLastCall = FALSE;

        return;
    }

    if (!lpesi->bLastSearchSuccessful)
        _stprintf(szText, String_LoadString(IDS_INCREMENTALSEARCH_FAILING), lpesi->pszSearchString);
    else if (lpesi->dwLastSearchFlags & PE_FIND_DIRECTION_UP)
        _stprintf(szText, String_LoadString(IDS_INCREMENTALSEARCH_REVERSE), lpesi->pszSearchString);
    else
        _stprintf(szText, String_LoadString(IDS_INCREMENTALSEARCH_FORWARD), lpesi->pszSearchString);

    Statusbar_SetPaneText(g_hwndStatusbarMain, 0, szText);
    bUpdatedAtLastCall = TRUE;
}

void Sub_TextEdit_OnPageSetup(LPEDITINTERFACE lpInterface)
{
    PAGESETUPDLG    psd;

    INITSTRUCT(psd, TRUE);
    psd.hwndOwner       = g_hwndMain;
    psd.Flags           = PSD_INHUNDREDTHSOFMILLIMETERS | PSD_DISABLEORIENTATION | PSD_DISABLEPAPER | PSD_MARGINS;
    psd.rtMargin.bottom = g_PrintSettings.uMarginBottom;
    psd.rtMargin.left   = g_PrintSettings.uMarginLeft;
    psd.rtMargin.right  = g_PrintSettings.uMarginRight;
    psd.rtMargin.top    = g_PrintSettings.uMarginTop;

    if (PageSetupDlg(&psd))
    {
        g_PrintSettings.uMarginBottom   = psd.rtMargin.bottom;
        g_PrintSettings.uMarginLeft     = psd.rtMargin.left;
        g_PrintSettings.uMarginRight    = psd.rtMargin.right;
        g_PrintSettings.uMarginTop      = psd.rtMargin.top;
    }
}

void Sub_TextEdit_UpdateStatusbar(LPEDITINTERFACE lpInterface)
{
    TCHAR szPosInfo[64];
    int cLineCount, nCurrentLine, nLineLength, nCurrentColumn;
    POINT ptCursorPos;
    int nStart, nEnd;

    ptCursorPos = PCP_Edit_GetCursorPos(lpInterface);
    PCP_Edit_GetSelectionPos(lpInterface, &nStart, &nEnd);

    cLineCount      = PCP_Edit_GetLineCount(lpInterface);
    nCurrentLine    = ptCursorPos.y + 1;
    nCurrentColumn  = PCP_Edit_CalculateActualOffset(lpInterface, ptCursorPos.y, ptCursorPos.x) + 1;
    nLineLength     = PCP_Edit_GetLineActualLength(lpInterface, ptCursorPos.y) + 1;

    //Percentage
//  iChar = ((iChar * 100) / (cChar + 1));
    if (nStart == nEnd)
    {
        _stprintf(szPosInfo, _T("Ln %d/%d, Col %d/%d"), nCurrentLine, cLineCount,
            nCurrentColumn, nLineLength);
    }
    else
    {
        _stprintf(szPosInfo, _T("Ln %d/%d, Col %d/%d Sel %d"), nCurrentLine, cLineCount,
            nCurrentColumn, nLineLength, (dwSelEnd - dwSelStart));
    }

    Statusbar_SetPaneText(g_hwndStatusbarMain, 1, szPosInfo);
}

void EditChild_SetLineFeed(HWND hwnd, int nType)
{
    HMENU           hSubMenu;
    UINT            uCheckItem;
    LPTSTR          pszCRLFMode;
    LPEDITINTERFACE lpInterface;

    if (!IsWindow(hwnd))
        return;

    lpInterface = PCP_Edit_GetInterface(hwnd);

    hSubMenu    = Main_GetMainSubMenu(g_hwndMain, IDM_MAIN_VIEW);

    ASSERT(nType >= CRLF_STYLE_DOS && nType <= CRLF_STYLE_MAC);

    PCP_TextEdit_SetCRLFMode(lpInterface, nType);
    nType = PCP_TextEdit_GetCRLFMode(lpew->lpes);

    switch (nType)
    {
    case CRLF_STYLE_DOS:
        uCheckItem  = IDM_VIEW_LINEFEEDMODE_DOS;
        pszCRLFMode = String_LoadString(IDS_STATUSBARPANE_DOS);
    break;
    case CRLF_STYLE_UNIX:
        uCheckItem  = IDM_VIEW_LINEFEEDMODE_UNIX;
        pszCRLFMode = String_LoadString(IDS_STATUSBARPANE_UNIX);
    break;
    case CRLF_STYLE_MAC:
        uCheckItem  = IDM_VIEW_LINEFEEDMODE_MAC;
        pszCRLFMode = String_LoadString(IDS_STATUSBARPANE_MAC);
    break;
    }

    Statusbar_SetPaneText(g_hwndStatusbarMain, 2, pszCRLFMode);
    CheckMenuRadioItem(hSubMenu, IDM_VIEW_LINEFEEDMODE_DOS, IDM_VIEW_LINEFEEDMODE_MAC, uCheckItem, MF_BYCOMMAND);
}

void EditChild_UpdateStatusbarInfo(HWND hwnd)
{
    HMENU hMenu;
    BOOL bReadOnly;
    int nCRLFMode;
    STATUSBARPANE sp;
    LPEDITINTERFACE lpInterface;

    lpInterface = PCP_Edit_GetInterface(hwnd);

    hMenu   = Main_GetMainSubMenu(g_hwndMain, IDM_MAIN_FILE);

    bReadOnly = PCP_Edit_GetReadOnly(lpInterface);
    nCRLFMode = PCP_TextEdit_GetCRLFMode(lpew->lpes);

    Menu_CheckMenuItem(hMenu, IDM_FILE_READONLY, bReadOnly);
    
    sp.fMask = SF_CRFG;
    sp.nPane = 4;

    if (bReadOnly)
        sp.crFg = SB_DEFAULT_CRFG;
    else
        sp.crFg = GetSysColor(COLOR_GRAYTEXT);

    Statusbar_SetPane(g_hwndStatusbarMain, &sp);

    EditChild_SetLineFeed(hwnd, nCRLFMode);
}

void Sub_TextEdit_GotoLastChange(LPEDITINTERFACE lpInterface)
{
    POINT ptLastChangePos = PCP_Edit_GetLastChangePos(lpInterface);

    if (ptLastChangePos.x < 0 || ptLastChangePos.y < 0)
        return;

    PCP_Edit_SetCursorPos(lpInterface, ptLastChangePos);
    PCP_Edit_SetSelection(lpInterface, ptLastChangePos, ptLastChangePos);
    PCP_Edit_EnsureVisible(lpInterface, ptLastChangePos);
}
