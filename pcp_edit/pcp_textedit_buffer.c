/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_buffer.c
 * Created    : 07/29/99
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:39:50
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* windows */
//#include <commctrl.h>

/* pcp_edit */
#include "pcp_edit.h"
#include "pcp_textedit_view.h"
#include "pcp_textedit_buffer.h"

/* pcp_generic */
#include <pcp_dpa.h>
#include <pcp_mem.h>
#include <pcp_path.h>
#include <pcp_string.h>

/****************************************************************
 * Type Definitions
 ****************************************************************/

enum
{
    UNDO_INSERT         = 0x0001,
    UNDO_BEGINGROUP     = 0x0100
};

typedef struct tagLINEINFO
{
    int     nLength;
    int     nMax;
    DWORD   dwFlags;
    LPTSTR  pszLine;
} LINEINFO, FAR *LPLINEINFO;

typedef struct tagUNDORECORD
{
    DWORD   dwFlags;
    POINT   ptStartPos;
    POINT   ptEndPos;
    int     iAction;
    union
    {
        LPTSTR pszText;
        TCHAR szText[2];
    };
} UNDORECORD, FAR *LPUNDORECORD;

#define LPLI(index)                 (((LPLINEINFO)(lpes->hdpaLines->pp[(index)])))
#define LPUR(index)                 (((LPUNDORECORD)(lpes->hdpaUndos->pp[(index)])))

#define CHAR_ALIGN                  16
#define ALIGN_BUF_SIZE(size)        ((((size) / CHAR_ALIGN) * CHAR_ALIGN) + CHAR_ALIGN)

#define UNDO_BUF_SIZE               1024

/****************************************************************
 * Function Definitions
 ****************************************************************/

void TextEdit_Buffer_InsertContext_RecalcPoint(INSERTCONTEXT ic, POINT *pt);
void TextEdit_Buffer_DeleteContext_RecalcPoint(INSERTCONTEXT dc, POINT *pt);

int TextEdit_Buffer_InsertLine(LPTEXTEDITSTATE lpes, LPCTSTR pszLine, int nLength, int nPosition);
int TextEdit_Buffer_AppendLine(LPTEXTEDITSTATE lpes, int nLineIndex, LPCTSTR pszChars, int nLength);

static int TextEdit_Buffer_FlagToIndex(DWORD dwFlag);

void UndoRecord_SetText(LPUNDORECORD lpur, LPCTSTR pszText);
LPCTSTR UndoRecord_GetText(LPUNDORECORD lpur);
void UndoRecord_FreeText(LPUNDORECORD lpur);
int TextEdit_Buffer_GetUndoCount(LPTEXTEDITSTATE lpes);
void TextEdit_Buffer_AddUndoRecord(LPTEXTEDITSTATE lpes, BOOL bInsert, const POINT *ptStartPos, const POINT *ptEndPos, LPCTSTR pszText, int nActionType);

/****************************************************************
 * Global Variables
 ****************************************************************/

static const TCHAR crlf[] = _T("\r\n");

static const LPTSTR crlfs[] =
{
    _T("\x0d\x0a"),     //DOS
    _T("\x0a"),         //UNIX
    _T("\x0d")          //MAC
};


/****************************************************************
 * Function Implementations
 ****************************************************************/

void UndoRecord_FreeText(LPUNDORECORD lpur)
{
    if (HIWORD((DWORD)lpur->pszText) != 0)
        Mem_Free(lpur->pszText);
}

LPCTSTR UndoRecord_GetText(LPUNDORECORD lpur)
{
    if (HIWORD((DWORD)lpur->pszText) != 0)
        return (lpur->pszText);
    else
        return (lpur->szText);
}

void UndoRecord_SetText(LPUNDORECORD lpur, LPCTSTR pszText)
{
    lpur->pszText = NULL;

    if (pszText != NULL && pszText[0] != _T('\0'))
    {
        int iLength = _tcslen(pszText);

        if (iLength > 1)
        {
            lpur->pszText = (LPTSTR)Mem_AllocStr(iLength);
            _tcscpy(lpur->pszText, pszText);
        }
        else
        {
            lpur->szText[0] = pszText[0];
        }
    }
}

void TextEdit_Buffer_RecalcPoint(INSERTCONTEXT ic, POINT *pt)
{
    if (ic.bInsert)
        TextEdit_Buffer_InsertContext_RecalcPoint(ic, pt);
    else
        TextEdit_Buffer_DeleteContext_RecalcPoint(ic, pt);
}

void TextEdit_Buffer_InsertContext_RecalcPoint(INSERTCONTEXT ic, POINT *pt)
{
    if (pt->y < ic.ptStart.y)
        return;
    if (pt->y > ic.ptStart.y)
    {
        pt->y += (ic.ptEnd.y - ic.ptStart.y);
        return;
    }
    if (pt->x <= ic.ptStart.x)
        return;
    pt->y += (ic.ptEnd.y - ic.ptStart.y);
    pt->x = ic.ptEnd.x + (pt->x - ic.ptStart.x);
}

void TextEdit_Buffer_DeleteContext_RecalcPoint(INSERTCONTEXT dc, POINT *pt)
{
    if (pt->y < dc.ptStart.y)
        return;
    if (pt->y > dc.ptEnd.y)
    {
        pt->y -= (dc.ptEnd.y - dc.ptStart.y);
        return;
    }
    if (pt->y == dc.ptEnd.y && pt->x >= dc.ptEnd.x)
    {
        pt->y = dc.ptStart.y;
        pt->x = dc.ptStart.x + (pt->x - dc.ptEnd.x);
        return;
    }
    if (pt->y == dc.ptStart.y)
    {
        if (pt->x > dc.ptStart.x)
            pt->x = dc.ptStart.x;
        return;
    }
    *pt = dc.ptStart;
}


int TextEdit_Buffer_InsertLine(LPTEXTEDITSTATE lpes, LPCTSTR pszLine, int nLength, int nPosition)
{
    LPLINEINFO lpli = (LPLINEINFO)Mem_Alloc(sizeof(LINEINFO));
    int cNewLines = 0;

    if (nLength == -1)
    {
        if (pszLine == NULL)
            nLength = 0;
        else
            nLength = _tcslen(pszLine);
    }

    while (nLength > TE_MAX_LINE_LENGTH)
    {
        int nOverflow = nLength - TE_MAX_LINE_LENGTH;

        cNewLines += TextEdit_Buffer_InsertLine(lpes, pszLine + (nLength - nOverflow), nOverflow, nPosition + 1);

        nLength -= nOverflow;

        cNewLines++;
    }

    lpli->nLength   = nLength;
    lpli->nMax      = ALIGN_BUF_SIZE(lpli->nLength); //nLength

    ASSERT(lpli->nMax >= lpli->nLength);

    if (lpli->nMax > 0)
        lpli->pszLine = Mem_AllocStr(lpli->nMax);
    
    if (lpli->nLength > 0)
        memcpy(lpli->pszLine, pszLine, sizeof(TCHAR) * lpli->nLength);

    DPA_InsertPtr(lpes->hdpaLines, nPosition, lpli);

    return (cNewLines);
}

int TextEdit_Buffer_AppendLine(LPTEXTEDITSTATE lpes, int nLineIndex, LPCTSTR pszChars, int nLength)
{
    register LPLINEINFO lpli;
    int nBufNeeded;
    int cNewLines = 0;

    if (nLength == -1)
    {
        if (pszChars == NULL)
            return (0);
        else
            nLength = _tcslen(pszChars);
    }

    if (nLength == 0)
        return (0);

    lpli = (LPLINEINFO)DPA_GetPtr(lpes->hdpaLines, nLineIndex);

    nBufNeeded = lpli->nLength + nLength;

    if (nBufNeeded > lpli->nMax)
    {
        LPTSTR pszNewBuf;

        lpli->nMax  = min(ALIGN_BUF_SIZE(nBufNeeded), TE_MAX_LINE_LENGTH);

        if (lpli->nMax < lpli->nLength + nLength)
        {
            int nOverflow = ((lpli->nLength + nLength) - lpli->nMax);

            cNewLines += TextEdit_Buffer_InsertLine(lpes, (pszChars + nLength - nOverflow), nOverflow, nLineIndex + 1);

            cNewLines++;

            nLength -= nOverflow;
        }

        pszNewBuf = Mem_AllocStr(lpli->nMax);
        if (lpli->nLength > 0)
            memcpy(pszNewBuf, lpli->pszLine, sizeof(TCHAR) * lpli->nLength);

        Mem_Free(lpli->pszLine);
        lpli->pszLine = pszNewBuf;
    }

    memcpy(lpli->pszLine + lpli->nLength, pszChars, sizeof(TCHAR) * nLength);
    lpli->nLength += nLength;
    memset(lpli->pszLine + lpli->nLength, _T('\0'), sizeof(TCHAR) * (lpli->nMax - lpli->nLength));

    ASSERT(lpli->nLength <= lpli->nMax);

    return (cNewLines);
}

void TextEdit_Buffer_FreeAll(LPTEXTEDITSTATE lpes)
{
    int cCount = DPA_GetCount(lpes->hdpaLines); //# Lines
    int i;
    LPLINEINFO      lpli;
    LPUNDORECORD    lpur;

    for (i = 0; i < cCount; i++)
    {
        lpli = (LPLINEINFO)DPA_GetPtr(lpes->hdpaLines, i);

        if (lpli->nMax > 0)
            Mem_Free(lpli->pszLine);
    }

    DPA_DeleteAllPtrs(lpes->hdpaLines);
//DPA_Destroy()?

    cCount = DPA_GetCount(lpes->hdpaUndos);

    for (i = 0; i < cCount; i++)
    {
        lpur = (LPUNDORECORD)DPA_GetPtr(lpes->hdpaUndos, i);
        UndoRecord_FreeText(lpur);
    }

    DPA_DeleteAllPtrs(lpes->hdpaUndos);

    DPA_Destroy(lpes->hdpaLines);
    DPA_Destroy(lpes->hdpaUndos);

    lpes->bInit = FALSE;
    lpes->ptLastChange.x = lpes->ptLastChange.y = -1;
}

BOOL TextEdit_Buffer_InitNew(LPTEXTEDITVIEW lpte, int iCrLfStyle)
{
    ASSERT(!lpte->lpes->bInit);
//  ASSERT(DPA_GetCount(lpte->lpes->hdpaLines) == 0);
    ASSERT(iCrLfStyle >= 0 && iCrLfStyle <= 2);
    lpte->lpes->hdpaLines = DPA_Create(4096);
    TextEdit_Buffer_InsertLine(lpte->lpes, _T(""), -1, -1);
    lpte->lpes->bInit               = TRUE;
    lpte->lpes->bReadOnly           = FALSE;
    lpte->lpes->iCRLFMode           = iCrLfStyle;
    lpte->lpes->bModified           = FALSE;
    lpte->lpes->nSyncPosition       = 0;
    lpte->lpes->nUndoPosition       = 0;
    lpte->lpes->bUndoGroup          = FALSE;
    lpte->lpes->bUndoBeginGroup     = FALSE;
    lpte->lpes->nUndoBufSize        = UNDO_BUF_SIZE;
    ASSERT(TextEdit_Buffer_GetUndoCount(lpte->lpes) == 0);
    lpte->lpes->ptLastChange.x = lpte->lpes->ptLastChange.y = -1;

    TextEdit_View_SetTextType(lpte, NULL);
    TextEdit_View_UpdateView(lpte, (LPINSERTCONTEXT)NULL, UPDATE_RESET, 0);

    return TRUE;
}

BOOL TextEdit_Buffer_GetReadOnly(LPTEXTEDITSTATE lpes) //LPTEXTEDITSTATE here?
{
    ASSERT(lpes->bInit);

    return lpes->bReadOnly;
}

void TextEdit_Buffer_SetReadOnly(LPTEXTEDITSTATE lpes, BOOL bReadOnly)
{
    ASSERT(lpes->bInit);

    lpes->bReadOnly = bReadOnly;
}

BOOL TextEdit_Buffer_LoadFromFile(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, LPCTSTR pszFileName, int iCrLfStyle)
{
    HANDLE hFile    = NULL;
    int nCurrentMax = 256;
    LPTSTR pszLineBuf = (LPTSTR)Mem_AllocStr(nCurrentMax);
    BOOL bSuccess = FALSE;

    ASSERT(!lpes->bInit);
//  ASSERT(DPA_GetCount(lpes->hdpaLines) == 0); //0 =( fuc...

    __try
    {
        DWORD dwFileAttributes = GetFileAttributes(pszFileName);
        int nCurrentLength = 0;
        const DWORD dwBufSize = 32768;
        LPTSTR pszBuf = (LPTSTR)Mem_AllocStr(dwBufSize); //need + SZ?
        DWORD dwCurSize;
        LPTSTR crlf;

        DWORD dwBufPtr = 0;
        int iCrLfPtr = 0;

        if (dwFileAttributes == (DWORD)-1)
            __leave;

        hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

        if (hFile == INVALID_HANDLE_VALUE) //-1
            __leave;

        if (!ReadFile(hFile, pszBuf, dwBufSize, &dwCurSize, NULL))
            __leave;

        if (iCrLfStyle == CRLF_STYLE_AUTOMATIC)
        {
            //Determine the style
            DWORD i;

            for (i = 0; i < dwCurSize; i++)
            {
                if (pszBuf[i] == _T('\x0a'))
                    break;
            }

            if (i == dwCurSize)
            {
                //Set default (DOS)
                iCrLfStyle = CRLF_STYLE_DOS;
            }
            else
            {
                //otherwise determine by scanning for line-feed char
                if (i > 0 && pszBuf[i - 1] == _T('\x0d'))
                {
                    iCrLfStyle = CRLF_STYLE_DOS;
                }
                else
                {
                    if (i < (dwCurSize - 1) && pszBuf[i + 1] == _T('\x0d'))
                        iCrLfStyle = CRLF_STYLE_MAC;
                    else
                        iCrLfStyle = CRLF_STYLE_UNIX;
                }
            }
        }

        ASSERT(iCrLfStyle >= 0 && iCrLfStyle <= 2);
        
        lpes->iCRLFMode = iCrLfStyle;
        crlf = crlfs[iCrLfStyle];

        lpes->hdpaLines = DPA_Create(4096);

        while (dwBufPtr < dwCurSize)
        {
            int c = pszBuf[dwBufPtr];
            dwBufPtr++;

            if (dwBufPtr == dwCurSize && dwCurSize == dwBufSize)
            {
                if (!ReadFile(hFile, pszBuf, dwBufSize, &dwCurSize, NULL))
                    __leave;

                dwBufPtr = 0;
            }

            pszLineBuf[nCurrentLength] = (TCHAR)c;
            nCurrentLength++;

            if (nCurrentLength == nCurrentMax)
            {
                //reallocate line buffer
                LPTSTR pszNewBuf;
                nCurrentMax += 256;
                pszNewBuf = Mem_AllocStr(nCurrentMax);
                memcpy(pszNewBuf, pszLineBuf, nCurrentLength);
                Mem_Free(pszLineBuf);
                pszLineBuf = pszNewBuf;
            }

            if ((TCHAR)c == crlf[iCrLfPtr]) //== '\r'
            {
                iCrLfPtr++;

                if (crlf[iCrLfPtr] == 0)
                {
                    pszLineBuf[nCurrentLength - iCrLfPtr] = 0;
                    TextEdit_Buffer_InsertLine(lpes, pszLineBuf, -1, -1);
                    nCurrentLength  = 0;
                    iCrLfPtr        = 0;
                }
            }
            else
            {
                iCrLfPtr = 0;
            }
        }

        pszLineBuf[nCurrentLength] = 0;
        TextEdit_Buffer_InsertLine(lpes, pszLineBuf, -1, -1);

        ASSERT(DPA_GetCount(lpes->hdpaLines) > 0);
        
        lpes->bInit         = TRUE;
        lpes->bReadOnly     = (dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
        lpes->bModified     = FALSE;
        lpes->bUndoGroup    = FALSE;
        lpes->bUndoBeginGroup = FALSE;
        lpes->nUndoBufSize  = UNDO_BUF_SIZE;
        lpes->nSyncPosition = 0;
        lpes->nUndoPosition = 0;

        ASSERT(TextEdit_Buffer_GetUndoCount(lpes) == 0);
        bSuccess = TRUE;

        TextEdit_View_SetTextType(lpte, Path_GetExt((LPTSTR)pszFileName));
        TextEdit_View_UpdateView(lpte, (LPINSERTCONTEXT)NULL, UPDATE_RESET, 0);
    }
    __finally
    {
        if (pszLineBuf != NULL)
            Mem_Free(pszLineBuf);

        if (hFile != NULL)
            CloseHandle(hFile);

        lpes->ptLastChange.x = lpes->ptLastChange.y = -1;
    }

    return (bSuccess);
}

BOOL TextEdit_Buffer_SaveToFile(LPTEXTEDITSTATE lpes, LPCTSTR pszFileName, int iCrLfStyle, BOOL bClearModifiedFlag, BOOL fBackup, LPCTSTR pszBackupFileName)
{
    HANDLE  hTempFile   = INVALID_HANDLE_VALUE;
    HANDLE  hSearch     = INVALID_HANDLE_VALUE;
    TCHAR   szTempFileDir[MAX_PATH];
    TCHAR   szTempFileName[MAX_PATH];
    BOOL    bSuccess    = FALSE;
    LPTSTR  pszCRLF;
    int     nCRLFLength;
    int     nLineCount;
    int     nLine;

    ASSERT(iCrLfStyle == CRLF_STYLE_AUTOMATIC || iCrLfStyle == CRLF_STYLE_DOS ||
            iCrLfStyle == CRLF_STYLE_UNIX || iCrLfStyle == CRLF_STYLE_MAC);
    ASSERT(lpes->bInit);

    __try
    {
        TCHAR drive[MAX_PATH], dir[MAX_PATH], name[MAX_PATH], ext[MAX_PATH];
#ifdef _UNICODE
        _wsplitpath(pszFileName, drive, dir, name, ext);
#else
        _splitpath(pszFileName, drive, dir, name, ext);
#endif
        _tcscpy(szTempFileDir, drive);
        _tcscat(szTempFileDir, dir);

        if (GetTempFileName(szTempFileDir, _T("CRE"), 0, szTempFileName) == 0)
            __leave;

        hTempFile = CreateFile(szTempFileName, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTempFile == INVALID_HANDLE_VALUE)
            __leave;

        if (iCrLfStyle == CRLF_STYLE_AUTOMATIC)
            iCrLfStyle = lpes->iCRLFMode;

        ASSERT(iCrLfStyle >= 0 && iCrLfStyle <= 2);
        pszCRLF     = crlfs[iCrLfStyle];
        nCRLFLength = _tcslen(pszCRLF);

        nLineCount = DPA_GetCount(lpes->hdpaLines);
    
        for (nLine = 0; nLine < nLineCount; nLine++)
        {
            LPLINEINFO lpli = (LPLINEINFO)DPA_GetPtr(lpes->hdpaLines, nLine);
            int nLength = lpli->nLength;
            DWORD dwWrittenBytes;
            
            if (nLength > 0)
            {
                if (!WriteFile(hTempFile,
                        lpli->pszLine,
                        nLength, &dwWrittenBytes, NULL))
                    __leave;
                if (nLength != (int)dwWrittenBytes)
                    __leave;
            }

            if (nLine < nLineCount - 1) //  Last line must not end with CRLF
            {
                if (!WriteFile(hTempFile,
                    pszCRLF, nCRLFLength, &dwWrittenBytes, NULL))
                    __leave;
                if (nCRLFLength != (int)dwWrittenBytes)
                    __leave;
            }
        }

        CloseHandle(hTempFile);
        hTempFile = INVALID_HANDLE_VALUE;

        if (fBackup && pszBackupFileName != NULL)
        {
            WIN32_FIND_DATA wfd;

            hSearch = FindFirstFile(pszFileName, &wfd);

            if (hSearch != INVALID_HANDLE_VALUE)
            {
                //  File exist - create backup file
                DeleteFile(pszBackupFileName);
                if (!MoveFile(pszFileName, pszBackupFileName))
                    __leave;
                FindClose(hSearch);
                hSearch = INVALID_HANDLE_VALUE;
            }
        }
        else
        {
            DeleteFile(pszFileName);
        }

        //  Move temporary file to target name
        if (!MoveFile(szTempFileName, pszFileName))
            __leave;

        if (bClearModifiedFlag)
        {
            TextEdit_Buffer_SetModified(lpes, FALSE);
            lpes->nSyncPosition = lpes->nUndoPosition;
        }

        bSuccess = TRUE;
    }
    __finally
    {
        if (hSearch != INVALID_HANDLE_VALUE)
            FindClose(hSearch);
        if (hTempFile != INVALID_HANDLE_VALUE)
            CloseHandle(hTempFile);
        DeleteFile(szTempFileName);
    }

    return bSuccess;
}

int TextEdit_Buffer_GetCRLFMode(LPTEXTEDITSTATE lpes)
{
    return lpes->iCRLFMode;
}

void TextEdit_Buffer_SetCRLFMode(LPTEXTEDITSTATE lpes, int iCRLFMode)
{
    ASSERT(iCRLFMode == CRLF_STYLE_DOS||
            iCRLFMode == CRLF_STYLE_UNIX ||
            iCRLFMode == CRLF_STYLE_MAC);
    lpes->iCRLFMode = iCRLFMode;
}

int TextEdit_Buffer_GetLineCount(LPTEXTEDITSTATE lpes)
{
    ASSERT(lpes->bInit);

    return DPA_GetCount(lpes->hdpaLines);
}

int TextEdit_Buffer_GetLineLength(LPTEXTEDITSTATE lpes, int iLine)
{
    ASSERT(lpes->bInit);

    return (((LPLINEINFO)(lpes->hdpaLines->pp[iLine]))->nLength);
}

LPTSTR TextEdit_Buffer_GetLineChars(LPTEXTEDITSTATE lpes, int iLine)
{
    ASSERT(lpes->bInit);

    return (LPLI(iLine)->pszLine);
}

DWORD TextEdit_Buffer_GetLineFlags(LPTEXTEDITSTATE lpes, int iLine)
{
    ASSERT(lpes->bInit);

    return (LPLI(iLine)->dwFlags);
}

static int TextEdit_Buffer_FlagToIndex(DWORD dwFlag)
{
    int iIndex = 0;

    while ((dwFlag & 1) == 0)
    {
        dwFlag = dwFlag >> 1;
        iIndex++;

        if (iIndex == 32)
            return -1;
    }

    dwFlag = dwFlag & 0xFFFFFFFE;
    if (dwFlag != 0)
        return -1;
    
    return iIndex;
}

int TextEdit_Buffer_FindLineWithFlag(LPTEXTEDITSTATE lpes, DWORD dwFlag)
{
    int iSize = DPA_GetCount(lpes->hdpaLines), iLine;

    for (iLine = 0; iLine < iSize; iLine++)
    {
        if ((LPLI(iLine)->dwFlags & dwFlag) != 0)
            return iLine;
    }

    return -1;
}

int TextEdit_Buffer_GetLineWithFlag(LPTEXTEDITSTATE lpes, DWORD dwFlag)
{
    int iFlagIndex = TextEdit_Buffer_FlagToIndex(dwFlag);

    if (iFlagIndex < 0)
    {
        ASSERT(FALSE);

        return -1;
    }

    return TextEdit_Buffer_FindLineWithFlag(lpes, dwFlag);
}

void TextEdit_Buffer_SetLineFlag(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int iLine, DWORD dwFlag, BOOL bSet, BOOL bRemoveFromPreviousLine)
{
    int iFlagIndex;
    DWORD dwNewFlags;
    LPLINEINFO lpli;

    ASSERT(lpes->bInit);

    iFlagIndex = TextEdit_Buffer_FlagToIndex(dwFlag);
    
    if (iFlagIndex < 0)
    {
        ASSERT(FALSE);

        return;
    }

    if (iLine == -1)
    {
        ASSERT(!bSet);
        
        iLine = TextEdit_Buffer_FindLineWithFlag(lpes, dwFlag);
        if (iLine == -1)
            return;
        bRemoveFromPreviousLine = FALSE;
    }

    lpli = (LPLINEINFO)DPA_GetPtr(lpes->hdpaLines, iLine);
    dwNewFlags = lpli->dwFlags;
    
    if (bSet)
        dwNewFlags = dwNewFlags | dwFlag;
    else
        dwNewFlags = dwNewFlags & ~dwFlag;

    if (lpli->dwFlags != dwNewFlags)
    {
        if (bRemoveFromPreviousLine)
        {
            int iPrevLine = TextEdit_Buffer_FindLineWithFlag(lpes, dwFlag);
            
            if (bSet)
            {
                if (iPrevLine >= 0)
                {
                    LPLINEINFO lpli2 = (LPLINEINFO)DPA_GetPtr(lpes->hdpaLines, iPrevLine);

                    ASSERT((lpli2->dwFlags & dwFlag) != 0);
                    lpli2->dwFlags &= ~dwFlag;
                    TextEdit_View_UpdateView(lpte, NULL, UPDATE_SINGLELINE | UPDATE_FLAGSONLY, iPrevLine);
                }
            }
            else
            {
                ASSERT(iPrevLine == iLine);
            }
        }

        lpli->dwFlags = dwNewFlags;
        TextEdit_View_UpdateView(lpte, NULL, UPDATE_SINGLELINE | UPDATE_FLAGSONLY, iLine);
    }
}

void TextEdit_Buffer_GetText(LPTEXTEDITSTATE lpes, int iStartLine, int iStartChar, int iEndLine, int iEndChar, LPTSTR *pszText, LPCTSTR pszCRLF)
{
    int iCRLFLength, iBufSize = 0, i;
    LPTSTR pszBuf, pszCurPos;
    register LPLINEINFO lpli;
    /*
    ASSERT(lpes->bInit);
    ASSERT(iStartLine >= 0 && iStartLine < lpes->la.nSize);
    ASSERT(iStartChar >= 0 && iStartChar <= lpes->la.liLines[iStartLine]->nLength);
    ASSERT(iEndLine >= 0 && iEndLine < lpes->la.nSize);
    ASSERT(iEndChar >= 0 && iEndChar <= lpes->la.liLines[iEndLine]->nLength);
    ASSERT(iStartLine < iEndLine || iStartLine == iEndLine && iStartChar < iEndChar);
*/
    if (pszCRLF == NULL)
        pszCRLF = crlf;
    iCRLFLength = _tcslen(pszCRLF);
    ASSERT(iCRLFLength > 0);

    for (i = iStartLine; i <= iEndLine; i++)
    {
        lpli = DPA_GetPtr(lpes->hdpaLines, i);
        iBufSize += lpli->nLength;
        iBufSize += iCRLFLength;
    }

    pszBuf      = *pszText = (LPTSTR)Mem_AllocStr(iBufSize + SZ);
    pszCurPos   = pszBuf;

    if (iStartLine < iEndLine)
    {
        int iCount;

        lpli    = DPA_GetPtr(lpes->hdpaLines, iStartLine);
        iCount  = lpli->nLength - iStartChar;
    
        if (iCount > 0)
        {
            memcpy(pszBuf, lpli->pszLine + iStartChar, sizeof(TCHAR) * iCount);
            pszBuf += iCount;
        }

        memcpy(pszBuf, pszCRLF, sizeof(TCHAR) * iCRLFLength);
        pszBuf += iCRLFLength;
        
        for (i = (iStartLine + 1); i < iEndLine; i++)
        {
            lpli = (LPLINEINFO)DPA_GetPtr(lpes->hdpaLines, i);
            iCount = lpli->nLength;
            
            if (iCount > 0)
            {
                memcpy(pszBuf, lpli->pszLine, sizeof(TCHAR) * iCount);
                pszBuf += iCount;
            }

            memcpy(pszBuf, pszCRLF, sizeof(TCHAR) * iCRLFLength);
            pszBuf += iCRLFLength;
        }

        if (iEndChar > 0)
        {
            lpli = DPA_GetPtr(lpes->hdpaLines, iEndLine);
            memcpy(pszBuf, lpli->pszLine, sizeof(TCHAR) * iEndChar);
            pszBuf += iEndChar;
        }
    }
    else
    {
        int iCount = iEndChar - iStartChar;

        lpli = (LPLINEINFO)DPA_GetPtr(lpes->hdpaLines, iStartLine);
        memcpy(pszBuf, lpli->pszLine + iStartChar, sizeof(TCHAR) * iCount);
        pszBuf += iCount;
    }

    pszBuf[0] = _T('\0');
}

BOOL TextEdit_Buffer_InternalDeleteText(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int iStartLine, int iStartChar, int iEndLine, int iEndChar)
{
    INSERTCONTEXT dcContext;

    /*
    ASSERT(lpes->bInit);
    ASSERT(iStartLine >= 0 && iStartLine < lpes->la.nSize);
    ASSERT(iStartChar >= 0 && iStartChar <= lpes->la.liLines[iStartLine]->nLength);
    ASSERT(iEndLine >= 0 && iEndLine < lpes->la.nSize);
    ASSERT(iEndChar >= 0 && iEndChar <= lpes->la.liLines[iEndLine]->nLength);
    ASSERT(iStartLine < iEndLine || iStartLine == iEndLine && iStartChar < iEndChar);
    */
    if (lpes->bReadOnly)
        return FALSE;

    dcContext.bInsert   = FALSE;
    dcContext.ptStart.y = iStartLine;
    dcContext.ptStart.x = iStartChar;
    dcContext.ptEnd.y   = iEndLine;
    dcContext.ptEnd.x   = iEndChar;

    if (iStartLine == iEndLine)
    {
        LPLINEINFO lpli = DPA_GetPtr(lpes->hdpaLines, iStartLine);

        if (iEndChar < lpli->nLength)
        {
            memcpy(lpli->pszLine + iStartChar, lpli->pszLine + iEndChar,
                    sizeof(TCHAR) * (lpli->nLength - iEndChar));
        }

        lpli->nLength -= (iEndChar - iStartChar);

        TextEdit_View_UpdateView(lpte, &dcContext, UPDATE_SINGLELINE | UPDATE_HORZRANGE, iStartLine);
    }
    else
    {
        LPLINEINFO lpli = DPA_GetPtr(lpes->hdpaLines, iEndLine);
        int iRestCount = lpli->nLength - iEndChar;
        int iDelCount, i;
        LPTSTR pszRestChars = NULL;

        if (iRestCount > 0)
        {
            pszRestChars = (LPTSTR)Mem_AllocStr(iRestCount + SZ);
            memcpy(pszRestChars, lpli->pszLine + iEndChar,
                iRestCount * sizeof(TCHAR));
        }

        iDelCount = iEndLine - iStartLine;
        
        for (i = (iStartLine + 1); i <= iEndLine; i++)
            Mem_Free(LPLI(i)->pszLine);
        
        DPA_DeleteRange(lpes->hdpaLines, iStartLine + 1, iDelCount);

        //  nEndLine is no more valid
        LPLI(iStartLine)->nLength = iStartChar;

        if (iRestCount > 0)
        {
            TextEdit_Buffer_AppendLine(lpes, iStartLine, pszRestChars, iRestCount);
            Mem_Free(pszRestChars);
        }

        TextEdit_View_UpdateView(lpte, &dcContext, UPDATE_HORZRANGE | UPDATE_VERTRANGE, iStartLine);
    }

    if (!lpes->bModified)
        TextEdit_Buffer_SetModified(lpes, TRUE);

    lpes->ptLastChange  = dcContext.ptStart;

    return (TRUE);
}

BOOL TextEdit_Buffer_InternalInsertText(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int iLine, int iPos, LPCTSTR pszText, int *iEndLine, int *iEndChar)
{
    INSERTCONTEXT icContext;
    int iRestCount, iCurrentLine;
    LPTSTR pszRestChars = NULL;
    BOOL bNewLines = FALSE;
    int iTextPos;
    int cNewLines;

    ASSERT(lpes->bInit);
    ASSERT(iLine >= 0 && iLine < DPA_GetCount(lpes->hdpaLines));
    ASSERT(iPos >= 0 && iPos <= LPLI(iLine)->nLength);

//  if (lpes->bReadOnly)
//      return (FALSE);

    icContext.bInsert   = TRUE;
    icContext.ptStart.x = iPos;
    icContext.ptStart.y = iLine;

    iRestCount = LPLI(iLine)->nLength - iPos;

    if (iRestCount > 0)
    {
        pszRestChars = (LPTSTR)Mem_AllocStr(iRestCount + SZ);
        memcpy(pszRestChars, LPLI(iLine)->pszLine + iPos,
            iRestCount * sizeof(TCHAR));
        LPLI(iLine)->nLength = iPos;
    }

    iCurrentLine = iLine;
    for (;;)
    {
        iTextPos = 0;
        while (pszText[iTextPos] != _T('\0') && pszText[iTextPos] != _T('\r') && pszText[iTextPos] != _T('\n'))
            iTextPos++;

        if (iCurrentLine == iLine)
        {
            
            cNewLines = TextEdit_Buffer_AppendLine(lpes, iLine, pszText, iTextPos);

            if (cNewLines > 0)
            {
                bNewLines = TRUE;
                iCurrentLine += cNewLines;
            }
        }
        else
        {
            cNewLines = TextEdit_Buffer_InsertLine(lpes, pszText, iTextPos, iCurrentLine);
            
            bNewLines = TRUE;

            if (cNewLines > 0)
                iCurrentLine += cNewLines;
        }

        if (pszText[iTextPos] == _T('\0'))
        {
            *iEndLine = iCurrentLine;
            *iEndChar = LPLI(iCurrentLine)->nLength;
            cNewLines = TextEdit_Buffer_AppendLine(lpes, iCurrentLine, pszRestChars, iRestCount);

            if (cNewLines > 0)
            {
                bNewLines = TRUE;
                iCurrentLine += cNewLines;
            }

            break;
        }

        iCurrentLine++;
//      iTextPos++;

        if (pszText[iTextPos] == _T('\n') || pszText[++iTextPos] == _T('\n'))
        {
            iTextPos++;
        }
        else
        {
            ASSERT(FALSE);          //  Invalid line-end format passed
        }

        pszText += iTextPos;
    }

    if (pszRestChars != NULL)
        Mem_Free(pszRestChars);

    icContext.ptEnd.x = (long)*iEndChar;
    icContext.ptEnd.y = (long)*iEndLine;

    if (bNewLines)
        TextEdit_View_UpdateView(lpte, &icContext, UPDATE_HORZRANGE | UPDATE_VERTRANGE, iLine);
    else
        TextEdit_View_UpdateView(lpte, &icContext, UPDATE_SINGLELINE | UPDATE_HORZRANGE, iLine);

    if (!lpes->bModified)
        TextEdit_Buffer_SetModified(lpes, TRUE);

    lpes->ptLastChange  = icContext.ptEnd;

    return (TRUE);
}

BOOL TextEdit_Buffer_CanUndo(LPTEXTEDITSTATE lpes)
{
    ASSERT(lpes->nUndoPosition >= 0 && lpes->nUndoPosition <= DPA_GetCount(lpes->hdpaUndos));
    return (lpes->nUndoPosition > 0);
}

BOOL TextEdit_Buffer_CanRedo(LPTEXTEDITSTATE lpes)
{
    int iCount = DPA_GetCount(lpes->hdpaUndos);
    
    ASSERT(lpes->nUndoPosition >= 0 && lpes->nUndoPosition <= iCount);
    
    return (lpes->nUndoPosition < iCount);
}

int TextEdit_Buffer_GetUndoDescription(LPTEXTEDITSTATE lpes, LPTSTR pszDesc, int iPos)
{
    int iPosition;

    ASSERT(TextEdit_Buffer_CanUndo(lpes));
    ASSERT((LPUR(0)->dwFlags & UNDO_BEGINGROUP) != 0);

    if (iPos == 0)
    {
        //  Start from beginning
        iPosition = lpes->nUndoPosition;
    }
    else
    {
        iPosition = iPos;
        ASSERT(iPosition > 0 && iPosition < lpes->nUndoPosition);
        ASSERT((LPUR(iPosition)->dwFlags & UNDO_BEGINGROUP) != 0);
    }

    //  Advance to next undo group
    iPosition--;

    while ((LPUR(iPosition)->dwFlags & UNDO_BEGINGROUP) == 0)
        iPosition--;

    //  Read description
    if (!PCP_Edit_GetActionDescription(LPUR(iPosition)->iAction, pszDesc))
        pszDesc[0] = _T('\0');      //  Use empty string as description

    //  Now, if we stop at zero position, this will be the last action,
    //  since we return (POSITION) nPosition
    return (iPosition);
}

int TextEdit_Buffer_GetRedoDescription(LPTEXTEDITSTATE lpes, LPTSTR pszDesc, int iPos)
{
    int iPosition;
    ASSERT(TextEdit_Buffer_CanRedo(lpes));
    ASSERT((LPUR(0)->dwFlags & UNDO_BEGINGROUP) != 0);
    ASSERT((LPUR(lpes->nUndoPosition)->dwFlags & UNDO_BEGINGROUP) != 0);

    if (iPos == 0)
    {
        //  Start from beginning
        iPosition = lpes->nUndoPosition;
    }
    else
    {
        iPosition = iPos;
        ASSERT(iPosition > lpes->nUndoPosition);
        ASSERT((LPUR(iPosition)->dwFlags & UNDO_BEGINGROUP) != 0);
    }

    //  Read description
    if (!PCP_Edit_GetActionDescription(LPUR(iPosition)->iAction, pszDesc))
        pszDesc[0] = _T('\0');      //  Use empty string as description

    //  Advance to next undo group
    iPosition++;

    while (iPosition < DPA_GetCount(lpes->hdpaUndos) &&
        (LPUR(iPosition)->dwFlags & UNDO_BEGINGROUP) == 0)
        iPosition--;

    if (iPosition >= DPA_GetCount(lpes->hdpaUndos))
        return (0); //  No more redo actions!

    return (iPosition);
}

BOOL TextEdit_Buffer_Undo(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, POINT *ptCursorPos)
{
    ASSERT(TextEdit_Buffer_CanUndo(lpes));
    ASSERT((LPUR(0)->dwFlags & UNDO_BEGINGROUP) != 0);

    for (;;)
    {
        const LPUNDORECORD lpur = LPUR(--lpes->nUndoPosition);

        if (lpur->dwFlags & UNDO_INSERT)
        {
            VERIFY(TextEdit_Buffer_InternalDeleteText(lpte, lpes, lpur->ptStartPos.y, lpur->ptStartPos.x, lpur->ptEndPos.y, lpur->ptEndPos.x));
            *ptCursorPos = lpur->ptStartPos;
        }
        else
        {
            INT iEndLine, iEndChar;
            ASSERT(lpes->bReadOnly != TRUE);
            VERIFY(TextEdit_Buffer_InternalInsertText(lpte, lpes, lpur->ptStartPos.y, lpur->ptStartPos.x, UndoRecord_GetText(lpur), &iEndLine, &iEndChar));
            *ptCursorPos = lpur->ptEndPos;
        }

        if (lpur->dwFlags & UNDO_BEGINGROUP)
            break;
    }
    if (lpes->bModified && lpes->nSyncPosition == lpes->nUndoPosition)
        TextEdit_Buffer_SetModified(lpes, FALSE);
    if (!lpes->bModified && lpes->nSyncPosition != lpes->nUndoPosition)
        TextEdit_Buffer_SetModified(lpes, TRUE);
    return (TRUE);
}

BOOL TextEdit_Buffer_Redo(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, POINT *ptCursorPos)
{
    ASSERT(TextEdit_Buffer_CanRedo(lpes));
    ASSERT((LPUR(0)->dwFlags & UNDO_BEGINGROUP) != 0);
    ASSERT((LPUR(lpes->nUndoPosition)->dwFlags & UNDO_BEGINGROUP) != 0);

    for (;;)
    {
        const LPUNDORECORD lpur = (LPUNDORECORD)DPA_GetPtr(lpes->hdpaUndos, lpes->nUndoPosition);

        if (lpur->dwFlags & UNDO_INSERT)
        {
            INT iEndLine, iEndChar;
            ASSERT(lpes->bReadOnly != TRUE);
            VERIFY(TextEdit_Buffer_InternalInsertText(lpte, lpes, lpur->ptStartPos.y, lpur->ptStartPos.x, UndoRecord_GetText(lpur), &iEndLine, &iEndChar));
            *ptCursorPos = lpur->ptEndPos;
        }
        else
        {
            VERIFY(TextEdit_Buffer_InternalDeleteText(lpte, lpes, lpur->ptStartPos.y, lpur->ptStartPos.x, lpur->ptEndPos.y, lpur->ptEndPos.x));
            *ptCursorPos = lpur->ptStartPos;
        }

        lpes->nUndoPosition++;
        
        if (lpes->nUndoPosition == DPA_GetCount(lpes->hdpaUndos))
            break;
        
        if ((LPUR(lpes->nUndoPosition)->dwFlags & UNDO_BEGINGROUP) != 0)
            break;
    }
    if (lpes->bModified && lpes->nSyncPosition == lpes->nUndoPosition)
        TextEdit_Buffer_SetModified(lpes, FALSE);
    if (!lpes->bModified && lpes->nSyncPosition != lpes->nUndoPosition)
        TextEdit_Buffer_SetModified(lpes, TRUE);
    return (TRUE);
}

void TextEdit_Buffer_AddUndoRecord(LPTEXTEDITSTATE lpes, BOOL bInsert, const POINT *ptStartPos, const POINT *ptEndPos, LPCTSTR pszText, int nActionType)
{
    int iBufSize;
    LPUNDORECORD lpur = (LPUNDORECORD)Mem_Alloc(sizeof(UNDORECORD));

    ASSERT(lpes->bUndoGroup);
    ASSERT(DPA_GetCount(lpes->hdpaUndos) == 0 || (LPUR(0)->dwFlags & UNDO_BEGINGROUP) != 0);

    if (lpes->hdpaUndos == NULL)
        lpes->hdpaUndos = DPA_Create(64);

    //  Strip unnecessary undo records (edit after undo)
    iBufSize = DPA_GetCount(lpes->hdpaUndos);
    if (lpes->nUndoPosition < iBufSize)
    {
        int i;
        for (i = lpes->nUndoPosition; i < iBufSize; i++)
            UndoRecord_FreeText(LPUR(i));
        lpes->hdpaUndos->cCount = lpes->nUndoPosition; //FIXME:OR?
    }
    
    //  If undo buffer size is close to critical, remove the oldest records
    ASSERT(DPA_GetCount(lpes->hdpaUndos) <= lpes->nUndoBufSize);
    iBufSize = DPA_GetCount(lpes->hdpaUndos);
    if (iBufSize >= lpes->nUndoBufSize)
    {
        int iIndex = 0;
        
        for (;;)
        {
            UndoRecord_FreeText(LPUR(iIndex));
            
            iIndex++;
        
            if (iIndex == iBufSize || (LPUR(iIndex)->dwFlags & UNDO_BEGINGROUP) != 0)
                break;
        }
    
        DPA_DeleteRange(lpes->hdpaUndos, 0, iIndex);
    }

    ASSERT(DPA_GetCount(lpes->hdpaUndos) < lpes->nUndoBufSize);
    
    //  Add new record
    lpur->dwFlags = bInsert ? UNDO_INSERT : 0;
    lpur->iAction = nActionType;

    if (lpes->bUndoBeginGroup)
    {
        lpur->dwFlags |= UNDO_BEGINGROUP;
        lpes->bUndoBeginGroup = FALSE;
    }

    lpur->ptStartPos    = *ptStartPos;
    lpur->ptEndPos      = *ptEndPos;
    UndoRecord_SetText(lpur, (LPCTSTR)pszText);
    
    DPA_InsertPtr(lpes->hdpaUndos, -1, lpur); //Append
    lpes->nUndoPosition = DPA_GetCount(lpes->hdpaUndos);

    ASSERT(DPA_GetCount(lpes->hdpaUndos) <= lpes->nUndoBufSize);
}

BOOL TextEdit_Buffer_InsertText(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int iLine, int iPos, LPCTSTR pszText, int *iEndLine, int *iEndChar, int iAction)
{
    BOOL bGroupFlag = FALSE;
    POINT ptStart, ptEnd;

    if (lpes->bReadOnly && iAction != PE_ACTION_INTERNALINSERT)
        return (FALSE);

    if (!TextEdit_Buffer_InternalInsertText(lpte, lpes, iLine, iPos, pszText, iEndLine, iEndChar))
        return (FALSE);

    if (!lpes->bUndoGroup)
    {
        TextEdit_Buffer_BeginUndoGroup(lpes, FALSE);
        bGroupFlag = TRUE;
    }

    ptStart.x   = iPos;
    ptStart.y   = iLine;
    ptEnd.x     = (long)*iEndChar;
    ptEnd.y     = (long)*iEndLine;
    TextEdit_Buffer_AddUndoRecord(lpes, TRUE, &ptStart, &ptEnd, pszText, iAction);

    if (bGroupFlag)
        TextEdit_Buffer_FlushUndoGroup(lpte, lpes);

    return (TRUE);
}

BOOL TextEdit_Buffer_DeleteText(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int iStartLine, int iStartChar, int iEndLine, int iEndChar, int iAction)
{
    LPTSTR  pszTextToDelete;
    BOOL bGroupFlag = FALSE;
    POINT ptStart, ptEnd;

    TextEdit_Buffer_GetText(lpes, iStartLine, iStartChar, iEndLine, iEndChar, &pszTextToDelete, NULL);

    if (!TextEdit_Buffer_InternalDeleteText(lpte, lpes, iStartLine, iStartChar, iEndLine, iEndChar))
        return (FALSE);

    if (!lpes->bUndoGroup)
    {
        TextEdit_Buffer_BeginUndoGroup(lpes, FALSE);
        bGroupFlag = TRUE;
    }

    ptStart.x   = iStartChar;
    ptStart.y   = iStartLine;
    ptEnd.x     = iEndChar;
    ptEnd.y     = iEndLine;
    TextEdit_Buffer_AddUndoRecord(lpes, FALSE, &ptStart, &ptEnd, pszTextToDelete, iAction);

    if (bGroupFlag)
        TextEdit_Buffer_FlushUndoGroup(lpte, lpes);

    Mem_Free(pszTextToDelete);

    return (TRUE);
}

BOOL TextEdit_Buffer_InsertTextPos(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int nPos, LPCTSTR pszText, int nAction)
{
    POINT ptPos = { 0, 0 };
    int nCurrentPos = 0;
    int nLineCount = TextEdit_Buffer_GetLineCount(lpes);
    BOOL bEdit = TRUE;
    int nDummy;
    int nDummy2;

    while (nCurrentPos < nPos)
    {
        if (ptPos.y >= nLineCount)
        {
            ptPos.y = nLineCount - 1;
            ptPos.x = TextEdit_Buffer_GetLineLength(lpes, ptPos.y);
            bEdit = FALSE;

            break;
        }

        nCurrentPos += TextEdit_Buffer_GetLineLength(lpes, ptPos.y++);
    }

    if (bEdit)
    {
        nCurrentPos -= (nCurrentPos - nPos);
        ptPos.x = nCurrentPos;
        ptPos.y -= (ptPos.y == 0) ? 0 : 1;
    }

    return (TextEdit_Buffer_InsertText(lpte, lpes, ptPos.y, ptPos.x, pszText, &nDummy, &nDummy2, nAction));
}

BOOL TextEdit_Buffer_DeleteTextPos(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes, int nStartPos, int nEndPos, int nAction)
{
    POINT ptStartPos = { 0, 0 }, ptEndPos;
    int nCurrentPos = 0;
    int nLineCount = TextEdit_Buffer_GetLineCount(lpes);
    BOOL bEdit = TRUE;

    if (nEndPos != -1)
        ASSERT(nStartPos <= nEndPos);

    while (nCurrentPos < nStartPos)
    {
        if (ptStartPos.y >= nLineCount)
        {
            ptStartPos.y = nLineCount - 1;
            ptStartPos.x = TextEdit_Buffer_GetLineLength(lpes, ptStartPos.y);
            bEdit = FALSE;

            break;
        }

        nCurrentPos += TextEdit_Buffer_GetLineLength(lpes, ptStartPos.y++);
    }

    if (bEdit)
    {
        nCurrentPos -= (nCurrentPos - nStartPos);
        ptStartPos.x = nCurrentPos;
        ptStartPos.y -= (ptStartPos.y == 0) ? 0 : 1;
    }
    else
    {
        bEdit = TRUE;
    }

    if (nEndPos == -1)
    {
        ptEndPos.y = nLineCount - 1;
        ptEndPos.x = TextEdit_Buffer_GetLineLength(lpes, ptEndPos.y);
        bEdit = FALSE;
    }
    else
    {
        while (nCurrentPos < nEndPos)
        {
            if (ptEndPos.y >= nLineCount)
            {
                ptEndPos.y = nLineCount - 1;
                ptEndPos.x = TextEdit_Buffer_GetLineLength(lpes, ptEndPos.y);
                bEdit = FALSE;

                break;
            }

            nCurrentPos += TextEdit_Buffer_GetLineLength(lpes, ptEndPos.y++);
        }
    }

    if (bEdit)
    {
        nCurrentPos -= (nCurrentPos - nEndPos);
        ptEndPos.x = nCurrentPos;
        ptEndPos.y -= (ptEndPos.y == 0) ? 0 : 1;
    }

    return (TextEdit_Buffer_DeleteText(lpte, lpes, ptStartPos.y, ptStartPos.x, ptEndPos.y, ptEndPos.x, nAction));
}

void TextEdit_Buffer_SetModified(LPTEXTEDITSTATE lpes, BOOL bModified)
{
    lpes->bModified = bModified;
}

BOOL TextEdit_Buffer_GetModified(LPTEXTEDITSTATE lpes)
{
    return (lpes->bModified);
}

void TextEdit_Buffer_BeginUndoGroup(LPTEXTEDITSTATE lpes, BOOL bMergeWithPrevious /*= FALSE*/)
{
    ASSERT(!lpes->bUndoGroup);
    lpes->bUndoGroup = TRUE;
    lpes->bUndoBeginGroup = lpes->nUndoPosition == 0 || !bMergeWithPrevious;
}

void TextEdit_Buffer_FlushUndoGroup(LPTEXTEDITVIEW lpte, LPTEXTEDITSTATE lpes)
{
    ASSERT(lpes->bUndoGroup);

    if (lpes != NULL)
    {
        ASSERT(lpes->nUndoPosition == DPA_GetCount(lpes->hdpaUndos));
        
        if (lpes->nUndoPosition > 0)
        {
            LPUNDORECORD lpur = DPA_GetPtr(lpes->hdpaUndos, lpes->nUndoPosition - 1);

            lpes->bUndoBeginGroup = TRUE;
            TextEdit_View_OnEditOperation(lpte, lpur->iAction, UndoRecord_GetText(lpur));
        }
    }

    lpes->bUndoGroup = FALSE;
}

int TextEdit_Buffer_GetUndoCount(LPTEXTEDITSTATE lpes)
{
//  ASSERT(TextEdit_Buffer_CanUndo(lpes));
    
    return DPA_GetCount(lpes->hdpaUndos);
}

int TextEdit_Buffer_FindNextBookmarkLine(LPTEXTEDITSTATE lpes, int iCurrentLine)
{
    BOOL bWrapIt = TRUE;
    DWORD dwFlags = TextEdit_Buffer_GetLineFlags(lpes, iCurrentLine);
    int iSize;

    if ((dwFlags & LF_BOOKMARKS) != 0)
        iCurrentLine++;

    iSize = DPA_GetCount(lpes->hdpaLines);
    
    for (;;)
    { 
        while (iCurrentLine < iSize)
        {
            if ((LPLI(iCurrentLine)->dwFlags & LF_BOOKMARKS) != 0)
                return (iCurrentLine);
            // Keep going
            iCurrentLine++;
        }
        // End of text reached
        if (!bWrapIt)
            return (-1);

        // Start from the beginning of text
        bWrapIt = FALSE;
        iCurrentLine = 0;
    }

    return (-1);
}

int TextEdit_Buffer_FindPrevBookmarkLine(LPTEXTEDITSTATE lpes, int iCurrentLine)
{
    BOOL bWrapIt = TRUE;
    DWORD dwFlags = TextEdit_Buffer_GetLineFlags(lpes, iCurrentLine);
    int iSize;
    if ((dwFlags & LF_BOOKMARKS) != 0)
        iCurrentLine--;

    iSize = DPA_GetCount(lpes->hdpaLines);

    for (;;)
    {
        while (iCurrentLine >= 0)
        {
            if ((LPLI(iCurrentLine)->dwFlags & LF_BOOKMARKS) != 0)
                return (iCurrentLine);
            // Keep moving up
            iCurrentLine--;
        }

        // Beginning of text reached
        if (!bWrapIt)
            return (-1);

        // Start from the end of text
        bWrapIt = FALSE;
        iCurrentLine = iSize - 1;
    }
    return (-1);
}

POINT TextEdit_Buffer_GetLastChangePos(LPTEXTEDITSTATE lpes)
{
    return (lpes->ptLastChange);
}

int TextEdit_Buffer_GetLastActionDescription(LPTEXTEDITSTATE lpes)
{
    int nCount = TextEdit_Buffer_GetUndoCount(lpes);

    return ((nCount) ? LPUR(nCount - 1)->iAction : 0);
}
