/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : dialogdec.c
 * Created    : not known   (before 08/29/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:06:09
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* windows */
#include <pshpack2.h>
#include <commctrl.h>
#include <tcl.h>

/* SlackEdit */
#include "../slack_main.h"
#include "tcl_internal.h"

/* pcp_generic */
#include <pcp_mem.h>
#include <pcp_string.h>

typedef enum
{
    IC_BUTTON       = 0x0080,
    IC_EDIT         = 0x0081,
    IC_STATIC       = 0x0082,
    IC_LISTBOX      = 0x0083,
    IC_SCROLLBAR    = 0x0084,
    IC_COMBOBOX     = 0x0085
} ICLASS;

typedef enum tagCTRLTYPE
{
    CT_COMBOBOX,
    CT_EDIT,
    CT_CHECK,
    CT_LABEL
} CTRLTYPE;

typedef struct tagDLGITEM
{
    int     nLength;
    LPWORD  lpwMem;
} DLGITEM, *LPDLGITEM;

typedef struct tagDLGITEMS
{
    int     nCount;
    DLGITEM adiItems[];
} DLGITEMS, *LPDLGITEMS;

typedef struct tagCTRLINFO
{
    ICLASS  icType;
    WORD    wID;
    LPTSTR  pszTextID;
    union
    {
        struct
        {
            BOOL    bExUI;
            BOOL    bRetSel;
            int     iSel;
            int     nCount;
            LPTSTR  pszString[];  // unsized array.
        } cbo;
        struct
        {
            int iVal;
        } chk;
    };
} CTRLINFO, *LPCTRLINFO;

typedef struct tagDLGDATA
{
    int             nRetVal;
    LPDLGTEMPLATE   lpdt;
    Tcl_Interp      *TclInterp;
    int             nCtrlCount;
    WORD            wItemIDCounter;
    LPCTRLINFO      lpci[];  //unsized array
} DLGDATA, FAR *LPDLGDATA;

typedef struct
{
     WORD   signature;           // MS says dlgVer is first and signature is
     WORD   dlgVer;              // second, but through trail and error,
     DWORD  dwHelpID;            // I found this was wrong!
     DWORD  dwExStyle;
     DWORD  dwStyle;
     WORD   cdit;
     short  x;
     short  y;
     short  cx;
     short  cy;
} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;

typedef struct
{
    DWORD   dwHelpID;
    DWORD   dwExStyle;
    DWORD   dwStyle;
    short   x;
    short   y;
    short   cx;
    short   cy;
    WORD    wID;
} DLGITEMTEMPLATEEX, FAR *LPDLGITEMTEMPLATEEX; 

static int DialogDec_AddDlgItem(LPWORD lpw, ICLASS icClass, DWORD dwFlags,
                       WORD wID, DWORD dwHelpID, int x, int y,
                       int cx, int cy, LPTSTR pszText);
static int DialogDec_AddDlgHeader(LPWORD lpw, WORD wCtrls, LPTSTR pszTitle, int cx, int cy);
static BOOL CALLBACK DialogDec_DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static int DialogDec_DecodeItem(Tcl_Interp *TclInterp, Tcl_Obj *ptoList, LPDLGITEMS *lpdiItems, LPDLGDATA *lpddData);
static BOOL DialogDec_InitCboList(HWND hwndDlg, UINT uID, LPCTRLINFO lpci);
static BOOL DialogDec_FillCboList(Tcl_Interp *TclInterp, Tcl_Obj *ptoList, LPCTRLINFO *lpci);

#define DWORD_ALIGN(x)  (((x) + (sizeof(DWORD) - 1)) & ~(sizeof(DWORD) - 1))

static int DialogDec_AddDlgItem(LPWORD lpw, ICLASS icClass, DWORD dwFlags,
                       WORD wID, DWORD dwHelpID, int x, int y,
                       int cx, int cy, LPTSTR pszText)
{
    LPWORD lpwStart;
    LPDLGITEMTEMPLATEEX lpditex;
    LPWSTR lpwString;
    int nChar;

    lpwStart = lpw;

    lpditex             = (LPDLGITEMTEMPLATEEX) lpw;
    lpditex->dwHelpID   = dwHelpID;
    lpditex->dwExStyle  = 0;
    lpditex->dwStyle    = WS_CHILD | WS_VISIBLE | dwFlags;
    lpditex->x          = (short)x;
    lpditex->y          = (short)y;
    lpditex->cx         = (short)cx;
    lpditex->cy         = (short)cy;
    lpditex->wID        = wID;
    lpw                 = (LPWORD)DWORD_ALIGN((DWORD)(lpditex + 1));

    *(lpw++)            = 0xFFFF;
    *(lpw++)            = (unsigned short)icClass;

    if (pszText != NULL && *pszText != _T('\0'))
    {
        lpwString = (LPWSTR)lpw;
        nChar = MultiByteToWideChar(CP_ACP, 0, pszText, -1, lpwString, 50);
        lpw += nChar;
    }
    else
    {
        *(lpw++) = 0;       // no title
    }

    *(lpw++) = 0;           // creation data
    lpw = (LPWORD)DWORD_ALIGN((DWORD)lpw);  // align DLGITEMTEMPLATEEX on DWORD boundary

    // return memory length used in sizeof(WORD).
    return (lpw - lpwStart);
}

static int DialogDec_AddDlgHeader(LPWORD lpw, WORD wCtrls, LPTSTR pszTitle, int cx, int cy)
{
    LPWORD lpwStart;
    LPWSTR lpwString;
    int nChar;
    LPDLGTEMPLATEEX lpdtex;

    lpwStart = lpw;

    lpdtex  = (LPDLGTEMPLATEEX)lpw; // Define a dialog box.
    lpdtex->signature   = 1;            // this is always 1.
    lpdtex->dlgVer      = 0xFFFF;       // dialog template is an extended type.
    lpdtex->dwHelpID    = 0;            // help context identifier.
    lpdtex->dwExStyle   = 0;            // extended styles
    // normal styles
    lpdtex->dwStyle     = WS_VISIBLE | WS_POPUP | WS_SYSMENU |
                            WS_CAPTION | DS_CENTER | DS_MODALFRAME |
                            DS_SETFOREGROUND | DS_NOFAILCREATE |
                            DS_SETFONT | DS_CONTEXTHELP;
    lpdtex->cdit        = wCtrls;       // number of controls
    lpdtex->x           = 0;
    lpdtex->y           = 0;
    lpdtex->cx          = (short)cx;
    lpdtex->cy          = (short)cy;
    lpw                 = (LPWORD)(lpdtex + 1); // an alignment might be needed here.
                                //  we'll see.
    *(lpw++)            = 0;                // menu
    *(lpw++)            = 0;                // dialog box class
    lpwString           = (LPWSTR)lpw;      // title
    nChar               = MultiByteToWideChar(CP_ACP, 0, pszTitle,
                                                -1, lpwString, 50);
    lpw                 += nChar;
    *(lpw++)            = 8;                // set font pitch
    *(lpw++)            = FW_NORMAL;        // set font weight
    *(lpw++)            = FALSE;            // set italic
    lpwString           = (LPWSTR)lpw;      // set font name
    nChar               = MultiByteToWideChar(CP_ACP, 0, _T("MS Sans Serif"),
                                                -1, lpwString, 25);
    lpw                 += nChar;
    lpw                 = (LPWORD)DWORD_ALIGN((DWORD)lpw);  // align next DLGITEMTEMPLATEEX on a DWORD boundary

    //return memory length used by the header in sizeof(WORD).
    return (lpw - lpwStart);
}

static BOOL CALLBACK DialogDec_DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto BOOL bRet = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        auto LPDLGDATA lpddData = (LPDLGDATA)lParam;
        auto int i;

        /* place the pointer to our data structure 
         * in the dialog's user space */
        SetWindowLong(hwndDlg, DWL_USER, (LONG)lpddData);

        for (i = 0; i < lpddData->nCtrlCount; i++)
        {
            switch (lpddData->lpci[i]->icType)
            {
                case IC_COMBOBOX:
                    // Turns on 'extended UI' for the combobox.
                    if (lpddData->lpci[i]->cbo.bExUI) 
                        ComboBox_SetExtendedUI(GetDlgItem(hwndDlg,
                            lpddData->lpci[i]->wID), 1);
                    // Stuffs the combobox with entries.
                    if (!DialogDec_InitCboList(hwndDlg,
                        lpddData->lpci[i]->wID, lpddData->lpci[i])) 
                    EndDialog(hwndDlg, 0);
                break;
                case IC_BUTTON:
                    if (lpddData->lpci[i]->chk.iVal == 1)
                        CheckDlgButton(hwndDlg,
                            lpddData->lpci[i]->wID, BST_CHECKED);
                    else
                        CheckDlgButton(hwndDlg,
                            lpddData->lpci[i]->wID, BST_UNCHECKED);
                break;
            }
        }
        
        bRet = TRUE;
    }
    break;
    case WM_ENTERIDLE:
        // let SlackEdit redraw and do pending events, if any.
        while (Tcl_DoOneEvent(TCL_DONT_WAIT))
            /* do nothing */;
        bRet = FALSE;
    break;
    case WM_COMMAND:
    {
        LPDLGDATA lpddData = (LPDLGDATA)GetWindowLong(hwndDlg, DWL_USER);
        LPTSTR ppszCtrlElements[2], pszElement;
        BOOL fFree = FALSE;
        int i, iSel;

        switch (LOWORD(wParam))
        {
        case IDOK:
            for (i = 0; i < lpddData->nCtrlCount; i++)
            {
                switch (lpddData->lpci[i]->icType)
                {
                case IC_COMBOBOX:
                    if (lpddData->lpci[i]->cbo.bRetSel == TRUE)
                    {
                        ppszCtrlElements[1] = (LPTSTR)Mem_AllocStr(33);
                        fFree = TRUE;
                        iSel = ComboBox_GetCurSel(
                            GetDlgItem(hwndDlg,
                            lpddData->lpci[i]->wID));
                        _itot(iSel, ppszCtrlElements[1], 10);
                        break;
                    }
                case IC_EDIT:
                    ppszCtrlElements[1] = (LPTSTR)Mem_AllocStr(256);
                    fFree = TRUE;
                    GetDlgItemText(hwndDlg,
                        lpddData->lpci[i]->wID,
                        ppszCtrlElements[1], 256);
                break;
                case IC_BUTTON:
                    if (IsDlgButtonChecked(hwndDlg,
                        lpddData->lpci[i]->wID) == BST_CHECKED)
                        ppszCtrlElements[1] = _T("1");
                    else
                        ppszCtrlElements[1] = _T("0");
                break;
                default:
                continue;
                }

                ppszCtrlElements[0] = lpddData->lpci[i]->pszTextID;
                pszElement = Tcl_Merge(2, ppszCtrlElements);
                Mem_Free(lpddData->lpci[i]->pszTextID);
                Tcl_AppendElement(lpddData->TclInterp, pszElement);
                if (fFree)
                { 
                    fFree = FALSE;
                    Mem_Free(ppszCtrlElements[1]);
                }

                Mem_Free(pszElement);
            }
        /* fall through */
        case IDCANCEL:
            EndDialog(hwndDlg, uMsg);
        break;
        }  
    }
    break;
    }

    return (bRet);
}

int MyTcl_Dialog(ClientData clientData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    LPTSTR pszTitle, pszHelpFile;
    int i, j;
    int nDlgWidth, nDlgHeight;
    LPDLGITEMS lpdiItems;
    LPWORD lpw;
    int iNewMemSize, iHeaderLength;
    int iRet = TCL_OK;
    LPDLGDATA lpddData;
    Tcl_Obj **aptoDialogItems;
    int cDialogItems;

    pszTitle            = _T("");
    pszHelpFile         = NULL;
    lpdiItems           = (LPDLGITEMS)Mem_Alloc(sizeof(DLGITEMS));
    lpdiItems->nCount   = 0;

    lpddData                    = (LPDLGDATA)Mem_Alloc(sizeof(DLGDATA));
    lpddData->TclInterp         = TclInterp;
    lpddData->nCtrlCount        = 0;
    lpddData->wItemIDCounter    = IDHELP + 2;

    if (objc > 3)
    {
        BOOL fExitLoopFlag = FALSE;

        for (j = 1; !fExitLoopFlag && j < (objc - 3); j++)
        {
            LPTSTR pszArg = Tcl_GetString(objv[j]);
            if (pszArg[0] == '-')
            {
                switch (pszArg[1])
                {
                    case _T('t'): /* title */
                        pszTitle = Tcl_GetString(objv[++j]); 
                    break;
                    case _T('h'): /* helpfile */
                        pszHelpFile = Tcl_Int_DoFileName(Tcl_GetString(objv[++j]));
                    break;
                    default:
                        Tcl_AppendResult(TclInterp, Tcl_GetString(objv[0]), _T(": "), Tcl_GetString(objv[j]), _T(" is an unknown option."), NULL);

                    return (TCL_ERROR);
                }
            }
            else
            {
                fExitLoopFlag = TRUE;
                --j;
            }
        }
        
        if (j + 3 != objc)
            goto Usage;
    }
    else 
    {
Usage:
        Tcl_AppendResult(TclInterp, _T("Usage: "), Tcl_GetString(objv[0]), _T(" [-t <Title>] [-h <helpfile>] <width> <height> <DialogItemsList>."), NULL);
        if (pszHelpFile)
            Mem_Free(pszHelpFile);

        return (TCL_ERROR);
    }

    // set the width
    if (Tcl_GetIntFromObj(TclInterp, objv[j++], &nDlgWidth) != TCL_OK)
    {
        if (pszHelpFile)
            Mem_Free(pszHelpFile);

        return (TCL_ERROR);
    }

    // ensure we have the space for the OK and Cancel buttons
    if (nDlgWidth < 116) nDlgWidth = 116;

    // set the height
    if (Tcl_GetIntFromObj(TclInterp, objv[j++], &nDlgHeight) != TCL_OK)
    {
        if (pszHelpFile)
            Mem_Free(pszHelpFile);

        return (TCL_ERROR);
    }

    nDlgHeight += 16;   //add the space for the OK and Cancel buttons

    Tcl_ListObjGetElements(TclInterp, objv[objc - 1], &cDialogItems, &aptoDialogItems);

    for (i = 0; i < cDialogItems; i++)
    {
        if (DialogDec_DecodeItem(TclInterp, aptoDialogItems[i], &lpdiItems, &lpddData) != TCL_OK)
        {
            if (pszHelpFile)
                Mem_Free(pszHelpFile);

            return (TCL_ERROR);
        }
    }

    // build the dialog template header in the memory we'll
    // use for the whole template
    lpw = (LPWORD)Mem_Alloc(512);
    iHeaderLength = DialogDec_AddDlgHeader(lpw, (WORD)(cDialogItems + 2), pszTitle,
                                nDlgWidth, nDlgHeight);

    // calculate the proper memory needs for our complete template
    iNewMemSize = iHeaderLength;

    for (i = 0; i < lpdiItems->nCount; i++)
        iNewMemSize += lpdiItems->adiItems[i].nLength;

    iNewMemSize += 40;  // the size of the OK and cancel buttons
    lpw = (LPWORD)Mem_ReAlloc(lpw, (iNewMemSize * sizeof(WORD)));
    lpddData->lpdt = (LPDLGTEMPLATE)lpw;
    lpw += iHeaderLength;

    // append to the header all the item templates.
    for (i = 0; i < lpdiItems->nCount; i++)
    {
        memcpy(lpw, lpdiItems->adiItems[i].lpwMem , 
            (lpdiItems->adiItems[i].nLength * sizeof(WORD)));
        lpw += lpdiItems->adiItems[i].nLength;
        Mem_Free(lpdiItems->adiItems[i].lpwMem);
    }

    Mem_Free(lpdiItems);

    lpw += DialogDec_AddDlgItem(lpw, IC_BUTTON, BS_DEFPUSHBUTTON|WS_TABSTOP, IDOK, 0,
        ((nDlgWidth / 2) - 52), (nDlgHeight - 16), 50, 14, _T("OK"));
    lpw += DialogDec_AddDlgItem(lpw, IC_BUTTON, BS_PUSHBUTTON|WS_TABSTOP, IDCANCEL, 0,
        ((nDlgWidth / 2) + 2), (nDlgHeight - 16), 50, 14, _T("Cancel"));

    if (DialogBoxIndirectParam(g_hInstance, (LPDLGTEMPLATE)lpddData->lpdt,
                    g_hwndMain, (DLGPROC)DialogDec_DialogProc, (LPARAM)lpddData) == -1)
    {
        LPTSTR pszMessage;

        // Retrieve the error message.
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                (LPTSTR)&pszMessage, 0, NULL );
        Tcl_AppendResult(TclInterp, _T("DialogBoxIndirectParam() failed!  GetLastError() returned: "), pszMessage, (LPTSTR)NULL);
        // Free the buffer.
        LocalFree(pszMessage);
        iRet = TCL_ERROR;
    }


    //CleanUpAndLeave:
    Mem_Free(lpddData->lpdt);
    if (pszHelpFile)
        Mem_Free(pszHelpFile);
    Mem_Free(lpddData);

    return (iRet);
}

static int DialogDec_DecodeItem(Tcl_Interp *TclInterp, Tcl_Obj *ptoList, LPDLGITEMS *lpdiItems, LPDLGDATA *lpddData)
{
    int iNewMemSize;
    int j;
    BOOL fExitLoopFlag = FALSE;
    DWORD dwHelpID = 0;
    ICLASS icClass;
    CTRLTYPE ctType;
    DWORD dwFlags;
    int x, y, w, h;
    LPWORD lpw;
    WORD wCtrlID;
    BOOL bExUI = FALSE;
    BOOL bChecked = FALSE;
    int iCboSel = -1;
    Tcl_Obj **aptoCommands;
    int cCommands;

    Tcl_ListObjGetElements(TclInterp, ptoList, &cCommands, &aptoCommands);

    if (cCommands > 5)
    {
        LPTSTR pszCommand = Tcl_GetString(aptoCommands[0]);

        if (!strncmp(pszCommand, _T("combo"), 5))
        {
            ctType  = CT_COMBOBOX;
            icClass = IC_COMBOBOX;
            dwFlags = CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL | CBS_AUTOHSCROLL | WS_VSCROLL |
                        CBS_HASSTRINGS | WS_TABSTOP;
        }
        else if (!strncmp(pszCommand, _T("label"), 5))
        {
            ctType  = CT_LABEL;
            icClass = IC_STATIC;
            dwFlags = 0; //SS_NOPREFIX..why?
        }
        else if (!strncmp(pszCommand, _T("check"), 5))
        {
            ctType  = CT_CHECK;
            icClass = IC_BUTTON;
            dwFlags = BS_AUTOCHECKBOX | WS_TABSTOP;
        }
        else if (!strncmp(pszCommand, _T("edit"), 4))
        {
            ctType  = CT_EDIT;
            icClass = IC_EDIT;
            dwFlags = ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP;
        }
        else
        {
            goto Usage;
        }

        for (j = 1; !fExitLoopFlag && j < (cCommands - 6); j++)
        {
            LPTSTR pszCommand = Tcl_GetString(aptoCommands[j]);

            if (pszCommand[0] == '-')
            {
                switch (pszCommand[1])
                {
                case _T('x'): /* extended UI */
                    if (ctType != CT_COMBOBOX)
                        goto Usage;

                    bExUI = TRUE;
                break;
                case _T('h'): /* context ID */
                {
                    int nTemp;

                    if (Tcl_GetIntFromObj(TclInterp, aptoCommands[++j], &nTemp) != TCL_OK)
                        return (TCL_ERROR);

                    dwHelpID = (DWORD)nTemp;
                }
                break;
                case _T('e'):
                    if (ctType != CT_COMBOBOX)
                        goto Usage;

                    dwFlags &= ~(CBS_DROPDOWNLIST);
                    dwFlags |= CBS_DROPDOWN;
                break;
                case _T('s'):
                    if (ctType != CT_COMBOBOX)
                        goto Usage;

                    if (Tcl_GetIntFromObj(TclInterp, aptoCommands[++j], &iCboSel) != TCL_OK)
                        goto Usage;
                break;
                case _T('v'):
                {
                    BOOL fOn;
                    
                    if (ctType != CT_CHECK)
                        goto Usage;
                    
                    if (Tcl_GetBooleanFromObj(TclInterp, aptoCommands[++j], &fOn) != TCL_OK)
                        goto Usage;
                    
                    if (fOn == TRUE)
                        bChecked = TRUE;
                }
                break;
                default:
                    Tcl_AppendResult(TclInterp, _T("Unknown option in DialogItem: "),
                        pszCommand, (LPTSTR)NULL);

                return (TCL_ERROR);
                }
            }
            else
            {
                fExitLoopFlag = TRUE;
                --j;
            }
        }
    }
    else 
    {
Usage:
        Tcl_AppendResult(TclInterp, _T("Usage for a DialogItem is: <Type> [<options>] <ID> <x> <y> <width> <height> [<value>]."), NULL);

        return (TCL_ERROR);
    }

    iNewMemSize = sizeof(DLGDATA) + (sizeof(LPCTRLINFO) * ((*lpddData)->nCtrlCount + 1));
    *lpddData = (LPDLGDATA)Mem_ReAlloc(*lpddData, iNewMemSize);
    (*lpddData)->lpci[(*lpddData)->nCtrlCount] = (LPCTRLINFO)Mem_Alloc(sizeof(CTRLINFO));
    (*lpddData)->lpci[(*lpddData)->nCtrlCount]->pszTextID = String_Duplicate(Tcl_GetString(aptoCommands[j++]));
    (*lpddData)->lpci[(*lpddData)->nCtrlCount]->wID = wCtrlID = (*lpddData)->wItemIDCounter++;
    (*lpddData)->lpci[(*lpddData)->nCtrlCount]->icType = icClass;
    if (bExUI)
        (*lpddData)->lpci[(*lpddData)->nCtrlCount]->cbo.bExUI = TRUE;
    if (bChecked)
        (*lpddData)->lpci[(*lpddData)->nCtrlCount]->chk.iVal = bChecked;
    if (iCboSel != -1) 
    {
        (*lpddData)->lpci[(*lpddData)->nCtrlCount]->cbo.bRetSel = TRUE;
        (*lpddData)->lpci[(*lpddData)->nCtrlCount]->cbo.iSel = iCboSel;
    }

    (*lpddData)->nCtrlCount++;

    if (Tcl_GetIntFromObj(TclInterp, aptoCommands[j], &x) != TCL_OK)
        return (TCL_ERROR);

    if (Tcl_GetIntFromObj(TclInterp, aptoCommands[++j], &y) != TCL_OK)
        return (TCL_ERROR);

    if (Tcl_GetIntFromObj(TclInterp, aptoCommands[++j], &w) != TCL_OK)
        return (TCL_ERROR);

    if (Tcl_GetIntFromObj(TclInterp, aptoCommands[++j], &h) != TCL_OK)
        return (TCL_ERROR);

    j++;

    iNewMemSize = sizeof(DLGITEMS) + (sizeof(DLGITEM) * ((*lpdiItems)->nCount + 1));
    *lpdiItems = (LPDLGITEMS)Mem_ReAlloc(*lpdiItems, iNewMemSize);

    lpw = (LPWORD)Mem_Alloc(1024);
    ((*lpdiItems)->adiItems[(*lpdiItems)->nCount]).lpwMem = lpw;

    switch (icClass)
    {
        case IC_COMBOBOX:
            ((*lpdiItems)->adiItems[(*lpdiItems)->nCount]).nLength = 
            DialogDec_AddDlgItem(lpw, icClass, dwFlags, wCtrlID, dwHelpID, x, y, w, h, NULL);
            if (!DialogDec_FillCboList((*lpddData)->TclInterp, aptoCommands[j],
                &(*lpddData)->lpci[(*lpddData)->nCtrlCount - 1]))
            {
                goto Usage;
            }
        break;
        default:
            ((*lpdiItems)->adiItems[(*lpdiItems)->nCount]).nLength = 
            DialogDec_AddDlgItem(lpw, icClass, dwFlags, wCtrlID, dwHelpID, x, y, w, h, (cCommands - 1) == j ? String_Duplicate(Tcl_GetString(aptoCommands[j])) : NULL);
        break;

    }
    
    (*lpdiItems)->nCount++;

    return (TCL_OK);
}

static BOOL DialogDec_InitCboList(HWND hwndDlg, UINT uID, LPCTRLINFO lpci)
{
    int i;

    ComboBox_ResetContent(GetDlgItem(hwndDlg, uID));
    
    for (i = 0;i < lpci->cbo.nCount; i++)
    {
        ComboBox_AddString(GetDlgItem(hwndDlg, uID), lpci->cbo.pszString[i]);
        Mem_Free(lpci->cbo.pszString[i]);
    }

    ComboBox_SetCurSel(GetDlgItem(hwndDlg, uID), lpci->cbo.iSel);

    return (TRUE);
}

static BOOL DialogDec_FillCboList(Tcl_Interp *TclInterp, Tcl_Obj *ptoList, LPCTRLINFO *lpci)
{
    Tcl_Obj **aptoItems;
    int cItems;
    int i;

    Tcl_ListObjGetElements(TclInterp, ptoList, &cItems, &aptoItems);

    for (i = 0; i < cItems; i++)
    {
        *lpci = (LPCTRLINFO)Mem_ReAlloc(*lpci, sizeof(CTRLINFO) + (sizeof(LPTSTR) * ((*lpci)->cbo.nCount + 1)));
        (*lpci)->cbo.pszString[(*lpci)->cbo.nCount] = String_Duplicate(Tcl_GetString(aptoItems[i]));
    }

    return (TRUE);
}
