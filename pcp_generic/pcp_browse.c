/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_browse.h
 * Created    : not known (before 12/22/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:31:56
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"
#include "pcp_definitions.h"
#include <shlobj.h>

#include "resource.h"

#include "pcp_string.h"
#include "pcp_path.h"

#include "pcp_browse.h"

int CALLBACK BrowseForPathUpdate(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
static LPTSTR Browse_MakeFilter(LPTSTR pszFilter);

static LPTSTR Browse_MakeFilter(LPTSTR pszFilter)
{
    for ( ; *pszFilter != _T('\0'); pszFilter++)
    {
        if (*pszFilter == _T('|'))
            *pszFilter = _T('\0');
    }

    return (pszFilter);
}

BOOL Browse_ForPath(LPBROWSEINFOEX lpbix)
{
    BROWSEINFO      bi;
    LPITEMIDLIST    pidl;
    IMalloc         *pMalloc;
    HINSTANCE       hOldResource = String_SetResourceHandle(PCPGeneric_GetHandle());

    if (!SUCCEEDED(SHGetMalloc(&pMalloc)))
        return (FALSE);

    bi.hwndOwner        = lpbix->hwndOwner;
    bi.pidlRoot         = NULL; 
    bi.pszDisplayName   = lpbix->pszPath;
    bi.lpszTitle        = (lpbix->pszTitle == NULL) ?
                        String_LoadString(IDS_BROWSEFORPATH) :
                        lpbix->pszTitle;
    bi.ulFlags          = (lpbix->dwFlags == 0) ?
                        BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT :
                        lpbix->dwFlags;
    bi.lpfn             = (BFFCALLBACK)BrowseForPathUpdate;
    bi.lParam           = (LPARAM)lpbix->pszStartPath;
    bi.iImage           = (int)NULL;

    String_SetResourceHandle(hOldResource);

    if ((pidl = SHBrowseForFolder(&bi)) == NULL)
        return (FALSE);

    if(!SHGetPathFromIDList(pidl, lpbix->pszPath))
        return (FALSE);

    pMalloc->lpVtbl->Free(pMalloc, pidl);
    pMalloc->lpVtbl->Release(pMalloc);

    return (TRUE);
}

int CALLBACK BrowseForPathUpdate(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch (uMsg)
    {
        case BFFM_INITIALIZED:
            if (lpData == 0)
                break;

            Path_VerifyBackslash((LPTSTR)lpData, (_tcslen((LPTSTR)lpData) > 3) ? FALSE : TRUE);
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        break;
        case BFFM_SELCHANGED:
        {
            TCHAR szDir[MAX_PATH];

            if (SHGetPathFromIDList((LPITEMIDLIST)lParam, szDir))
                SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
        }
        break;
    }

    return (FALSE);
}

BOOL Browse_ForFile(LPBROWSEINFOEX lpbix)
{
    OPENFILENAME ofn;
    BOOL bSuccess;

    INITSTRUCT(ofn, TRUE);

    ofn.hwndOwner           = lpbix->hwndOwner;
    ofn.lpstrFilter         = Browse_MakeFilter(lpbix->pszFilter);
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = lpbix->pszFile;
    ofn.nMaxFile            = MAX_PATH;
    ofn.nMaxFileTitle       = MAX_PATH;
    ofn.lpstrInitialDir     = lpbix->pszPath;
    ofn.lpstrTitle          = lpbix->pszTitle;
    ofn.Flags               = OFN_EXPLORER | lpbix->dwFlags;

    bSuccess = GetOpenFileName(&ofn);

    if (!bSuccess || !(ofn.Flags & OFN_ALLOWMULTISELECT) ||
        !(GetFileAttributes(lpbix->pszFile) & FILE_ATTRIBUTE_DIRECTORY) ||
        !(_tcslen(lpbix->pszFile) && Path_HasRoot(lpbix->pszFile)))
    {
        return (bSuccess);
    }

    Browse_HandleMultipleFileNames(lpbix->pszFile, lpbix->MultipleFileCallback, (LPARAM)NULL);

    return (TRUE);
}

void Browse_HandleMultipleFileNames(LPCTSTR pszFileNames, BROWSEMULTIPLEFILECALLBACK MultipleFileCallback, LPARAM lParam)
{
    TCHAR szDirectory[MAX_PATH];
    LPTSTR psz;

    _tcscpy(szDirectory, pszFileNames);
    Path_VerifyBackslash(szDirectory, TRUE);
    psz = (LPTSTR)pszFileNames;

    for ( ; ; )
    {
        TCHAR szGluedFileName[MAX_PATH];
        int nLength = _tcslen(psz);

        /* copy and strip off */
        psz += nLength + 1;
        if (*psz == _T('\0'))
            break;

        _tcscpy(szGluedFileName, szDirectory);
        _tcscat(szGluedFileName, psz);

        if (!MultipleFileCallback(szGluedFileName, lParam))
            break;
    }
}
