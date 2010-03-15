/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pp_tcl_shell.c
 * Created    : not known (before 12/21/99)
 * Owner      : pcppopper
 * Revised on : 07/06/00
 * Comments   : Tcl Shell command implementations 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* windows */
#include <tcl.h>
#include <shlobj.h>

/* SlackEdit */
#include "../slack_main.h"
#include "tcl_main.h"
#include "tcl_internal.h"
#include "tcl_shell.h"
#include "../settings/settings.h"

/* pcp_generic */
#include <pcp_browse.h>
#include <pcp_clipboard.h>
#include <pcp_linkedlist.h>
#include <pcp_mem.h>
#include <pcp_path.h>
#include <pcp_registry.h>
#include <pcp_string.h>

PVOID ClipboardMonitorList_CreateData(PVOID pData);
INT ClipboardMonitorList_DeleteData(PVOID pData);
INT ClipboardMonitorList_CompareData(PVOID pFromNode, PVOID pFromCall, UINT uDataType);

/****************************************************************
 * Function Implementation                                      *
 ****************************************************************/

int MyTcl_MsgBox(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    int i = 1;
    DWORD dwIcon = 0xFFFFFFFFL;
    DWORD dwButtons = 0xFFFFFFFFL;
    LPTSTR pszTitle = NULL, pszText = NULL, pszTextOut = _T(""), pszReturn;
    DWORD dwLen;

    if (objc < 2)
    {
Usage:
        Tcl_AppendResult(TclInterp, _T("Wrong # args: "), (LPTSTR)NULL);
BadOpt:
        Tcl_AppendResult(TclInterp, _T("should be "), Tcl_GetString(objv[0]),
            _T(" [-t <title>]"),
            _T(" [-b aborty_retry_ignore|ok|ok_cancel|retry_cancel|yes_no|yes_no_cancel]"),
            _T(" [-i exclamation/warning|info/asterisk|question|stop/error/hand|0]"),
            _T(" <message>"),
            (LPTSTR)NULL);

        return (TCL_ERROR);
    }

    if (objc > 2)
    {
        while ((i + 1) < objc)
        {
            if (_tcscmp(Tcl_GetString(objv[i]), _T("-t")) == 0)
            {
                i++;
                if (i == objc)
                    goto Usage;
                pszTitle = Tcl_GetString(objv[i]);
            }
            else if (_tcscmp(Tcl_GetString(objv[i]), _T("-b")) == 0)
            {
                LPTSTR pszArg;
                i++;
                if (i == objc)
                    goto Usage;

                pszArg = Tcl_GetString(objv[i]);

                if (_tcscmp(pszArg, _T("abort_retry_ignore")) == 0)
                    dwButtons = MB_ABORTRETRYIGNORE;
                else if (_tcscmp(pszArg, _T("ok")) == 0)
                    dwButtons = MB_OK;
                else if (_tcscmp(pszArg, _T("ok_cancel")) == 0)
                    dwButtons = MB_OKCANCEL;
                else if (_tcscmp(pszArg, _T("retry_cancel")) == 0)
                    dwButtons = MB_RETRYCANCEL;
                else if (_tcscmp(pszArg, _T("yes_no")) == 0)
                    dwButtons = MB_YESNO;
                else if (_tcscmp(pszArg, _T("yes_no_cancel")) == 0)
                    dwButtons = MB_YESNOCANCEL;
                else
                {
                    Tcl_AppendResult(TclInterp, _T("Unknown button combination \""),
                        pszArg, _T("\" for -b option: "),
                        _T("should be one of"),
                        _T("abort_retry_ignore, ok, ok_cancel, "),
                        _T("retry_cancel, yes_no, or yes_no_cancel"),
                        NULL);

                    return (TCL_ERROR);
                }
            }
            else if (_tcscmp(Tcl_GetString(objv[i]), _T("-i")) == 0)
            {
                LPTSTR pszArg;

                i++;
                if (i == objc)
                    goto Usage;
                
                pszArg = Tcl_GetString(objv[i]);

                if (_tcscmp(pszArg, _T("exclamation")) == 0 ||
                    _tcscmp(pszArg, _T("warning")) == 0)
                    dwIcon = MB_ICONEXCLAMATION;
                else if (_tcscmp(pszArg, _T("info")) == 0 ||
                    _tcscmp(pszArg, _T("asterisk")) == 0)
                    dwIcon = MB_ICONINFORMATION;
                else if (_tcscmp(pszArg, _T("question")) == 0)
                    dwIcon = MB_ICONQUESTION;
                else if (_tcscmp(pszArg, _T("stop")) == 0 ||
                    _tcscmp(pszArg, _T("error")) == 0 ||
                    _tcscmp(pszArg, _T("hand")) == 0)
                    dwIcon = MB_ICONSTOP;
                else if (pszArg[0] == _T('\0'))
                    dwIcon = 0;
                else
                {
                    Tcl_AppendResult(TclInterp, _T("Unknown button \""), pszArg,
                        _T("\" to -i option: should be one "),
                        _T("of exclamation/warning, info/asterisk, ")
                        _T("question, stop/error/hand"),
                        _T("or 0 to specify none"), NULL);

                    return (TCL_ERROR);
                }
            }
            else
            {
                Tcl_AppendResult(TclInterp, _T("Unknown option \""),
                            Tcl_GetString(objv[i]), _T("\": "), NULL);

                goto BadOpt;
            }

            i++;
        }
    }

    pszText = Tcl_GetString(objv[i++]);

    if (i != objc)
    {
        Tcl_AppendResult(TclInterp, _T("Wrong # args: should be \""),
                Tcl_GetString(objv[0]), _T("[<options>] <message>"), NULL);

        return (TCL_ERROR);
    }

    if (dwIcon == 0xFFFFFFFFL)
        dwIcon = 0;
    if (dwButtons == 0xFFFFFFFFL)
        dwButtons = MB_OK;

    dwLen = _tcslen(pszText);

    if (dwLen > 0)
    {
        LPTSTR ip, op;

        if ((pszTextOut = (LPTSTR)Mem_AllocStr(dwLen + SZ)) == NULL)
        {
            Tcl_Int_ReportAllocFailure(TclInterp);

            return (TCL_ERROR);
        }
        
        ip = pszText;
        op = pszTextOut;

        while (*ip != _T('\0'))
        {
            if (*ip == _T('\\') && *(ip + 1) == _T('n'))
            {
                *op = _T('\n');
                ip++;
            }
            else
            {
                *op = *ip;
            }

            ip++;
            op++;
        }
        
        *op = _T('\0');
        pszText = pszTextOut;
    }

    switch (MessageBox(g_hwndMain, pszTextOut, pszTitle, dwButtons | dwIcon | MB_SETFOREGROUND))
    {
        case IDABORT:
            pszReturn = _T("abort");
        break;
        case IDCANCEL:
            pszReturn = _T("cancel");
        break;
        case IDIGNORE:
            pszReturn = _T("ignore");
        break;
        case IDNO:
            pszReturn = _T("no");
        break;
        case IDOK:
            pszReturn = _T("ok");
        break;
        case IDRETRY:
            pszReturn = _T("retry");
        break;
        case IDYES:
            pszReturn = _T("yes");
        break;
        default:
            Tcl_AppendResult(TclInterp, _T("Out of memory"), NULL);
            Mem_Free(pszTextOut);       
        return (TCL_ERROR);
    }

    Tcl_AppendResult(TclInterp, pszReturn, NULL);

    Mem_Free(pszTextOut);

    return (TCL_OK);
}

int MyTcl_ShellExecute(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    int i;
    LPTSTR pszOperation = NULL, pszParams = NULL, pszDir = NULL, pszFile = NULL;
    INT nShowCmd = SW_SHOWNORMAL;
    unsigned long hInstance;

    if (objc > 1)
    {
        for (i = 1; (i + 1) < objc; i++)
        {
            if (_tcsncmp(Tcl_GetString(objv[i]), _T("-"), 1) == 0) //we have a switch
            {
                switch (*(Tcl_GetString(objv[i]) + 1))
                {
                    case _T('v'):
                        pszOperation = Tcl_GetString(objv[++i]);
                    break;
                    case _T('p'):
                        pszParams = Tcl_GetString(objv[++i]);
                    break;
                    case _T('d'):
                        if ((pszDir = Tcl_Int_DoFileName(Tcl_GetString(objv[++i]))) == NULL)
                            goto Usage;
                    break;
                    case _T('s'):
                    {
                        LPTSTR pszArg = Tcl_GetString(objv[++i]);
                        if (_tcsncmp(pszArg, _T("hi"), 2) == 0)
                            nShowCmd = SW_HIDE;
                        else if (_tcsncmp(pszArg, _T("ma"), 2) == 0)
                            nShowCmd = SW_SHOWMAXIMIZED;
                        else if (_tcsncmp(pszArg, _T("mi"), 2) == 0)
                            nShowCmd = SW_SHOWMINNOACTIVE;
                        else
                        {
                            Tcl_AppendResult(TclInterp, Tcl_GetString(objv[0]), _T(": Unknown switch: \""),
                                pszArg, _T("\" for -s it must be one of"),
                                _T(" [hi]dden|[ma]ximized|[mi]nimized"), NULL);

                            return (TCL_ERROR);
                        }
                    }
                    break;
                    default:
                        Tcl_AppendResult(TclInterp, Tcl_GetString(objv[0]), _T(": "),
                            Tcl_GetString(objv[i]), _T("is an unknown option. "), NULL);
                        goto Usage;
                    break;
                }
            }
        }

        pszFile = Tcl_GetString(objv[objc - 1]);
    }
    else
    {
Usage:
        Tcl_AppendResult(TclInterp, _T("Usage: "), Tcl_GetString(objv[0]), _T(" [-v <verb>] [-p <parms>] [-d <DefDirectory>] [-s <ShowStyle>] <file>"), NULL);

        return (TCL_ERROR);
    }

    if ((hInstance = (unsigned long)ShellExecute(g_hwndMain, pszOperation, pszFile,
                    pszParams, pszDir, nShowCmd)) > 32)
    {
        if (pszDir != NULL)
            Mem_Free(pszDir);

        return (TCL_OK);
    }
    else
    {
        auto TCHAR szErr[MAX_PATH + 120];

        switch (hInstance)
        {
            case 0:
                _stprintf(szErr, _T("The operation system is out of memory or resources (\"%s\")."), pszFile);
            break;
            case ERROR_FILE_NOT_FOUND:
                _stprintf(szErr, _T("The specified file (\"%s\") wasn't found."), pszFile);
            break;
            case ERROR_PATH_NOT_FOUND:
                _stprintf(szErr, _T("The specified path was not found for (\"%s\")."), pszFile);
            break;
            case ERROR_BAD_FORMAT:
                _stprintf(szErr, _T("The .EXE file (\"%s\") is invalid."), pszFile);
            break;
            case SE_ERR_ACCESSDENIED:
                _stprintf(szErr, _T("You don't have access to the file: \"%s\"."), pszFile);
            break;
            case SE_ERR_ASSOCINCOMPLETE:
                _stprintf(szErr, _T("The file \"%s\" doesn't have an association."), pszFile);
            break;
            case SE_ERR_DDEBUSY:
                _stprintf(szErr, _T("The DDE transaction couldn't be completed since other DDE transactions were being processed for (\"%s\")."), pszFile);
            break;
            case SE_ERR_DDEFAIL:
                _stprintf(szErr, _T("The DDE transaction failed for file: \"%s\"."), pszFile);
            break;
            case SE_ERR_DDETIMEOUT:
                _stprintf(szErr, _T("The DDE transaction timed out for file: \"%s\"."), pszFile);
            break;
            case SE_ERR_DLLNOTFOUND:
                _stprintf(szErr, _T("The dll wasn't found (\"%s\")."), pszFile);
            break;
            case SE_ERR_NOASSOC:
                _stprintf(szErr, _T("The file \"%s\" doesn't have an association."), pszFile);
            break;
            case SE_ERR_OOM:
                _stprintf(szErr, _T("Not enough memory to run \"%s\"."), pszFile);
            break;
            case SE_ERR_SHARE:
                _stprintf(szErr, _T("A sharing violation ocurred with \"%s\"."), pszFile);
            break;
            default:
                _stprintf(szErr, _T("Unknown error while running \"%s\"."), pszFile);
            break;
        }

        Tcl_AppendResult(TclInterp, szErr, (LPTSTR)NULL);

        if (pszDir != NULL)
            Mem_Free(pszDir);

        return (TCL_ERROR);
    }
}

int MyTcl_Browse(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    if (objc < 2)
    {
        Tcl_AppendResult(TclInterp, Tcl_GetString(objv[0]), _T(": Wrong # args."),
                    _T(" browse <file|path>"), NULL);

        return (TCL_ERROR);
    }

    if (String_Equal(Tcl_GetString(objv[1]), _T("file"), FALSE))
    {
        LPTSTR pszFile, pszFilter = NULL, pszDir = NULL;
        int i, j, iRet, iErr;
        TCHAR szBuf[MAX_PATH], szBuf2[MAX_PATH];
        OPENFILENAME ofn;
        BOOL fSave = FALSE, fNoExist = FALSE, bSuccess = FALSE;

        if ((pszFile = (LPTSTR)Mem_AllocStr(MAX_PATH * 48)) == NULL)
        {
            Tcl_Int_ReportAllocFailure(TclInterp);

            return (TCL_ERROR);
        }

        INITSTRUCT(ofn, TRUE);

        ofn.nMaxFile    = (MAX_PATH * 48);
        ofn.hwndOwner   = g_hwndMain;
        ofn.Flags       = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK | 
            OFN_NONETWORKBUTTON | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | 
            OFN_LONGNAMES | OFN_EXPLORER;
        ofn.lpstrTitle  = _T("Open File(s)");

        if (objc > 2)
        {
            for (i = 2; i < objc; i++)
            {
                if (_tcsncmp(Tcl_GetString(objv[i]), _T("-"), 1) == 0)
                {
                    switch (*(Tcl_GetString(objv[i]) + 1))
                    {
                        case _T('t'): //Title
                            ofn.lpstrTitle = Tcl_GetString(objv[++i]);
                        break;
                        case _T('f'): //Filter
                        {
                            LPTSTR pszArg = Tcl_GetString(objv[++i]);

                            if ((pszFilter = (LPTSTR)Mem_AllocStr(_tcslen(pszArg) + 1 + SZ)) == NULL)
                            {
                                Tcl_Int_ReportAllocFailure(TclInterp);
                                
                                return (TCL_ERROR);
                            }

                            for (j = 0; pszArg[j] != _T('\0'); j++)
                            {
                                //filter seperator is | (might use that for other stuff to)
                                if (pszArg[j] == _T('|'))
                                    *(pszFilter + j) = _T('\0');
                                else
                                    *(pszFilter + j) = pszArg[j];
                            }

                            *(pszFilter + j) = _T('\0');

                            if (*(pszFilter + j - 1) != _T('\0'))
                                *(pszFilter + j + 1) = _T('\0');

                            ofn.lpstrFilter = pszFilter;
                            ofn.nFilterIndex = 1;
                        }
                        break;
                        case _T('d'): //Default Directory
                            if ((i + 2) < objc)
                            {
                                if ((pszDir = Tcl_Int_DoFileName(Tcl_GetString(objv[++i]))) == NULL)
                                {
                                    Tcl_Int_ReportAllocFailure(TclInterp);

                                    return (TCL_ERROR);
                                }
                                
                                ofn.lpstrInitialDir = pszDir;
                            }
                            else
                            {
                                Tcl_AppendResult(TclInterp, Tcl_GetString(objv[0]), _T(": -d usage is '-d <defaultDir>'"), NULL);

                                return (TCL_ERROR);
                            }
                        break;
                        case _T('M'): //Multiselect
                            ofn.Flags |= OFN_ALLOWMULTISELECT;
                        break;
                        case _T('n'): //Networkbutton
                            ofn.Flags &= ~(OFN_NONETWORKBUTTON);
                        break;
                        case _T('s'): //Save dialog instead
                            fSave       = TRUE;
                            fNoExist    = TRUE;
                        break;
                        default:
                            Tcl_AppendResult(TclInterp, Tcl_GetString(objv[0]), _T(": "),
                                        Tcl_GetString(objv[0]),
                                        _T(" is an unknown option."), NULL);
                        return (TCL_ERROR);
                    }
                }
            }
        }

        if (fNoExist == TRUE)
            ofn.Flags &= ~(OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);

        if (fSave == TRUE)
            ofn.Flags &= ~(OFN_ALLOWMULTISELECT);

        pszFile[0] = _T('\0');
        ofn.lpstrFile = pszFile;

        if (!(ofn.Flags & OFN_EXPLORER) && pszFilter == NULL)
        {
            ofn.lpstrFilter = _T("All Files\0*.*\0");
            ofn.nFilterIndex = 1;
        }

        if (fSave)
            bSuccess = GetSaveFileName(&ofn);
        else
            bSuccess = GetOpenFileName(&ofn);

        if (bSuccess)
        {
            int i, iPos;

            String_ReplaceChar(ofn.lpstrFile, '\\', '/');

            iPos = _tcslen(ofn.lpstrFile) + SZ;

            if (ofn.Flags & OFN_ALLOWMULTISELECT)
            {
                int iDirLen;
                TCHAR szNewFile[MAX_PATH];
                int j;

                /* check if we just have one entry */
                if (*(ofn.lpstrFile + iPos) == _T('\0'))                    
                    goto JustOne;

                /* copy DirName into newFile. */
                for (i = 0; *(ofn.lpstrFile + i) != _T('\0'); i++)
                    *(szNewFile + i) = *(ofn.lpstrFile + i);

                /* append a directory seperator */
                *(szNewFile + (i++)) = _T('/');
                /* terminate string */
                *(szNewFile + i) = _T('\0');
                /* save its length. */
                iDirLen = _tcslen(szNewFile);

                do
                {
                    for (i = iPos, j = 0; *(ofn.lpstrFile + i) != _T('\0'); i++, j++)
                        *(szNewFile + iDirLen + j) = *(ofn.lpstrFile + i);
                    /* terminate the new string */
                    *(szNewFile + iDirLen + j) = _T('\0');
                    Tcl_AppendElement(TclInterp, szNewFile);
                    iPos += j + 1;
                } while (*(ofn.lpstrFile + iPos) != _T('\0'));

                goto TheEnd;
            }

JustOne:
            Tcl_AppendResult(TclInterp, ofn.lpstrFile, NULL);
TheEnd:
            iRet = TCL_OK;
        }
        else if ((iErr = CommDlgExtendedError()) == CDERR_GENERALCODES)
        {
            iRet = TCL_OK;
        }
        else
        {
            switch (iErr)
            {
                case CDERR_STRUCTSIZE: 
                    _tcscpy(szBuf, _T("CDERR_STRUCTSIZE: The lStructSize member of the structure for the corresponding common dialog box is invalid."));
                break;
                case CDERR_INITIALIZATION:
                    _tcscpy(szBuf, _T("CDERR_INITIALIZATION: The common dialog box procedure failed during initialization. This error often occurs when insufficient memory is available."));
                break;
                case CDERR_NOTEMPLATE:
                    _tcscpy(szBuf, _T("CDERR_NOTEMPLATE: The ENABLETEMPLATE flag was specified in the Flags member of a structure for the corresponding common dialog box, but the application failed to provide a corresponding template."));
                break;
                case CDERR_NOHINSTANCE:
                    _tcscpy(szBuf, _T("CDERR_NOHINSTANCE: The ENABLETEMPLATE flag was specified in the Flags member of a structure for the corresponding common dialog box, but the application failed to provide a corresponding instance handle."));
                break;
                case CDERR_LOADSTRFAILURE:
                    _tcscpy(szBuf, _T("CDERR_LOADSTRFAILURE: The common dialog box procedure failed to load a specified string."));
                break;
                case CDERR_FINDRESFAILURE:
                    _tcscpy(szBuf, _T("CDERR_FINDRESFAILURE: The common dialog box procedure failed to find a specified resource!"));
                break;
                case CDERR_LOADRESFAILURE:
                    _tcscpy(szBuf, _T("CDERR_LOADRESFAILURE: The common dialog box procedure failed to load a specified resource."));
                break;
                case CDERR_LOCKRESFAILURE:
                    _tcscpy(szBuf, _T("CDERR_LOCKRESFAILURE: The common dialog box procedure failed to lock a specified resource."));
                break;
                case CDERR_MEMALLOCFAILURE:
                    _tcscpy(szBuf, _T("CDERR_MEMALLOCFAILURE: The common dialog box procedure was unable to allocate memory for internal structures."));
                break;
                case CDERR_MEMLOCKFAILURE:
                    _tcscpy(szBuf, _T("CDERR_MEMLOCKFAILURE: The common dialog box procedure was unable to lock the memory associated with a handle."));
                break;
                case CDERR_NOHOOK:
                    _tcscpy(szBuf, _T("CDERR_NOHOOK: The ENABLEHOOK flag was specified in the Flags member of a structure for the corresponding common dialog box, but the application failed to provide a pointer to a corresponding hook function."));
                break;
                case FNERR_FILENAMECODES:
                    _tcscpy(szBuf, _T("FNERR_FILENAMECODES: Error codes for the Open and Save As common dialog boxes. these errors are in the range 0x3000 through 0x3FFF."));
                break;
                case FNERR_SUBCLASSFAILURE:
                    _tcscpy(szBuf, _T("FNERR_SUBCLASSFAILURE: An attempt to subclass a list box failed because insufficient memory was available."));
                break;
                case FNERR_INVALIDFILENAME:
                    _tcscpy(szBuf, _T("FNERR_INVALIDFILENAME: A filename is invalid."));
                break;
                case FNERR_BUFFERTOOSMALL:
                    _tcscpy(szBuf, _T("FNERR_BUFFERTOOSMALL: The buffer for a filename is too small. (This buffer is pointed to by the lpstrFile member of the structure for a common dialog box.)"));
                break;
                default: 
                    _tcscpy(szBuf, _T("Unknown error code!"));
                break;
            }

            Tcl_AppendResult(TclInterp, _T("CommonDlgError# 0x"), _ltot(iErr, szBuf2, 16), _T(" "), szBuf, NULL);

            iRet = TCL_ERROR;
        }

        if (ofn.lpstrInitialDir != NULL)
            Mem_Free(pszDir);
        if (ofn.lpstrFilter != NULL)
            Mem_Free(pszFilter);
        Mem_Free(pszFile);

        return (iRet);
    }
    else if (String_Equal(Tcl_GetString(objv[1]), _T("path"), FALSE))
    {
        BROWSEINFOEX bix;
        LPTSTR pszDir;
        TCHAR szPath[MAX_PATH] = _T("");
        TCHAR szStartPath[MAX_PATH] = _T("");
        int i;

        bix.hwndOwner       = g_hwndMain;
        bix.dwFlags         = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
        bix.pszStartPath    = szStartPath;
        bix.pszPath         = szPath;

        if (objc > 2)
        {
            for (i = 2; i < objc; i++)
            {
                if (_tcsncmp(Tcl_GetString(objv[i]), _T("-"), 1) == 0)
                {
                    switch (*(Tcl_GetString(objv[i]) + 1))
                    {
                        case _T('t'): //Title
                            bix.pszTitle = Tcl_GetString(objv[++i]);
                        break;
                        case _T('d'): //Default Directory
                            if ((i + 2) < objc)
                            {
                                if ((pszDir = Tcl_Int_DoFileName(Tcl_GetString(objv[++i]))) == NULL)
                                {
                                    Tcl_Int_ReportAllocFailure(TclInterp);

                                    return (TCL_ERROR);
                                }

                                _tcscpy(szStartPath, pszDir);
                                Mem_Free(pszDir);
                            }
                            else
                            {
                                Tcl_AppendResult(TclInterp, Tcl_GetString(objv[0]), _T(": -d usage is '-d <defaultDir>'"), NULL);
          
                                return (TCL_ERROR);
                            }
                        break;
                        case _T('c'):
                            bix.dwFlags &= ~BIF_RETURNONLYFSDIRS;
                            bix.dwFlags &= ~BIF_BROWSEFORPRINTER;
                            bix.dwFlags |= BIF_BROWSEFORCOMPUTER;
                        break;
                        case _T('p'):
                            bix.dwFlags &= ~BIF_RETURNONLYFSDIRS;
                            bix.dwFlags &= ~BIF_BROWSEFORCOMPUTER;
                            bix.dwFlags |= BIF_BROWSEFORPRINTER;
                        break;
                        default:
                            Tcl_AppendResult(TclInterp, Tcl_GetString(objv[0]), _T(": "),
                                        Tcl_GetString(objv[i]),
                                        _T(" is an unknown option."), NULL);
                        return (TCL_ERROR);
                    }
                }
            }
        }

        if (Browse_ForPath(&bix))
        {
            Tcl_Obj *ptoszResult;

            String_ReplaceChar(szPath, '\\', '/');
            ptoszResult = Tcl_NewStringObj(szPath, _tcslen(szPath));
            Tcl_SetObjResult(TclInterp, ptoszResult);
        }
        
        return (TCL_OK);
    }
    else
    {
        Tcl_AppendResult(TclInterp, Tcl_GetString(objv[0]), _T(": Wrong parameter "), Tcl_GetString(objv[0]),
            _T(" should be <file|path>"), NULL);

        return (TCL_ERROR);
    }
}

int MyTcl_SetCookie(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    HKEY hKey;
    TCHAR szKey[MAX_PATH];
    PLISTNODE pNode = List_FindNode(g_TclScriptsList, TclInterp, SLC_TCLINTERP);
    PTCLSCRIPT pts = (PTCLSCRIPT)pNode->pData;

    ASSERT(pts != NULL);

    if (objc != 3)
    {
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T("<name> <value>"));

        return (TCL_ERROR);
    }

    _stprintf(szKey, _T("%s\\%s\\%s"), REG_SLACKEDIT, REG_COOKIES, Path_GetFileName(pts->szFileName));
    RegCreateKey(REG_ROOT, szKey, &hKey);
    Registry_SetSZ(hKey, Tcl_GetString(objv[1]), Tcl_GetString(objv[2]));
    RegCloseKey(hKey);

    return (TCL_OK);
}

int MyTcl_GetCookie(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    HKEY hKey;
    TCHAR szKey[MAX_PATH + 120];
    PLISTNODE pNode = List_FindNode(g_TclScriptsList, TclInterp, SLC_TCLINTERP);
    PTCLSCRIPT pts = (PTCLSCRIPT)pNode->pData;
    DWORD dwType;
    DWORD dwSize;
    LPTSTR pszResult;
    Tcl_Obj *ptoszResult;

    ASSERT(pts != NULL);

    if (objc < 2)
    {
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T("<name> [default]"));

        return (TCL_ERROR);
    }

    _stprintf(szKey, _T("%s\\%s\\%s"), REG_SLACKEDIT, REG_COOKIES, Path_GetFileName(pts->szFileName));

    if (RegOpenKey(REG_ROOT, szKey, &hKey) != ERROR_SUCCESS)
    {
        Tcl_AppendResult(TclInterp, objv[0], _T(": Failed to open registry key"), (LPTSTR)NULL);

        return (TCL_ERROR);
    }

    if (RegQueryValueEx(hKey, Tcl_GetString(objv[1]), NULL, &dwType, NULL, &dwSize) != ERROR_SUCCESS || dwType != REG_SZ)
    {
        Tcl_AppendResult(TclInterp, objv[0], _T(": Either the key could not be used or the data contained isn't valid"), (LPTSTR)NULL);

        return (TCL_ERROR);
    }

    if ((pszResult = Mem_AllocStr(dwSize)) == NULL)
    {

        Tcl_Int_ReportAllocFailure(TclInterp);

        return (TCL_ERROR);
    }

    Registry_GetSZ(hKey, Tcl_GetString(objv[1]), pszResult, ((objc == 3) ? Tcl_GetString(objv[2]) : _T("")), dwSize);
    RegCloseKey(hKey);

    ptoszResult = Tcl_NewStringObj(pszResult, _tcslen(pszResult));
    Mem_Free(pszResult);

    Tcl_SetObjResult(TclInterp, ptoszResult);

    return (TCL_OK);
}

int MyTcl_Clipboard(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    if (objc < 2)
    {
Usage:
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T("<get_text|set_text|monitor> [text]|<id> [script]"));

        return (TCL_ERROR);
    }

    if (String_Equal(_T("get_text"), Tcl_GetString(objv[1]), FALSE))
    {
        LPTSTR pszText;
        Tcl_Obj *ptoszResult;

        Clipboard_GetText(&pszText);
        ptoszResult = Tcl_NewStringObj(pszText, _tcslen(pszText));
        Mem_Free(pszText);
        Tcl_SetObjResult(TclInterp, ptoszResult);
    }
    else if (String_Equal(_T("set_text"), Tcl_GetString(objv[1]), FALSE))
    {
        LPTSTR pszText;
        Tcl_Obj *ptoszResult;

        Clipboard_GetText(&pszText);
        ptoszResult = Tcl_NewStringObj(pszText, _tcslen(pszText));
        Mem_Free(pszText);
        Tcl_SetObjResult(TclInterp, ptoszResult);
    }
    else if (String_Equal(_T("monitor"), Tcl_GetString(objv[1]), FALSE))
    {
        TCLCLIPBOARDMONITORDATA tcmd;
        PLISTNODE pNode = List_FindNode(g_TclScriptsList, TclInterp, SLC_TCLINTERP);
        PTCLSCRIPT pts = (PTCLSCRIPT)pNode->pData;
        UINT uID;

        if (objc < 3)
            goto Usage;

        Tcl_GetIntFromObj(TclInterp, objv[2], (int *)&uID);

        if (objc < 4)
        {

            // Remove the monitor with objv[2] as it's id
            List_DeleteNode(pts->pClipboardMonitorDataList, List_FindNode(pts->pClipboardMonitorDataList, (PVOID)uID, 0));
        }
        else
        {
            PLISTNODE pNode = List_FindNode(pts->pClipboardMonitorDataList, (PVOID)uID, 0);

            tcmd.uID = uID;
            tcmd.pszScript = Tcl_GetString(objv[3]);

            if (pNode != NULL)
            {
                // just change the script of the monitor
                Mem_Free(((PTCLCLIPBOARDMONITORDATA)pNode->pData)->pszScript);
                ((PTCLCLIPBOARDMONITORDATA)pNode->pData)->pszScript = String_Duplicate(tcmd.pszScript);
            }
            else
            {
                List_AddNodeAtHead(pts->pClipboardMonitorDataList, &tcmd);
            }
        }
    }
    else
    {
        goto Usage;
    }

    return (TCL_OK);
}

PLINKEDLIST MyTcl_CreateClipboardMonitorList(void)
{
    return (List_CreateList(ClipboardMonitorList_CreateData, ClipboardMonitorList_DeleteData, NULL, ClipboardMonitorList_CompareData, 0));
}

PVOID ClipboardMonitorList_CreateData(PVOID pData)
{
    PTCLCLIPBOARDMONITORDATA pcmd;

    pcmd = (PTCLCLIPBOARDMONITORDATA)Mem_Alloc(sizeof(TCLCLIPBOARDMONITORDATA));
    pcmd->pszScript = String_Duplicate(((PTCLCLIPBOARDMONITORDATA)pData)->pszScript);
    pcmd->uID = ((PTCLCLIPBOARDMONITORDATA)pData)->uID;

    return (pcmd);
}

INT ClipboardMonitorList_DeleteData(PVOID pData)
{
    Mem_Free(((PTCLCLIPBOARDMONITORDATA)pData)->pszScript);
    Mem_Free(pData);

    return (TRUE);
}

INT ClipboardMonitorList_CompareData(PVOID pFromNode, PVOID pFromCall, UINT uDataType)
{
    if (((PTCLCLIPBOARDMONITORDATA)pFromNode)->uID == (UINT)pFromCall)
        return (0);

    return (-1);
}

void MyTcl_ClipboardMonitorNotify(void)
{
    PLISTNODE pNode;
    
    if (g_TclScriptsList == NULL)
        return;

    pNode = g_TclScriptsList->pHeadNode;

    for ( ; pNode != NULL; pNode = pNode->pNextNode)
    {
        PLISTNODE pMonitorListNode = ((PTCLSCRIPT)pNode->pData)->pClipboardMonitorDataList->pHeadNode;

        for ( ; pMonitorListNode != NULL; pMonitorListNode = pMonitorListNode->pNextNode)
            Tcl_EvalEx(((PTCLSCRIPT)pNode->pData)->TclInterp, ((PTCLCLIPBOARDMONITORDATA)pMonitorListNode->pData)->pszScript, -1, TCL_EVAL_GLOBAL);
    }
}
