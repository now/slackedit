/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : tcl.c
 * Created    : not known   (before 07/24/99)
 * Owner      : pcppopper
 * Revised on : 07/06/00
 * Comments   :  
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* tcl */
#include <tcl.h>

/* SlackEdit */
#include "../slack_main.h"
#include "tcl_main.h"
#include "tcl_internal.h"
#include "tcl_dialog.h"
#include "tcl_hotkey.h"
#include "tcl_load.h"
#include "tcl_window.h"
#include "tcl_shell.h"
#include "tcl_pcpedit.h"
#include "../settings/settings.h"

/* pcp_generic */
#include <pcp_browse.h>
#include <pcp_mem.h>
#include <pcp_path.h>
#include <pcp_string.h>

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

/* Intrinsic */

static Tcl_Interp *MyTcl_CreateInterp(void);
static void MyTcl_Error(Tcl_Interp *TclInterp);

PVOID ScriptList_CreateData(PVOID pData);
INT ScriptList_DeleteData(PVOID pData);
INT ScriptList_CompareData(PVOID pFromNode, PVOID pFromCall, UINT uDataType);

/* Extrinsic =P*/

static int MyTcl_Unload(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
static int MyTcl_Load(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

BOOL MyTcl_Init(void)
{
    int i;
    TCHAR szBuffer[MAX_PATH];
    HKEY hKey;
    DWORD dwSize = sizeof(szBuffer);
    DWORD dwType = REG_SZ;
    TCLSCRIPT ts;

    if (!MyTcl_LoadLibrary(_T("8"), _T("3"), FALSE))
        return (FALSE);

    g_TclScriptsList = List_CreateList(ScriptList_CreateData, ScriptList_DeleteData,
                            NULL, ScriptList_CompareData, 0);

    ts.pHotkeyTable     = HashTable_Create(NULL, NULL);
    _tcscpy(ts.szFileName, _T("c:\Main Script Handler"));
    ts.TclInterp        = MyTcl_CreateInterp();
    ts.pClipboardMonitorDataList = MyTcl_CreateClipboardMonitorList();

    List_AddNodeAtHead(g_TclScriptsList, &ts);

    if (RegOpenKeyEx(REG_ROOT, (REG_SLACKEDIT _T("\\Tcl Scripts")), 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return (TRUE); // Just break out...don't stop loading

    for (i = 0; RegEnumValue(hKey, i, szBuffer, &dwSize, NULL, &dwType, NULL, NULL) != ERROR_NO_MORE_ITEMS; i++)
    {
        TCHAR szFileName[MAX_PATH];
        DWORD dwSize2 = sizeof(szFileName);

        if (dwType != REG_SZ)
        {
            dwType = REG_SZ;

            continue;
        }

        RegQueryValueEx(hKey, szBuffer, NULL, &dwType, szFileName, &dwSize2);
        MyTcl_LoadFile(szFileName, TRUE);

        dwSize = sizeof(szBuffer);
    }

    return (TRUE);
}

int MyTcl_Eval(Tcl_Interp *TclInterp, LPTSTR pszScript, BOOL bComplain)
{
    int iRetVal;
    Tcl_Interp *TclUsedInterp = TclInterp;

    if (TclUsedInterp == NULL)
    {
        PLISTNODE pNode = g_TclScriptsList->pHeadNode;

        TclUsedInterp = ((PTCLSCRIPT)pNode->pData)->TclInterp;

        iRetVal = Tcl_EvalEx(TclUsedInterp, pszScript, -1, TCL_EVAL_GLOBAL);
    }
    else
    {
        iRetVal = Tcl_EvalEx(TclUsedInterp, pszScript, -1, TCL_EVAL_GLOBAL);
    }

    if (iRetVal != TCL_OK)
        MyTcl_Error(TclUsedInterp);

    return (iRetVal);
}

void MyTcl_OpenFile(void)
{
    TCHAR szFile[MAX_PATH] = _T("");
    BROWSEINFOEX bix;

    INITSTRUCT(bix, FALSE);
    bix.hwndOwner   = g_hwndMain;
    bix.pszFile = szFile;
    bix.pszTitle    = _T("Open TCL Script File");
    bix.pszFilter   = _T("TCL Filez (*.tcl)\0*.tcl");


    if (Browse_ForFile(&bix))
        MyTcl_LoadFile(szFile, TRUE);
}

int MyTcl_LoadFile(LPTSTR pszFile, BOOL bComplain)
{
    int iReturn;
    PLISTNODE pTempNode = g_TclScriptsList->pHeadNode;
    TCLSCRIPT ts;

    if ((pTempNode = List_FindNode(g_TclScriptsList, pszFile, SLC_FILENAME)) != NULL)
    {
        // Reload it
        List_DeleteNode(g_TclScriptsList, pTempNode);
    }

    ts.pHotkeyTable     = HashTable_Create(NULL, NULL);
    _tcscpy(ts.szFileName, pszFile);
    ts.TclInterp        = MyTcl_CreateInterp();
    ts.pClipboardMonitorDataList = MyTcl_CreateClipboardMonitorList();

    List_AddNodeAtHead(g_TclScriptsList, &ts);

    if ((iReturn = Tcl_EvalFile(ts.TclInterp, pszFile)) != TCL_OK)
    {
        TCHAR szFile[MAX_PATH];

        _stprintf(szFile, "%s\\%s", g_ScriptSettings.szScriptDir, Path_GetFileName(pszFile));

        if ((iReturn = Tcl_EvalFile(ts.TclInterp, szFile)) != TCL_OK)
        {
            if (bComplain)
                MyTcl_Error(ts.TclInterp);

            List_DeleteNode(g_TclScriptsList, g_TclScriptsList->pHeadNode);

            return (iReturn);
        }

        return (iReturn);
    }
    else
    {
        return (iReturn);
    }
}

static Tcl_Interp *MyTcl_CreateInterp(void)
{
    Tcl_Interp *TclInterp = Tcl_CreateInterp();
    TCHAR szPath[MAX_PATH];

    GetModuleFileName(g_hInstance, szPath, MAX_PATH);
    Tcl_FindExecutable(szPath);

    /* hm...maybe should be put in later */
    Tcl_Init(TclInterp);// != TCL_OK)
//      MessageBox(g_hwndMain, TclInterp->result, _T("Tcl Error"), MB_OK | MB_ICONEXCLAMATION);

    Tcl_CreateObjCommand(TclInterp, _T("::slack::load"), MyTcl_Load, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::unload"), MyTcl_Unload, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::hotkey"), MyTcl_Hotkey, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    Tcl_CreateObjCommand(TclInterp, _T("::slack::replace_sel"), MyTcl_ReplaceSel, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::get_sel_text"), MyTcl_GetSelText, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::set_sel"), MyTcl_SetSelection, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::get_sel"), MyTcl_GetSelection, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::get_filename"), MyTcl_GetFileName, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::get_linecount"), MyTcl_GetLineCount, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::get_line"), MyTcl_GetLine, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::get_linelength"), MyTcl_GetLineLength, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    Tcl_CreateObjCommand(TclInterp, _T("::slack::shlexec"), MyTcl_ShellExecute, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::dialog"), MyTcl_Dialog, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::msgbox"), MyTcl_MsgBox, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::browse"), MyTcl_Browse, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    Tcl_CreateObjCommand(TclInterp, _T("::slack::open_file"), MyTcl_OpenFileCmd, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::set_cookie"), MyTcl_SetCookie, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::get_cookie"), MyTcl_GetCookie, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    Tcl_CreateObjCommand(TclInterp, _T("::slack::clipboard"), MyTcl_Clipboard, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    Tcl_CreateObjCommand(TclInterp, _T("::slack::syntax"), MyTcl_Syntax, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    MyTcl_Hotkey_AddCommandValues(TclInterp);

    return (TclInterp);
}

static void MyTcl_Error(Tcl_Interp *TclInterp)
{
    static TCHAR szMessageTemplate[] = _T("You Killed Kenny!\n")
                        _T("Don't worry, he can still be saved by reading some debug info.\n")
                        _T("TclInterp->result is:\n\n")
                        _T("%s.\n\n")
                        _T("Do you want extended information?");
    LPTSTR pszMessage  = (LPTSTR)Mem_AllocStr(_tcslen(TclInterp->result) +
                                    _tcslen(szMessageTemplate) + SZ);
    _stprintf(pszMessage, szMessageTemplate, TclInterp->result);
    
    if (MessageBox(NULL, pszMessage, _T("Tcl Error"),
        MB_YESNO | MB_ICONEXCLAMATION | MB_TASKMODAL) == IDYES)
    {
        static TCHAR szMessageTemplateEx[] = _T("errorInfo is:\n\n")
                            _T("%s.\n\n")
                            _T("Hope this helps...");
        LPTSTR pszErrorInfo = Tcl_GetVar(TclInterp, _T("errorInfo"), TCL_GLOBAL_ONLY);
        LPTSTR pszMessageEx = (LPTSTR)Mem_AllocStr(_tcslen(pszErrorInfo) +
                                            _tcslen(szMessageTemplateEx) + SZ);
        _stprintf(pszMessageEx, szMessageTemplateEx, pszErrorInfo);

        MessageBox(NULL, pszMessageEx, _T("Tcl Error"), MB_OK | MB_TASKMODAL);

        Mem_Free(pszMessageEx);
    }

    Mem_Free(pszMessage);
}

PVOID ScriptList_CreateData(PVOID pData)
{
    PTCLSCRIPT pTclScript;

    pTclScript = (PTCLSCRIPT)Mem_Alloc(sizeof(TCLSCRIPT));
    pTclScript->pHotkeyTable    = ((PTCLSCRIPT)pData)->pHotkeyTable;
    _tcscpy(pTclScript->szFileName, ((PTCLSCRIPT)pData)->szFileName);
    pTclScript->TclInterp       = ((PTCLSCRIPT)pData)->TclInterp;
    pTclScript->pClipboardMonitorDataList = ((PTCLSCRIPT)pData)->pClipboardMonitorDataList;

    return (pTclScript);
}

INT ScriptList_DeleteData(PVOID pData)
{
    PTCLSCRIPT pTclScript = (PTCLSCRIPT)pData;

    HashTable_ForeachRemove(pTclScript->pHotkeyTable, HotkeyTable_DeleteData, (PVOID)0);
    HashTable_Destroy(pTclScript->pHotkeyTable);

    List_DestroyList(pTclScript->pClipboardMonitorDataList);

    Tcl_DeleteInterp(pTclScript->TclInterp);

    Mem_Free(pTclScript);

    return (TRUE);
}

INT ScriptList_CompareData(PVOID pFromNode, PVOID pFromCall, UINT uDataType)
{
    PTCLSCRIPT pTclScript = (PTCLSCRIPT)pFromNode;

    if (uDataType == SLC_TCLINTERP)
    {
        if (pTclScript->TclInterp == (Tcl_Interp *)pFromCall)
            return (0);
        else
            return (-1);
    }
    else if (uDataType == SLC_FILENAME)
    {
        return (_tcsicmp(pTclScript->szFileName, (LPTSTR)pFromCall));
    }

    ASSERT(FALSE);

    return (-1);
}

static int MyTcl_Load(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    if (objc < 2)
    {
        MyTcl_OpenFile();

        return (TCL_OK);
    }
    else
    {
        return (MyTcl_LoadFile(Tcl_GetStringFromObj(objv[1], NULL), TRUE));
    }
}

static int MyTcl_Unload(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    if (objc < 2)
    {
Usage:
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T("<filename>"));

        return (TCL_ERROR);
    }

    if (List_DeleteNode(g_TclScriptsList, List_FindNode(g_TclScriptsList, Tcl_GetStringFromObj(objv[1], NULL), SLC_FILENAME)))
        return (TCL_OK);
    else
        goto Usage;
}
