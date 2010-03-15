/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_clipboard.c
 * Created    : not known (before 01/05/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:36:39
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"
#include "pcp_definitions.h"

#include "pcp_clipboard.h"
#include "pcp_string.h"
#include "pcp_mem.h"

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

void Clipboard_SetText(LPCTSTR pszText)
{
    HGLOBAL hClipboard;
    LPTSTR pClipboard;

    OpenClipboard(NULL);
    EmptyClipboard();

    hClipboard = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (_tcslen(pszText) + SZ));
    pClipboard = (LPTSTR)GlobalLock(hClipboard);

    _tcscpy(pClipboard, pszText);

    GlobalUnlock(hClipboard);
    SetClipboardData(CF_TEXT, hClipboard);
    CloseClipboard();
}

void Clipboard_AppendText(LPCTSTR pszText, BOOL bPrepend)
{
    LPTSTR lpszNewClipboard, lpszClipboard;
    HGLOBAL hGlobal;

    if (!OpenClipboard(NULL))
        return;

    if (IsClipboardFormatAvailable(CF_TEXT))
    {
        hGlobal         = GetClipboardData(CF_TEXT);
        lpszClipboard   = (LPTSTR)GlobalLock(hGlobal);

        lpszNewClipboard = (LPTSTR)Mem_AllocStr(_tcslen(lpszClipboard) +
                                            _tcslen(pszText) + SZ);
        if (!bPrepend)
        {
            _tcscpy(lpszNewClipboard, lpszClipboard);
            _tcscat(lpszNewClipboard, pszText);
        }
        else
        {
            _tcscpy(lpszNewClipboard, pszText);
            _tcscat(lpszNewClipboard, lpszClipboard);
        }

        GlobalUnlock(hGlobal);
    }
    else
    {
        lpszNewClipboard = (LPTSTR)Mem_AllocStr(_tcslen(pszText) + SZ);
    
        _tcscpy(lpszNewClipboard, pszText);
    }

    EmptyClipboard();

    hGlobal         = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (_tcslen(lpszNewClipboard) + SZ));
    lpszClipboard   = (LPTSTR)GlobalLock(hGlobal);

    _tcscpy(lpszClipboard, lpszNewClipboard);

    GlobalUnlock(hGlobal);
    SetClipboardData(CF_TEXT, hGlobal);

    CloseClipboard();

    Mem_Free(lpszNewClipboard);
}

BOOL Clipboard_GetText(LPTSTR *pszText)
{
    BOOL bSuccess = FALSE;
    
    if (OpenClipboard(NULL))
    {
        HGLOBAL hData = GetClipboardData(CF_TEXT);

        if (hData != NULL)
        {
            LPTSTR pszData = (LPTSTR)GlobalLock(hData);

            if (pszData != NULL)
            {
                if (pszText != NULL)
                {
                    *pszText = Mem_AllocStr(_tcslen(pszData));
                    _tcscpy(*pszText, pszData);
                }

                bSuccess = TRUE;

                GlobalUnlock(hData);
            }
        }

        CloseClipboard();
    }

    return (bSuccess);
}
