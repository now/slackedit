/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : tcl_hotkey.c
 * Created    : 07/16/00
 * Owner      : pcppopper
 * Revised on : 07/16/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* Tcl */
#include <tcl.h>

/* SlackEdit */
#include <slack_resource.h>

#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "tcl_hotkey.h"
#include "tcl_main.h"

/* pcp_generic */
#include <pcp_mem.h>
#include <pcp_string.h>

/****************************************************************
 * Type Definitions
 ****************************************************************/

struct tagTCLSTRINGANDCOMMANDVALUE
{
    LPTSTR  pszName;
    UINT    uCommand;
} s_tclStringAndCommandValues[] =
{
    { _T("file_new"),               IDM_FILE_NEW },
    { _T("file_open"),              IDM_FILE_OPEN },
    { _T("file_advanced_open"),     IDM_FILE_ADVANCEDOPEN },
    { _T("file_open_favorites"),    IDM_FILE_FAVORITEOPEN },
    { _T("file_save"),              IDM_FILE_SAVE },
    { _T("file_save_as"),           IDM_FILE_SAVEAS },
    { _T("file_save_all"),          IDM_FILE_SAVEALL },
    { _T("file_read_only"),         IDM_FILE_READONLY },
    { _T("file_print"),             IDM_FILE_PRINT },
    { _T("slackedit_close"),        IDM_FILE_CLOSE },
    { _T("slackedit_exit"),         IDM_FILE_EXIT },
    { _T("edit_undo"),              IDM_EDIT_UNDO },
    { _T("edit_redo"),              IDM_EDIT_REDO },
    { _T("edit_cut"),               IDM_EDIT_CUT },
    { _T("edit_copy"),              IDM_EDIT_COPY },
    { _T("edit_paste"),             IDM_EDIT_PASTE },
    { _T("edit_delete"),            IDM_EDIT_DELETE },
    { _T("edit_select_all"),        IDM_EDIT_SELECTALL },
    { _T("bookmarks_clear"),        IDM_EDIT_NUMBOOKMARKS_CLEARBOOKMARKS },
    { _T("bookmarks_toggle_1"),     IDM_EDIT_NUMBOOKMARKS_BOOKMARK1 },
    { _T("bookmarks_toggle_2"),     IDM_EDIT_NUMBOOKMARKS_BOOKMARK2 },
    { _T("bookmarks_toggle_3"),     IDM_EDIT_NUMBOOKMARKS_BOOKMARK3 },
    { _T("bookmarks_toggle_4"),     IDM_EDIT_NUMBOOKMARKS_BOOKMARK4 },
    { _T("bookmarks_toggle_5"),     IDM_EDIT_NUMBOOKMARKS_BOOKMARK5 },
    { _T("bookmarks_toggle_6"),     IDM_EDIT_NUMBOOKMARKS_BOOKMARK6 },
    { _T("bookmarks_toggle_7"),     IDM_EDIT_NUMBOOKMARKS_BOOKMARK7 },
    { _T("bookmarks_toggle_8"),     IDM_EDIT_NUMBOOKMARKS_BOOKMARK8 },
    { _T("bookmarks_toggle_9"),     IDM_EDIT_NUMBOOKMARKS_BOOKMARK9 },
    { _T("bookmarks_toggle_0"),     IDM_EDIT_NUMBOOKMARKS_BOOKMARK0 },
    { _T("bookmarks_goto_1"),       IDM_EDIT_GOTOBOOKMARKS_BOOKMARK1 },
    { _T("bookmarks_goto_2"),       IDM_EDIT_GOTOBOOKMARKS_BOOKMARK2 },
    { _T("bookmarks_goto_3"),       IDM_EDIT_GOTOBOOKMARKS_BOOKMARK3 },
    { _T("bookmarks_goto_4"),       IDM_EDIT_GOTOBOOKMARKS_BOOKMARK4 },
    { _T("bookmarks_goto_5"),       IDM_EDIT_GOTOBOOKMARKS_BOOKMARK5 },
    { _T("bookmarks_goto_6"),       IDM_EDIT_GOTOBOOKMARKS_BOOKMARK6 },
    { _T("bookmarks_goto_7"),       IDM_EDIT_GOTOBOOKMARKS_BOOKMARK7 },
    { _T("bookmarks_goto_8"),       IDM_EDIT_GOTOBOOKMARKS_BOOKMARK8 },
    { _T("bookmarks_goto_9"),       IDM_EDIT_GOTOBOOKMARKS_BOOKMARK9 },
    { _T("bookmarks_goto_0"),       IDM_EDIT_GOTOBOOKMARKS_BOOKMARK0 },
    { _T("toggle_bookmark"),        IDM_EDIT_BOOKMARKS_TOGGLEBOOKMARK },
    { _T("next_bookmark"),          IDM_EDIT_BOOKMARKS_NEXTBOOKMARK },
    { _T("previous_bookmark"),      IDM_EDIT_BOOKMARKS_PREVIOUSBOOKMARK },
    { _T("clear_all_bookmarks"),    IDM_EDIT_BOOKMARKS_CLEARALLBOOKMARKS },
    { _T("auto_complete"),          IDM_EDIT_AUTOCOMPLETE },
    { _T("match_brace"),            IDM_EDIT_MATCHBRACE },
    { _T("goto_last_change"),       IDM_EDIT_GOTOLASTCHANGE },
    { _T("edit_indent"),            IDM_EDIT_INDENT },
    { _T("edit_unindent"),          IDM_EDIT_UNINDENT },
    { _T("find"),                   IDM_SEARCH_FIND },
    { _T("find_next"),              IDM_SEARCH_FINDNEXT },
    { _T("find_previous"),          IDM_SEARCH_FINDPREVIOUS },
    { _T("replace"),                IDM_SEARCH_REPLACE },
    { _T("quickfind_next"),         IDM_SEARCH_QUICKFIND_NEXT },
    { _T("quickfind_previous"),     IDM_SEARCH_QUICKFIND_PREVIOUS },
    { _T("incremental_forward"),    IDM_SEARCH_INCREMENTALFORWARD },
    { _T("incremental_reverse"),    IDM_SEARCH_INCREMENTALBACKWARD },
    { _T("view_clipbook"),          IDM_VIEW_CHILDREN_CLIPBOOK },
    { _T("view_outputwindow"),      IDM_VIEW_CHILDREN_OUTPUTWINDOW },
    { _T("view_statusbar"),         IDM_VIEW_CHILDREN_STATUSBAR },
    { _T("view_windowbar"),         IDM_VIEW_CHILDREN_WINDOWBAR },
    { _T("view_toolbar"),           IDM_VIEW_REBARITEMS_TOOLBAR },
    { _T("view_quicksearch"),       IDM_VIEW_REBARITEMS_QUICKSEARCH },
    { _T("view_gotoline"),          IDM_VIEW_REBARITEMS_GOTOLINE },
    { _T("view_fullscreen"),        IDM_VIEW_FULLSCREEN },
    { _T("linefeed_dos"),           IDM_VIEW_LINEFEEDMODE_DOS },
    { _T("linefeed_unix"),          IDM_VIEW_LINEFEEDMODE_UNIX },
    { _T("linefeed_mac"),           IDM_VIEW_LINEFEEDMODE_MAC },
    { _T("settings_font"),          IDM_OPTIONS_SETFONT },
    { _T("settings"),               IDM_OPTIONS_SETTINGS },
    { _T("window_close"),           IDM_WINDOW_CLOSE },
    { _T("window_close_all"),       IDM_WINDOW_CLOSEALL },
    { _T("window_next"),            IDM_WINDOW_NEXT },
    { _T("window_previous"),        IDM_WINDOW_PREVIOUS },
    { _T("window_cascade"),         IDM_WINDOW_CASCADE },
    { _T("window_tile_horz"),       IDM_WINDOW_TILEHORIZONTAL },
    { _T("window_tile_vert"),       IDM_WINDOW_TILEVERTICAL },
    { _T("window_windowlist"),      IDM_WINDOW_WINDOWLIST },
    { _T("help_about"),             IDM_HELP_ABOUT },
    { _T("help_tipofday"),          IDM_HELP_TIPOFDADAY },
    { _T("activate_mdiclient"),     IDA_ACTIVATE_MDICLIENT },
    { _T("activate_outputwindow"),  IDA_ACTIVATE_OUTPUTWINDOW },
    { _T("activate_clipbook"),      IDA_ACTIVATE_CLIPBOOK },
    { _T("activate_quicksearch"),   IDA_ACTIVATE_QUICKSEARCH },
    { _T("activate_next"),          IDA_ACTIVATE_NEXT },
    { _T("activate_previous"),      IDA_ACTIVATE_PREV },
    { _T("select_char_mode"),       IDA_SELECT_CHARMODE },
    { (LPTSTR)NULL,                 (UINT)-1 },
};

/****************************************************************
 * Function Definitions
 ****************************************************************/

static BOOL MyTcl_HandleCommandAccelerator(HWND hwnd, PTCLHOTKEY pHotkey, LPMSG lpMsg);
static BOOL MyTcl_Hotkey_AddHotkeyDescription(UINT uCmd, LPCTSTR pszDescription);
static BOOL MyTcl_Hotkey_LookupCommandValue(UINT uCmd);

static LPTSTR MyTcl_Hotkey_CommandValuesArrayTraceProc(ClientData clientData, Tcl_Interp *TclInterp, LPTSTR pszName1, LPTSTR pszName2, int nFlags);

/****************************************************************
 * Global Variables
 ****************************************************************/

static LPCTSTR s_pszCmdArrayName = _T("slack::cmds");

/****************************************************************
 * Function Implementations
 ****************************************************************/

BOOL MyTcl_Hotkey_AddCommandValues(Tcl_Interp *TclInterp)
{
    Tcl_Obj *ptoCommandArray;
    int i;

    ptoCommandArray = Tcl_NewStringObj(s_pszCmdArrayName, _tcslen(s_pszCmdArrayName));

    for (i = 0; s_tclStringAndCommandValues[i].pszName != NULL; i++)
    {
        Tcl_Obj *ptoCommandName = Tcl_NewStringObj(s_tclStringAndCommandValues[i].pszName,
                                _tcslen(s_tclStringAndCommandValues[i].pszName));
        Tcl_Obj *ptoValue = Tcl_NewIntObj((int)s_tclStringAndCommandValues[i].uCommand);

        Tcl_ObjSetVar2(TclInterp, ptoCommandArray, ptoCommandName, ptoValue, TCL_LEAVE_ERR_MSG);
    }

    /* Doesn't seem to work */
    Tcl_TraceVar(TclInterp, (LPTSTR)s_pszCmdArrayName, TCL_TRACE_WRITES | TCL_TRACE_UNSETS,
                MyTcl_Hotkey_CommandValuesArrayTraceProc, (ClientData)NULL);

    return (TRUE);
}

static LPTSTR MyTcl_Hotkey_CommandValuesArrayTraceProc(ClientData clientData, Tcl_Interp *TclInterp, LPTSTR pszName1, LPTSTR pszName2, int nFlags)
{
    static TCHAR szMessage[40];
    PLISTNODE pNode = List_FindNode(g_TclScriptsList, TclInterp, SLC_TCLINTERP);
    PTCLSCRIPT pTclScript;

    if (nFlags & TCL_INTERP_DESTROYED || pNode == NULL)
        return (NULL);

    pTclScript = (PTCLSCRIPT)pNode->pData;

    if (nFlags & TCL_TRACE_UNSETS)
    {
        MyTcl_Hotkey_AddCommandValues(TclInterp);

        _stprintf(szMessage, String_LoadString(IDS_FORMAT_ERROR_TCL_READONLY_VARIABLE_UNSET),
                            pTclScript->szFileName, pszName1, pszName1, pTclScript->szFileName);

        return (szMessage);
    }
    else if (nFlags & TCL_TRACE_WRITES)
    {
        int i;

        for (i = 0; s_tclStringAndCommandValues[i].pszName != NULL; i++)
        {
            if (String_Equal(pszName2, s_tclStringAndCommandValues[i].pszName, TRUE))
            {
                Tcl_Obj *ptoCommandArray = Tcl_NewStringObj(s_pszCmdArrayName, _tcslen(s_pszCmdArrayName));
                Tcl_Obj *ptoCommandName = Tcl_NewStringObj(s_tclStringAndCommandValues[i].pszName,
                                    _tcslen(s_tclStringAndCommandValues[i].pszName));
                Tcl_Obj *ptoValue = Tcl_NewIntObj((int)s_tclStringAndCommandValues[i].uCommand);

                Tcl_ObjSetVar2(TclInterp, ptoCommandArray, ptoCommandName, ptoValue, TCL_LEAVE_ERR_MSG);

                break;
            }
        }

        _stprintf(szMessage, String_LoadString(IDS_FORMAT_ERROR_TCL_READONLY_VARIABLE_WRITE),
                            pTclScript->szFileName, pszName1, pszName2,
                            pszName1, pszName2, pTclScript->szFileName);

        return (szMessage);

    }

    return ((LPTSTR)NULL);
}

BOOL HotkeyTable_DeleteData(PVOID pKey, PVOID pData, PVOID pUserData)
{
    ASSERT(pData != NULL);

    List_DestroyList((PLINKEDLIST)pData);

    return (TRUE);
}


INT HotkeyList_DeleteData(PVOID pData)
{
    PTCLHOTKEY pTclHotkey = (PTCLHOTKEY)pData;

    ASSERT(pTclHotkey != NULL);

    if (!pTclHotkey->bCommand)
        Mem_Free(pTclHotkey->pszScript);

    Mem_Free(pTclHotkey);

    return (TRUE);
}

BOOL MyTcl_TranslateAccelerator(HWND hwnd, LPMSG lpMsg)
{
    BOOL bTranslated = FALSE;

    if (lpMsg->message == WM_KEYDOWN ||// lpMsg->message == WM_KEYUP ||
        lpMsg->message == WM_SYSKEYDOWN)// || lpMsg->message == WM_SYSKEYUP ||
        //lpMsg->message == WM_CHAR)
/*      (lpMsg->wParam == VK_BACK || lpMsg->wParam == VK_TAB || lpMsg->wParam == VK_RETURN ||
        lpMsg->wParam == VK_CLEAR || (lpMsg->wParam >= VK_PAUSE && lpMsg->wParam <= VK_SCROLL)))
*/  {
        PLISTNODE pNode = g_TclScriptsList->pHeadNode;
        BYTE fVirt      = FVIRTKEY | FNOINVERT;
        fVirt           |= IsCtrlDown() ? FCONTROL : 0;
        fVirt           |= IsAltDown() ? FALT : 0;
        fVirt           |= IsShiftDown() ? FSHIFT : 0;

        for ( ; pNode != NULL; pNode = pNode->pNextNode)
        {
            PTCLSCRIPT pTclScript = (PTCLSCRIPT)pNode->pData;

            if (HashTable_GetSize(pTclScript->pHotkeyTable) > 0)
            {
                PLINKEDLIST pHotkeyList = (PLINKEDLIST)HashTable_Lookup(pTclScript->pHotkeyTable, (PVOID)(WORD)lpMsg->wParam);

                if (pHotkeyList != NULL)
                {
                    PLISTNODE pNode;
                    PTCLHOTKEY pTclHotkey;

                    for (pNode = pHotkeyList->pHeadNode; pNode != NULL; pNode = pNode->pNextNode)
                    {
                        pTclHotkey = (PTCLHOTKEY)pNode->pData;

                        if (pTclHotkey->fVirt == fVirt)
                        {
                            if (((PTCLHOTKEY)pNode->pData)->bCommand)
                            {
                                if (MyTcl_HandleCommandAccelerator(hwnd, (PTCLHOTKEY)pNode->pData, lpMsg) && !bTranslated)
                                    bTranslated = TRUE;
                            }
                            else
                            {
                                MyTcl_Eval(pTclScript->TclInterp, ((PTCLHOTKEY)pNode->pData)->pszScript, TRUE);

                                bTranslated = TRUE;
                            }
                        }
                    }
                }
            }
        }
    }

    return (bTranslated);
}

static BOOL MyTcl_HandleCommandAccelerator(HWND hwnd, PTCLHOTKEY pTclHotkey, LPMSG lpMsg)
{
    HMENU hMenu;
    HMENU hSubMenu;
    MENUITEMINFO mii;
    int i;

    ASSERT(lpMsg->wParam = pTclHotkey->wKey);

    hMenu = GetMenu(hwnd);

    ASSERT(hMenu != NULL);

    INITSTRUCT(mii, TRUE);
    mii.fMask = MIIM_ID;

    for (i = 0; (hSubMenu = GetSubMenu(hMenu, i)) != NULL; i++)
    {
        if (GetMenuItemInfo(hSubMenu, pTclHotkey->uCmd, FALSE, &mii))
        {
            ASSERT(mii.wID == pTclHotkey->uCmd);

            break;
        }
    }

    SendMessage(hwnd, WM_INITMENUPOPUP, (WPARAM)hSubMenu, MAKELPARAM(i, FALSE));

    mii.fMask = MIIM_STATE;

    if (!GetMenuItemInfo(hSubMenu, pTclHotkey->uCmd, FALSE, &mii))
        return (FALSE);

    if (!(mii.fState & (MF_GRAYED | MF_DISABLED)))
    {
        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(pTclHotkey->uCmd, 1), (LPARAM)NULL);

        return (TRUE);
    }

    return (FALSE);
}

int MyTcl_Hotkey(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    ACCEL acHotkey;
    PLISTNODE pNode = List_FindNode(g_TclScriptsList, TclInterp, SLC_TCLINTERP);
    PTCLSCRIPT pts = (PTCLSCRIPT)pNode->pData;
    PTCLHOTKEY pTclHotkey = (PTCLHOTKEY)Mem_Alloc(sizeof(TCLHOTKEY));
    PLINKEDLIST pHotkeyList;
    LPTSTR pszHotkey;
    int i;
    BOOL bDone = FALSE;
    BOOL bAddDescription = TRUE;

    ASSERT(pts != NULL);

    if (objc < 3)
    {
Usage:
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T("[-c] [-n] <[<ctrl|alt|shift>+]key> <script|command_value>"));

        return (TCL_ERROR);
    }

    for (i = 1; i < objc && !bDone; i++)
    {
        LPCTSTR pszArg = Tcl_GetString(objv[i]);

        if (*pszArg == _T('-'))
        {
            switch (*(pszArg + 1))
            {
            case _T('c'):
                pTclHotkey->bCommand = TRUE;
            break;
            case _T('n'):
                bAddDescription = FALSE;
            break;
            case _T('-'):
                bDone = TRUE;
            break;
            case _T('\0'):
                // Not an option, the user wants to use the sutract (-) key
                i--;
                bDone = TRUE;
            break;
            default:
                Tcl_AppendResult(TclInterp, objv[0], _T(": unrecognized option "), objv[i], (LPTSTR)NULL);
            return (TCL_ERROR);
            }
        }
        else
        {
            i--;
            bDone = TRUE;
        }
    }

    if (objc - i < 2)
        goto Usage;

    pszHotkey = Tcl_GetString(objv[i++]);
    String_ProcessHotkey(pszHotkey, &acHotkey);

    pTclHotkey->fVirt       = acHotkey.fVirt;
    pTclHotkey->wKey        = acHotkey.key;

    if (pTclHotkey->wKey == 0)
    {
        Tcl_AppendResult(TclInterp, objv[0],
                    _T(": unknown key combination specified, "), pszHotkey,
                    _T(". Please refer to slacktcl.txt for a listing of")
                    _T(" avalable keys."), (LPTSTR)NULL);

        return (TCL_ERROR);
    }

    if (pTclHotkey->bCommand)
    {
        if (Tcl_GetIntFromObj(TclInterp, objv[i++], &pTclHotkey->uCmd) != TCL_OK)
            goto Usage;

        if (!MyTcl_Hotkey_LookupCommandValue(pTclHotkey->uCmd))
        {
            TCHAR szValue[20];

            Tcl_AppendResult(TclInterp, objv[0],
                    _T(": unknown command value specified, "),
                    _itoa(pTclHotkey->uCmd, szValue, 10),
                    _T(". Please refer to slacktcl.txt for a description of")
                    _T(" available command values."), (LPTSTR)NULL);

            return (TCL_ERROR);
        }

        if (bAddDescription)
            MyTcl_Hotkey_AddHotkeyDescription(pTclHotkey->uCmd, pszHotkey);
    }
    else
    {
        pTclHotkey->pszScript   = String_Duplicate(Tcl_GetString(objv[i++]));
    }

    if ((pHotkeyList = (PLINKEDLIST)HashTable_Lookup(pts->pHotkeyTable, (PVOID)pTclHotkey->wKey)) != NULL)
    {
        List_AddNodeAtHead(pHotkeyList, pTclHotkey);
    }
    else
    {
        pHotkeyList = List_CreateList(NULL, HotkeyList_DeleteData, NULL, NULL, 0);

        List_AddNodeAtHead(pHotkeyList, pTclHotkey);
        HashTable_Insert(pts->pHotkeyTable, (PVOID)pTclHotkey->wKey, pHotkeyList);
    }

    return (TCL_OK);
}

static BOOL MyTcl_Hotkey_AddHotkeyDescription(UINT uCmd, LPCTSTR pszDescription)
{
    MENUITEMINFO mii;
    TCHAR szMenuText[MAX_PATH] = _T("");
    LPTSTR pszNewDescription;
    LPTSTR psz;

    INITSTRUCT(mii, TRUE);
    mii.fMask       = MIIM_TYPE;
    mii.dwTypeData  = szMenuText;
    mii.cch         = MAX_PATH;

    if (!GetMenuItemInfo(g_hMainMenu, uCmd, FALSE, &mii))
        return (FALSE);

    if ((mii.fType & MFT_STRING) != MFT_STRING)
        return (FALSE);

    psz = pszNewDescription = String_Duplicate(pszDescription);

    if (pszNewDescription == NULL)
        return (FALSE);

    *(psz++) = _istalpha(*psz) ? _toupper(*psz) : *psz;

    for ( ; *psz != _T('\0'); psz++)
    {
        if (*psz == _T('+'))
        {
            if (*(psz++) != _T('\0'))
                *(psz++) = _istalpha(*psz) ? _toupper(*psz) : *psz;
            else
                break;
        }
    }

    if (String_CountSubChars(szMenuText, _T('\t')))
    {
        _tcscat(szMenuText, _T("/"));
        _tcscat(szMenuText, pszNewDescription);
    }
    else
    {
        _tcscat(szMenuText, _T("\t"));
        _tcscat(szMenuText, pszNewDescription);
    }

    Mem_Free(pszNewDescription);

    SetMenuItemInfo(g_hMainMenu, uCmd, FALSE, &mii);

    return (TRUE);
}

static BOOL MyTcl_Hotkey_LookupCommandValue(UINT uCmd)
{
    int i;

    for (i = 0; s_tclStringAndCommandValues[i].pszName != NULL; i++)
    {
        if (s_tclStringAndCommandValues[i].uCommand == uCmd)
            return (TRUE);
    }

    return (FALSE);
}
