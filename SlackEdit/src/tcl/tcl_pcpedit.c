/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pp_tcl_pcpedit.c
 * Created    : not known (before 12/21/99)
 * Owner      : pcppopper
 * Revised on : 07/05/00
 * Comments   : Tcl PCPEdit command implementations 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* windows */
#include <tcl.h>
#include <commctrl.h>

/* pcp_generic */
#include <pcp_string.h>
#include <pcp_mem.h>
#include <pcp_math.h>

/* SlackEdit */
#include "../slack_main.h"
#include "tcl_internal.h"
#include "../subclasses/sub_pcp_edit.h"

/* pcp_edit */
#include <pcp_edit.h>

#define CI_WHITESPACE       _T("ci_whitespace")
#define CI_BACKGROUND       _T("ci_background")
#define CI_NORMALTEXT       _T("ci_normaltext")
#define CI_SELMARGIN        _T("ci_selmargin")
#define CI_SELBACKGROUND    _T("ci_selbackground")
#define CI_SELTEXT          _T("ci_seltext")
#define CI_PREFIXED         _T("ci_prefixed")
#define CI_KEYWORD          _T("ci_keyword")
#define CI_FUNCTIONNAME     _T("ci_functionname")
#define CI_COMMENT          _T("ci_comment")
#define CI_NUMBER           _T("ci_number")
#define CI_OPERATOR         _T("ci_operator")
#define CI_STRING           _T("ci_string")
#define CI_PREPROCESSOR     _T("ci_preprocessor")
#define CI_USER1            _T("ci_user1")
#define CI_USER2            _T("ci_user2")

static int MyTcl_ParseKeywordList(Tcl_Interp *TclInterp, LPTSTR **pszKeywordList, Tcl_Obj *ptoList);

/****************************************************************
 * Function Implementation                                      *
 ****************************************************************/

int MyTcl_GetSelText(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    LPTSTR pszSelectedText;
    Tcl_Obj *ptoszResult;
    POINT ptStart;
    POINT ptEnd;
    LPEDITINTERFACE lpInterface;

    lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

    PCP_Edit_GetSelection(lpInterface, &ptStart, &ptEnd);
    PCP_Edit_GetText(lpInterface, ptStart, ptEnd, &pszSelectedText);

    ptoszResult = Tcl_NewStringObj(pszSelectedText, _tcslen(pszSelectedText));
    Mem_Free(pszSelectedText);

    Tcl_SetObjResult(TclInterp, ptoszResult);

    return (TCL_OK);
}

int MyTcl_ReplaceSel(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    LPEDITINTERFACE lpInterface;
    int i;

    if (objc < 2)
    {
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T("<text>"));

        return (TCL_ERROR);
    }

    lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

    for (i = 1; i < objc; i++)
        PCP_Edit_ReplaceSelection(lpInterface, Tcl_GetString(objv[i]));

    return (TCL_OK);
}

int MyTcl_GetSelection(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    int nStart, nEnd;
    TCHAR szPos[16];
    Tcl_Obj *ptoList;
    Tcl_Obj *ptoArray[2];
    LPEDITINTERFACE lpInterface;

    lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

    PCP_Edit_GetSelectionPos(lpInterface, &nStart, &nEnd);

    _ltoa(nStart, szPos, 10);
    ptoArray[0] = Tcl_NewStringObj(szPos, _tcslen(szPos));
    _ltoa(nEnd, szPos, 10);
    ptoArray[1] = Tcl_NewStringObj(szPos, _tcslen(szPos));

    ptoList = Tcl_NewListObj(2, (Tcl_Obj **)ptoArray);
    Tcl_SetObjResult(TclInterp, ptoList);

    return (TCL_OK);
}

int MyTcl_SetSelection(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    int nStart, nEnd;
    LPEDITINTERFACE lpInterface;

    if (objc != 3) //have to remember that the command is included as well
    {
        Tcl_AppendResult(TclInterp, _T("Usage: "), Tcl_GetString(objv[0]),
            _T(" <beginpos> <endpos>"),
            _T(" If beginpos is larger than endpos the two will be transposed.")
            _T(" If endpos == -1 the selection will be extended to the end of the document"), NULL);

        return (TCL_ERROR);
    }

    lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

    Tcl_GetLongFromObj(TclInterp, objv[1], &nStart);
    Tcl_GetLongFromObj(TclInterp, objv[1], &nEnd);

    if (nEnd != -1 && (nStart > nEnd))
        Math_TransposeValues(nStart, nEnd);

    PCP_Edit_SetSelectionPos(lpInterface, nStart, nEnd);

    return (TCL_OK);
}

int MyTcl_GetLineCount(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    int cLines;
    Tcl_Obj *ptonResult;
    LPEDITINTERFACE lpInterface;

    lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));
 
    cLines = PCP_Edit_GetLineCount(lpInterface);

    ptonResult = Tcl_NewIntObj(cLines);
    Tcl_SetObjResult(TclInterp, ptonResult);

    return (TCL_OK);
}

int MyTcl_GetLine(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    LPCTSTR pszLine;
    int nLine;
    Tcl_Obj *ptoszResult;
    LPEDITINTERFACE lpInterface;

    if (objc != 2)
    {
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T("<line>"));

        return (TCL_ERROR);
    }

    lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

    Tcl_GetLongFromObj(TclInterp, objv[1], &nLine);
    pszLine = PCP_Edit_GetLineChars(lpInterface, nLine);
    ptoszResult = Tcl_NewStringObj(pszLine, _tcslen(pszLine));

    Tcl_SetObjResult(TclInterp, ptoszResult);

    return (TCL_OK);
}

int MyTcl_GetLineLength(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *ptonResult;
    int nLine;
    LPEDITINTERFACE lpInterface;
    
    if (objc != 2)
    {
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T("<line>"));

        return (TCL_ERROR);
    }

    lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

    Tcl_GetIntFromObj(TclInterp, objv[1], &nLine);
    ptonResult = Tcl_NewIntObj(PCP_Edit_GetLineLength(lpInterface, nLine));
    Tcl_SetObjResult(TclInterp, ptonResult);

    return (TCL_OK);
}

int MyTcl_Insert(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    int nLine;
    int nColumn;
    LPTSTR pszText;
    int nStart;
    int nEnd;
    LPEDITINTERFACE lpInterface;

    if (objc != 4)
    {
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T("<line> <column> <text>"));

        return (TCL_ERROR);
    }

    lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

    Tcl_GetIntFromObj(TclInterp, objv[1], &nLine);
    Tcl_GetIntFromObj(TclInterp, objv[2], &nColumn);
    pszText = Tcl_GetString(objv[3]);

    PCP_Edit_InsertText(lpInterface, nLine, nColumn, pszText, &nStart, &nEnd, CE_ACTION_UNKNOWN);

    return (TCL_OK);
}

int MyTcl_InsertPos(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    int nPos;
    LPTSTR pszText;
    int nStart;
    int nEnd;
    LPEDITINTERFACE lpInterface;

    if (objc != 3)
    {
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T("<pos> <text>"));

        return (TCL_ERROR);
    }

    lpInterface = PCP_Edit_GetInterface(MDI_MyGetActive(TRUE));

    Tcl_GetIntFromObj(TclInterp, objv[1], &nPos);
    pszText = Tcl_GetString(objv[2]);

    PCP_Edit_InsertTextPos(lpInterface, nPos, pszText, &nStart, &nEnd, CE_ACTION_UNKNOWN);

    return (TCL_OK);
}

//  syntax --name C/C++ --extensions c,cpp,h,hpp --flags SRC_OPT_ --tabsize 4 \
//  --opencomment / * --closecomment * / --opencomment2 XXX --closecomment2 XXX \
//  --linecomment // --linecomment2 XXX --stringchar \" --charchar \' \
//  --escapechar \\ --preprocessorchar # --functionbeginchar ( --case 1 \
//  --keywords {auto,int} --user1words {LPSTR,LPTSTR} -- user2words -- {XXX,XXX} \
//  --colors { { ci_whitespace,ffffff }, { ci_bkgnd,ffffff } }

int MyTcl_Syntax(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    SYNTAXDEFINITION sd;
    int i;

    INITSTRUCT(sd, FALSE);
    sd.cChar            = _T('\'');
    sd.crBackground     = -1;
    sd.crComment        = -1;
    sd.crFunctionName   = -1;
    sd.crKeyword        = -1;
    sd.crNormalText     = -1;
    sd.crNumber         = -1;
    sd.crOperator       = -1;
    sd.crPreprocessor   = -1;
    sd.crSelBackground  = -1;
    sd.crSelMargin      = -1;
    sd.crSelText        = -1;
    sd.crString         = -1;
    sd.crUser1          = -1;
    sd.crUser2          = -1;
    sd.crWhiteSpace     = -1;
    sd.crPrefixed       = -1;
    sd.cString          = _T('\"');
    sd.dwFlags          = SRCOPT_INSERTTABS | SRCOPT_BSATBOL | SRCOPT_SELMARGIN | SRCOPT_AUTOINDENT | SRCOPT_BRACEANSI | SRCOPT_EOLNDOS;
    sd.dwTabSize        = 4;
    sd.bCase            = TRUE;
    sd.bCommentFirstRealChar = FALSE;
    sd.bPrefixesInStrings   = FALSE;

    if (objc < 5)
    {
Usage:
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T(" <--name <language_name>>")
            _T(" <--extentions <type_file_extentions>>")
            _T(" [--flags opt_inserttabs|opt_showtabs|opt_bsatbol|opt_showmargin|opt_autoindent|opt_braceansi|opt_bracegnu|opt_eoldos|opt_eolunix|opt_eolmac|opt_fnbrace")
            _T(" [--tabsize <number>]")
            _T(" [--opencomment <string>]")
            _T(" [--closecomment <string>]")
            _T(" [--opencomment2 <string>]")
            _T(" [--closecomment2 <string>]")
            _T(" [--linecomment <string>]")
            _T(" [--linecomment2 <string>]")
            _T(" [--commentfirstonline <boolean>]")
            _T(" [--stringchar <char>]")
            _T(" [--charchar <char>]")
            _T(" [--escapechar <char>]")
            _T(" [--preprocessorchar <char>]")
            _T(" [--functionbeginchar <char>]")
            _T(" [--linecontinuationchar <char>]")
            _T(" [--lineendchar <char>]")
            _T(" [--disablechar <char>]")
            _T(" [--enablechar <char>]")
            _T(" [--case <boolean>]")
            _T(" [--prefixesinstrings <boolean>]")
            _T(" [--indentstrings <list>]")
            _T(" [--unindentstrings <list>]")
            _T(" [--prefixes <list>]")
            _T(" [--operators <string>]")
            _T(" [--keywords <list>]")
            _T(" [--user1words <list>]")
            _T(" [--user2words <list>]")
            _T(" [--colors <list [list] [list]>")
            );

        return (TCL_ERROR);
    }

    for (i = 1; (i + 1) < objc; i++)
    {
        LPTSTR pszArg = Tcl_GetString(objv[i]); // Skip --

        if (String_Equal(pszArg, _T("--name"), FALSE))
        {
            _tcsncpy(sd.szName, Tcl_GetString(objv[++i]), 256);
        }
        else if (String_Equal(pszArg, _T("--extentions"), FALSE))
        {
            _tcsncpy(sd.szExts, Tcl_GetString(objv[++i]), 256);
        }
        else if (String_Equal(pszArg, _T("--flags"), FALSE))
        {
            LPTSTR pszFlags = Tcl_GetString(objv[++i]);
            LPTSTR pszFlag = _tcstok(pszFlags, _T("|"));
            DWORD dwFlags = 0;
            BOOL    fRemove = FALSE;

            do
            {
                if (*pszFlag == _T('!'))
                {
                    pszFlag++;
                    fRemove = TRUE;
                }

                if (String_Equal(pszFlag, _T("opt_inserttabs"), FALSE))
                {
                    dwFlags |= SRCOPT_INSERTTABS;
                }
                else if (String_Equal(pszFlag, _T("opt_showtabs"), FALSE))
                {
                    dwFlags |= SRCOPT_SHOWTABS;
                }
                else if (String_Equal(pszFlag, _T("opt_bsatbol"), FALSE))
                {
                    dwFlags |= SRCOPT_BSATBOL;
                }
                else if (String_Equal(pszFlag, _T("opt_showmargin"), FALSE))
                {
                    dwFlags |= SRCOPT_SELMARGIN;
                }
                else if (String_Equal(pszFlag, _T("opt_autoindent"), FALSE))
                {
                    dwFlags |= SRCOPT_AUTOINDENT;
                }
                else if (String_Equal(pszFlag, _T("opt_braceansi"), FALSE))
                {
                    dwFlags |= SRCOPT_BRACEANSI;
                }
                else if (String_Equal(pszFlag, _T("opt_bracegnu"), FALSE))
                {
                    dwFlags |= SRCOPT_BRACEGNU;
                }
                else if (String_Equal(pszFlag, _T("opt_eoldos"), FALSE))
                {
                    dwFlags |= SRCOPT_EOLNDOS;
                    sd.dwFlags &= ~(SRCOPT_EOLNUNIX|SRCOPT_EOLNMAC);
                    fRemove = FALSE;
                }
                else if (String_Equal(pszFlag, _T("opt_eolunix"), FALSE))
                {
                    dwFlags |= SRCOPT_EOLNUNIX;
                    sd.dwFlags &= ~(SRCOPT_EOLNMAC|SRCOPT_EOLNDOS);
                    fRemove = FALSE;
                }
                else if (String_Equal(pszFlag, _T("opt_eolmac"), FALSE))
                {
                    dwFlags |= SRCOPT_EOLNMAC;
                    sd.dwFlags &= ~(SRCOPT_EOLNUNIX|SRCOPT_EOLNDOS);
                    fRemove = FALSE;
                }
                else if (String_Equal(pszFlag, _T("opt_fnbrace"), FALSE))
                {
                    dwFlags |= SRCOPT_FNBRACE;
                }

                if (dwFlags == 0)
                    goto Usage;

                if (fRemove)
                {
                    fRemove = FALSE;
                    sd.dwFlags &= ~(dwFlags);
                }
                else
                {
                    sd.dwFlags |= dwFlags;
                }
            }
            while ((pszFlag = _tcstok(NULL, _T("|"))) != NULL);
        }
        else if (String_Equal(pszArg, _T("--tabsize"), FALSE))
        {
            Tcl_GetIntFromObj(TclInterp, objv[++i], &sd.dwTabSize);
        }
        else if (String_Equal(pszArg, _T("--opencomment"), FALSE))
        {
            _tcsncpy(sd.szOpenComment, Tcl_GetString(objv[++i]), 8);
            sd.nOpenCommentLen  = _tcslen(sd.szOpenComment);
        }
        else if (String_Equal(pszArg, _T("--closecomment"), FALSE))
        {
            _tcsncpy(sd.szCloseComment, Tcl_GetString(objv[++i]), 8);
            sd.nCloseCommentLen = _tcslen(sd.szCloseComment);
        }
        else if (String_Equal(pszArg, _T("--opencomment2"), FALSE))
        {
            _tcsncpy(sd.szOpenComment2, Tcl_GetString(objv[++i]), 8);
            sd.nOpenComment2Len = _tcslen(sd.szOpenComment2);
        }
        else if (String_Equal(pszArg, _T("--closecomment2"), FALSE))
        {
            _tcsncpy(sd.szCloseComment2, Tcl_GetString(objv[++i]), 8);
            sd.nCloseComment2Len    = _tcslen(sd.szCloseComment2);
        }
        else if (String_Equal(pszArg, _T("--linecomment"), FALSE))
        {
            _tcsncpy(sd.szLineComment, Tcl_GetString(objv[++i]), 8);
            sd.nLineCommentLen  = _tcslen(sd.szLineComment);
        }
        else if (String_Equal(pszArg, _T("--linecomment2"), FALSE))
        {
            _tcsncpy(sd.szLineComment2, Tcl_GetString(objv[++i]), 8);
            sd.nLineComment2Len = _tcslen(sd.szLineComment2);
        }
        else if (String_Equal(pszArg, _T("--commentfirstonline"), FALSE))
        {
            Tcl_GetBooleanFromObj(TclInterp, objv[++i], &sd.bCommentFirstRealChar);
        }
        else if (String_Equal(pszArg, _T("--stringchar"), FALSE))
        {
            sd.cString = *(Tcl_GetString(objv[++i]));
        }
        else if (String_Equal(pszArg, _T("--charchar"), FALSE))
        {
            sd.cChar = *(Tcl_GetString(objv[++i]));
        }
        else if (String_Equal(pszArg, _T("--escapechar"), FALSE))
        {
            sd.cEscape = *(Tcl_GetString(objv[++i]));
        }
        else if (String_Equal(pszArg, _T("--preprocessorchar"), FALSE))
        {
            sd.cPreProcessor = *(Tcl_GetString(objv[++i]));
        }
        else if (String_Equal(pszArg, _T("--functionbeginchar"), FALSE))
        {
            sd.cFunctionBegin = *(Tcl_GetString(objv[++i]));
        }
        else if (String_Equal(pszArg, _T("--linecontinuationchar"), FALSE))
        {
            sd.cLineContinuation = *(Tcl_GetString(objv[++i]));
        }
        else if (String_Equal(pszArg, _T("--lineendchar"), FALSE))
        {
            sd.cLineEnd = *(Tcl_GetString(objv[++i]));
        }
        else if (String_Equal(pszArg, _T("--disablechar"), FALSE))
        {
            sd.cDisable = *(Tcl_GetString(objv[++i]));
        }
        else if (String_Equal(pszArg, _T("--enablechar"), FALSE))
        {
            sd.cEnable = *(Tcl_GetString(objv[++i]));
        }
        else if (String_Equal(pszArg, _T("--case"), FALSE))
        {
            Tcl_GetBooleanFromObj(TclInterp, objv[++i], &sd.bCase);
        }
        else if (String_Equal(pszArg, _T("--prefixesinstrings"), FALSE))
        {
            Tcl_GetBooleanFromObj(TclInterp, objv[++i], &sd.bPrefixesInStrings);
        }
        else if (String_Equal(pszArg, _T("--prefixes"), FALSE))
        {
            if (MyTcl_ParseKeywordList(TclInterp, &sd.apszPrefixes, objv[++i]) != TCL_OK)
                goto Usage;
        }
        else if (String_Equal(pszArg, _T("--operators"), FALSE))
        {
            LPTSTR pszOperators = Tcl_GetString(objv[++i]);

            if (pszOperators == NULL)
                goto Usage;

            sd.pszOperators = String_Duplicate(pszOperators);
        }
        else if (String_Equal(pszArg, _T("--keywords"), FALSE))
        {
            if (MyTcl_ParseKeywordList(TclInterp, &sd.apszKeywords, objv[++i]) != TCL_OK)
                goto Usage;
        }
        else if (String_Equal(pszArg, _T("--indentstrings"), FALSE))
        {
            if (MyTcl_ParseKeywordList(TclInterp, &sd.apszIndent, objv[++i]) != TCL_OK)
                goto Usage;
        }
        else if (String_Equal(pszArg, _T("--unindentstrings"), FALSE))
        {
            if (MyTcl_ParseKeywordList(TclInterp, &sd.apszUnIndent, objv[++i]) != TCL_OK)
                goto Usage;
        }
        else if (String_Equal(pszArg, _T("--user1words"), FALSE))
        {
            if (MyTcl_ParseKeywordList(TclInterp, &sd.apszUser1Keywords, objv[++i]) != TCL_OK)
                return (TCL_ERROR);
        }
        else if (String_Equal(pszArg, _T("--user2words"), FALSE))
        {
            if (MyTcl_ParseKeywordList(TclInterp, &sd.apszUser2Keywords, objv[++i]) != TCL_OK)
                goto Usage;
        }
        else if (String_Equal(pszArg, _T("--colors"), FALSE))
        {
            Tcl_Obj **aptoColorLists;
            int nColorItems;
            int j;

            if (Tcl_ListObjGetElements(TclInterp, objv[++i], &nColorItems, &aptoColorLists) != TCL_OK)
                goto Usage;

            for (j = 0; j < nColorItems; j++)
            {
                LPTSTR pszName;
                LPTSTR pszValue;
                Tcl_Obj **aptoColorItem;
                int nItems;

                if (Tcl_ListObjGetElements(TclInterp, aptoColorLists[j], &nItems, &aptoColorItem) != TCL_OK)
                    goto Usage;

                if (nItems > 2)
                    goto Usage;

                pszName = Tcl_GetString(aptoColorItem[0]);
                pszValue = Tcl_GetString(aptoColorItem[1]);

                if (String_Equal(pszName, CI_WHITESPACE, FALSE))
                    sd.crWhiteSpace = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_BACKGROUND, FALSE))
                    sd.crBackground = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_NORMALTEXT, FALSE))
                    sd.crNormalText = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_SELMARGIN, FALSE))
                    sd.crSelMargin = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_SELBACKGROUND, FALSE))
                    sd.crSelBackground = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_SELTEXT, FALSE))
                    sd.crSelText = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_PREFIXED, FALSE))
                    sd.crPrefixed = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_KEYWORD, FALSE))
                    sd.crKeyword = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_FUNCTIONNAME, FALSE))
                    sd.crFunctionName = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_COMMENT, FALSE))
                    sd.crComment = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_NUMBER, FALSE))
                    sd.crNumber = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_OPERATOR, FALSE))
                    sd.crOperator = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_STRING, FALSE))
                    sd.crString = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_PREPROCESSOR, FALSE))
                    sd.crPreprocessor = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_USER1, FALSE))
                    sd.crUser1 = String_MakeRGB(pszValue);
                else if (String_Equal(pszName, CI_USER2, FALSE))
                    sd.crUser2 = String_MakeRGB(pszValue);
                else
                    goto Usage;
            }
        }
        else
        {
            goto Usage;
        }
    }

    PCP_Edit_TextEdit_AddSyntaxDefintion(&sd);

    return (TCL_OK);
}

static int MyTcl_ParseKeywordList(Tcl_Interp *TclInterp, LPTSTR **pszKeywordList, Tcl_Obj *ptoList)
{
    Tcl_Obj **aptoKeywordList;
    int nListItems;
    int i;

    if (Tcl_ListObjGetElements(TclInterp, ptoList, &nListItems, &aptoKeywordList) != TCL_OK)
        return (TCL_ERROR);

    for (i = 0; i < nListItems; i++)
    {
        if (*pszKeywordList == NULL)
            *pszKeywordList = (LPTSTR *)Mem_Alloc(sizeof(LPTSTR));
        else
            *pszKeywordList = (LPTSTR *)Mem_ReAlloc(*pszKeywordList, Mem_Size(*pszKeywordList) + sizeof(LPTSTR));

        (*pszKeywordList)[i] = String_Duplicate(Tcl_GetString(aptoKeywordList[i]));
    }

    return (TCL_OK);
}
