/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : ecp_parser.c
 * Created    : not known (before 12/27/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:17:46
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* pcp_generic */
#include <pcp_string.h>
#include <pcp_mem.h>

/* SlackEdit */
#include "../slack_main.h"
#include "parse_ecp.h"
#include "../tcl/tcl_main.h"

/* pcp_edit */
#include <pcp_edit.h>

#define ECP_MAX_LINE 8092

#define CBFUNC_COUNT 3

struct tagECPCBFUNCTION
{
    TCHAR   szName[24];
    enum { CBTITLE, CBTCLFILE, CBTCLENABLE, CBSORT } eType;
} cbFunctions[CBFUNC_COUNT] =
{
    { _T("ClipbookTitle"), CBTITLE },
    { _T("ClipbookSort"), CBSORT },
//  { _T("ClipbookLoadTclFile"), CBTCLFILE },
    { _T("ClipbookTclEnable"), CBTCLENABLE }
};

#define ITFUNC_COUNT 3

struct tagECPITFUNCTION
{
    TCHAR szName[24];
    enum { ITTITLE, ITTCL, ITBODY } eType;
} itFunctions[ITFUNC_COUNT] =
{
    { _T("ItemTitle"), ITTITLE },
    { _T("ItemTcl"), ITTCL },
    { _T("ItemBody"), ITBODY }
};

BOOL ECP_GetString(LPTSTR *pszDst, LPTSTR pszSrc, BOOL bRecurse);
BOOL ECP_GetBoolean(LPBOOL lpBool, LPTSTR pszSrc);
void ECP_AddItem(HWND hwndList, PECPITEM pItem);

BOOL ECP_GetClipbook(LPCTSTR pszFileName, PECPCLIPBOOK pClipbook)
{
    FILE *fEcpFile;
    TCHAR szLine[ECP_MAX_LINE];
    LPTSTR pszLine;

    if ((fEcpFile = fopen(pszFileName, _T("r"))) == NULL)
        goto ErrReturn;

    _tcscpy(pClipbook->szFileName, pszFileName);

    while (!feof(fEcpFile))
    {
        int i;

        _fgetts(szLine, ECP_MAX_LINE, fEcpFile);
        pszLine = szLine;

        String_TrimLeft(pszLine, -1);
        String_TrimRight(pszLine, _T('\n'));
        String_TrimRight(pszLine, -1);

        if (pszLine[0] == _T('#')) // Comment
            continue;

        for (i = 0; i < CBFUNC_COUNT; i++)
        {
            if (String_NumEqual(pszLine, cbFunctions[i].szName, _tcslen(cbFunctions[i].szName), TRUE))
            {
                pszLine += _tcslen(cbFunctions[i].szName);
                if (pszLine[0] != _T('='))
                {
                    MessageBox(g_hwndMain, _T("Error while parsing Clipbook. Missing ="), _T("Clipbook Error"), MB_OK);

                    goto ErrReturn;
                }
                pszLine++;

                switch (cbFunctions[i].eType)
                {
                case CBTITLE:
                    ECP_GetString(&pClipbook->pszTitle, pszLine, FALSE);
                break;
                case CBSORT:
                    if (!ECP_GetBoolean(&pClipbook->bSort, pszLine))
                        goto ErrReturn;
                break;
                case CBTCLENABLE:
                    if (!ECP_GetBoolean(&pClipbook->bTclFile, pszLine))
                        goto ErrReturn;
                break;
                }

                break; // out of the inner for ()
            }
        }
    }

    fclose(fEcpFile);

    return (TRUE);

ErrReturn:
    pClipbook = NULL;

    if (fEcpFile != NULL)
        fclose(fEcpFile);
    
    return (FALSE);
}

BOOL ECP_GetItems(HWND hwndList, PECPCLIPBOOK pClipbook)
{
    HANDLE hFile;
    LPTSTR pszFile, pszLine;
    PECPITEM pItem = (PECPITEM)Mem_Alloc(sizeof(ECPITEM));
    DWORD dwFileSize;

    INITSTRUCT(*pItem, FALSE);

    hFile = CreateFile(pClipbook->szFileName, GENERIC_READ,
                0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return (FALSE);

    dwFileSize = GetFileSize(hFile, NULL);
    pszFile = (LPTSTR)Mem_AllocStr(dwFileSize + SZ);
    ReadFile(hFile, pszFile, dwFileSize, &dwFileSize, NULL);
    CloseHandle(hFile);

    pszLine = _tcstok(pszFile, _T("\n"));

    do
    {
        int i;

        String_TrimLeft(pszLine, -1);

        if (*pszLine == _T('#'))
            continue;

        for (i = 0; i < ITFUNC_COUNT; i++)
        {
            if (String_NumEqual(pszLine, itFunctions[i].szName, _tcslen(itFunctions[i].szName), TRUE))
            {
                pszLine += _tcslen(itFunctions[i].szName);
                if (pszLine[0] != _T('='))
                {
                    MessageBox(g_hwndMain, _T("Error while parsing Clipbook. Missing ="), _T("Clipbook Error"), MB_OK);
                    
                    return (FALSE);
                }
                pszLine++;

                switch (itFunctions[i].eType)
                {
                case ITTITLE:
                    if (pItem->pszTitle != NULL)
                    {
                        if (pClipbook->bTclFile)
                            pItem->bTcl = TRUE;

                        ECP_AddItem(hwndList, pItem);
                        pItem = (PECPITEM)Mem_Alloc(sizeof(ECPITEM));
                        INITSTRUCT(*pItem, FALSE);
                    }

                    String_TrimRight(pszLine, _T('\r'));
                    ECP_GetString(&pItem->pszTitle, pszLine, FALSE);
                break;
                case ITTCL:
                    if (!ECP_GetBoolean(&pItem->bTcl, pszLine))
                        return (FALSE);
                break;
                case ITBODY:
                {
                    BOOL bRecurse = FALSE;

                    while (!ECP_GetString(&pItem->pszBody, pszLine, bRecurse))
                    {
                        bRecurse = TRUE;
                        if ((pszLine = _tcstok(NULL, _T("\n"))) == NULL)
                            break;
                    }
                }
                break;
                }

                break; // out of the inner for ()
            }
        }
    }
    while ((pszLine = _tcstok(NULL, _T("\n"))) != NULL);

    // Add last item
    if (pItem->pszTitle != NULL)
    {
        if (pClipbook->bTclFile)
            pItem->bTcl = TRUE;
        
        ECP_AddItem(hwndList, pItem);
    }

    return (TRUE);
}

BOOL ECP_GetString(LPTSTR *pszDst, LPTSTR pszSrc, BOOL bRecurse)
{
    LPTSTR psz;

    if (*pszDst == NULL)
    {
        psz = *pszDst = (LPTSTR)Mem_AllocStr(_tcslen(pszSrc) + SZ);
    }
    else
    {
        psz = *pszDst = (LPTSTR)Mem_ReAllocStr(*pszDst, _tcslen(*pszDst) + _tcslen(pszSrc) + SZ);
        ZeroMemory(psz + _tcslen(*pszDst), Mem_Size(*pszDst) - _tcslen(*pszDst));
    }

    if (pszSrc[0] == _T('\"') || bRecurse)
    {
        if (pszSrc[0] == _T('\"') && !bRecurse)
            pszSrc++;

        psz += _tcslen(*pszDst);

        while (*pszSrc != _T('\0'))
        {
            if (*pszSrc == _T('\\') && *(pszSrc + 1) == _T('\\'))
            {
                *(psz++) = *(++pszSrc);
                pszSrc++;
            }
            else if (*pszSrc == _T('\\') && *(pszSrc + 1) == _T('\"'))
            {
                *(psz++) = *(++pszSrc);
                pszSrc++;
            }
            else if (*pszSrc == _T('\\') && *(pszSrc + 1) == _T('n'))
            {
                *(psz++) = _T('\n');
                pszSrc += 2;
            }
            else if (*pszSrc == _T('\"'))
            {
                *pszDst = (LPTSTR)Mem_ReAllocStr(*pszDst, _tcslen(*pszDst) + SZ);

                return (TRUE);
            }
            else if (*pszSrc == _T('\r'))
            {
                *(psz++) = _T('\n');
                pszSrc++;
            }
            else
            {
                *(psz++) = *(pszSrc++);
            }
        }

        return (FALSE);
    }
    else
    {
        while (*pszSrc != _T('\0') && *pszSrc != _T(' ') && *pszSrc != _T('\t'))
        {
            if (*pszSrc == _T('\\') && *(pszSrc + 1) == _T('\\'))
            {
                *(psz++) = *(++pszSrc);
                pszSrc++;
            }
            else if (*pszSrc == _T('\\') && *(pszSrc + 1) == _T('\"'))
            {
                *(psz++) = *(++pszSrc);
                pszSrc++;
            }
            else if (*pszSrc == _T('\\') && *(pszSrc + 1) == _T('n'))
            {
                *(psz++) = _T('\n');
                pszSrc += 2;
            }
            else if (*pszSrc == _T('\r'))
            {
                *(psz++) = _T('\n');
                pszSrc++;
            }
            else
            {
                *(psz++) = *(pszSrc++);
            }
        }
    }

    *psz = _T('\0');
    *pszDst = (LPTSTR)Mem_ReAllocStr(*pszDst, _tcslen(*pszDst) + SZ);

    return (TRUE);
}

BOOL ECP_GetBoolean(LPBOOL lpBool, LPTSTR pszSrc)
{
    if (String_Equal(pszSrc, _T("TRUE"), FALSE))
    {
        *lpBool = TRUE;
    }
    else if (String_Equal(pszSrc, _T("FALSE"), FALSE))
    {
        *lpBool = FALSE;
    }
    else
    {
        MessageBox(g_hwndMain, _T("Error while parsing boolean value. Must be TRUE or FALSE."), _T("Clipbook Error"), MB_OK);

        return (FALSE);
    }

    return (TRUE);
}

void ECP_AddItem(HWND hwndList, PECPITEM pItem)
{
    if (pItem->pszBody == NULL)
    {
        // We have an empty item! try title instead
        pItem->pszBody = (LPTSTR)Mem_AllocStr(_tcslen(pItem->pszTitle) + SZ);
        _tcscpy(pItem->pszBody, pItem->pszTitle);
    }

    String_TrimRight(pItem->pszBody, _T('\n'));
    String_TrimRight(pItem->pszBody, _T('\r'));

    // i can't read this at all =)
    ListBox_SetItemData(hwndList, ListBox_AddString(hwndList, pItem->pszTitle), pItem);
    Mem_Free(pItem->pszTitle);
}

void ECP_ProcessItem(PECPITEM pItem, BOOL bSwitchFocus)
{
    LPTSTR pszCmd, psz, pszBuf;
    POINT ptOldSelStart, ptOldSelEnd, ptSelStart = { -1, -1 }, ptSelEnd = { -1, -1 };
    int yLines = 0, xChars = 0;

    if (pItem->bTcl == TRUE)
    {
        MyTcl_Eval(NULL, pItem->pszBody, TRUE);

        return;
    }
    else
    {
        pszBuf = pszCmd = (LPTSTR)Mem_AllocStr(_tcslen(pItem->pszBody) + SZ);
        psz = pItem->pszBody;
    }

    while (*psz != _T('\0'))
    {
        if (*psz == _T('\n'))
        {
            yLines++;
            xChars = -1; // Will be increased at end of while
        }
        
        if (*psz == _T('^'))
        {
            switch (*(++psz))
            {
            case _T('^'): // Escape Route
                *(pszBuf++) = *(psz++);
            break;
            case _T('|'): // Put Cursor here
                ptSelStart.y = ptSelEnd.y = yLines;
                ptSelStart.x = ptSelEnd.x = xChars;
                psz++;
            break;
            case _T('s'): // Put current selection here
            case _T('S'):
            {
                LPTSTR pszSelText = NULL;

                EditView_GetSelText(MDI_MyGetActive(TRUE), &pszSelText);

                pszCmd = (LPTSTR)Mem_ReAlloc(pszCmd,
                                            _tcslen(pItem->pszBody) +
                                            _tcslen(pszSelText) +
                                            _tcslen(pszCmd) +
                                            SZ);
                pszBuf = pszCmd + _tcslen(pszCmd);
                memcpy(pszBuf, pszSelText, _tcslen(pszSelText));
                pszBuf += _tcslen(pszSelText);
                psz++;
                        
                Mem_Free(pszSelText);
            }
            break;
            case _T('b'): // Begin selection here
            case _T('B'):
            {
                int i;
                LPTSTR pszEnd = (LPTSTR)Mem_AllocStr(_tcslen(++psz) + SZ);
                LPTSTR pszTempEnd;

                _tcscpy(pszEnd, psz);
                pszTempEnd = _tcsstr(pszEnd, _T("^e"));

                pszEnd[_tcslen(pszEnd) - _tcslen(pszTempEnd)] = _T('\0');
                psz += _tcslen(pszEnd);

                ptSelStart.y = yLines;
                ptSelStart.x = xChars;

                pszTempEnd = pszEnd;
                for (i = 0; i < (int)_tcslen(pszEnd); i++)
                {
                    if (pszEnd[i] == _T('\n'))
                        ptSelEnd.y++;

                    pszTempEnd++;
                }

                for ( ; pszTempEnd > pszEnd && *(--pszTempEnd) != _T('\n') ; )
                    ; // Empty Body

                ptSelEnd.x = _tcslen(pszTempEnd);
                ptSelEnd.x += (ptSelStart.y == ptSelEnd.y) ? ptSelStart.x : 0;

                memcpy(pszBuf, pszEnd, _tcslen(pszEnd));
                pszBuf += _tcslen(pszEnd);

                Mem_Free(pszEnd);
            }
            case _T('e'):
            case _T('E'):
                psz += 2;
            break;
            default:
                *(pszBuf++) = *(psz - 1);
                *pszBuf = *psz;
            break;
            }
        }
        else if (*psz == _T('\\'))
        {
            switch (*(++psz))
            {
//          case _T('\"'):
//          case _T('\\'):
            case _T('#'):
                *(pszBuf++) = *(psz++);
            break;
/*          case _T('n'):
            case _T('N'):
                *(pszBuf++) = _T('\r');
                *(pszBuf++) = _T('\n');
                psz++;
            break;*/
            default:
                *(pszBuf++) = *(psz - 1);
            break;
            }
        }
        else
        {
            *(pszBuf++) = *(psz++);
        }

        xChars++;
    }

    if (_tcslen(pszCmd) > 0)
        EditView_ReplaceSel(MDI_MyGetActive(TRUE), pszCmd);

    if (ptSelStart.y != -1)
    {
        EditView_GetSel(MDI_MyGetActive(TRUE), &ptOldSelStart, &ptOldSelEnd);
        ptSelStart.y += ptOldSelStart.y - yLines;
        ptSelEnd.y  += ptOldSelStart.y - yLines;
        EditView_SetSel(MDI_MyGetActive(TRUE), &ptSelStart, &ptSelEnd);
    }

    Mem_Free(pszCmd);

    if (bSwitchFocus)
        SetFocus(MDI_MyGetActive(TRUE));
}
