/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_path.c
 * Created    : 01/22/99
 * Owner      : pcppopper
 * Revised on : 07/02/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"

#include "pcp_path.h"

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

void Path_CompactPath(LPTSTR pszOut, LPCTSTR pszIn, int cchMax)
{
    if ((int)_tcslen(pszIn) < cchMax)
    {
        _tcscpy(pszOut, pszIn);

        return;
    }
    else
    {
        LPCTSTR psz = pszIn + _tcslen(pszIn);
        LPCTSTR pszBackslash;
        int nLen = _tcslen(psz);

        while (nLen < cchMax && psz > pszIn) // - '...'
        {
            psz--;
            nLen++;
        }

        pszBackslash = psz;

        pszBackslash = _tcschr(psz, _T('\\'));

//      while (*pszBackslash != _T('\\') && *pszBackslash != _T('\0'))
//          psz++;

        _tcsncpy(pszOut, pszIn, 3);
        pszOut[3] = _T('\0');
        _tcscat(pszOut, _T("..."));
        if (pszBackslash == NULL)
        {
            _tcscat(pszOut, _T("\\"));
            _tcscat(pszOut, psz);
        }
        else
        {
            _tcscat(pszOut, pszBackslash);
        }
    }
}

LPTSTR Path_VerifyBackslash(LPTSTR pszFileName, BOOL bWantBackslash)
{
    if (bWantBackslash)
    {
        if (*(pszFileName + _tcslen(pszFileName) - 1) != _T('\\'))
            _tcscat(pszFileName, _T("\\"));
    }
    else
    {
        if (*(pszFileName + _tcslen(pszFileName) - 1) == _T('\\'))
            *(pszFileName + _tcslen(pszFileName) - 1) = _T('\0');
    }

    if (_tcslen(pszFileName) == 2 && pszFileName[1] == _T(':'))
        _tcscat(pszFileName, _T("\\"));

    // return a pointer to the end of the string (for _tcscat(Path_VerifyBackslash(pszPath, TRUE), pszFileName)
    return (pszFileName + _tcslen(pszFileName) + 1);
}

LPTSTR Path_RemoveFileName(LPTSTR pszFileName)
{
    LPTSTR psz;

    psz = _tcsrchr(pszFileName, _T('\\'));

    if (psz != NULL)
        *(psz + 1) = _T('\0');

    return (pszFileName);
}

void Path_RemoveExt(LPTSTR pszFileName)
{
    LPTSTR psz;

    psz = pszFileName + _tcslen(pszFileName);

    while (psz >= pszFileName && *psz != _T('.'))
        *(psz--) = _T('\0');
}

LPTSTR Path_GetFileName(LPCTSTR pszFilePath)
{
    LPCTSTR psz = pszFilePath + _tcslen(pszFilePath);

    while (psz >= pszFilePath && *psz != _T('\\'))
        psz--;

    return ((LPTSTR)++psz);
}

LPTSTR Path_SwapExt(LPTSTR pszFileName, LPCTSTR pszNewExt)
{
    LPTSTR psz;

    psz = pszFileName + _tcslen(pszFileName);

    while (psz >= pszFileName && *psz != _T('.'))
        psz--;

    // check that the new extension is shorter or same length
    ASSERT(_tcslen(psz) >= _tcslen(pszNewExt) + ((pszNewExt[0] == _T('.')) ? 0 : 1));
    ASSERT(_tcslen(psz) + _tcslen(pszNewExt) < MAX_PATH);

    _tcscpy(((pszNewExt[0] == _T('.')) ? psz : ++psz), pszNewExt);

    return (pszFileName);
}

LPTSTR Path_GetExt(LPCTSTR pszFileName)
{
    LPCTSTR psz = pszFileName + _tcslen(pszFileName);

    while (psz >= pszFileName && *psz != _T('.'))
        psz--;

    return ((LPTSTR)++psz);
}

/*****************************************************
 * Start of the shell32.dll Path stuff rip
 *****************************************************/

BOOL Path_HasRoot(LPCTSTR pszPath)
{
    if (*(pszPath + 1) == _T(':') && *(pszPath + 2) == _T('\\')) /* X:\ */
        return (TRUE);

    if (*(pszPath) == _T('\\') && *(pszPath + 1) == _T('\\')) /* \\X\ */
    {
        int nBackslashes = 0;

        pszPath += 2;

        while (*pszPath)
        {
            if (*pszPath == _T('\\'))
                nBackslashes++;
        }

        if (nBackslashes <= 1)
            return (TRUE);
    }

    return (FALSE);
}

BOOL Path_FileExists(LPCTSTR pszFileName)
{
    DWORD dwFileAttributes = GetFileAttributes(pszFileName);

    if (dwFileAttributes == (DWORD)-1 || (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        return (FALSE);
    else
        return (TRUE);
}

BOOL Path_DirExists(LPCTSTR pszDirName)
{
    DWORD dwFileAttributes = GetFileAttributes(pszDirName);

    if (dwFileAttributes != (DWORD)-1 && (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        return (TRUE);
    else
        return (FALSE);
}

__inline DWORD Path_GetModuleFileName(HMODULE hModule, LPTSTR pszFileName, DWORD nSize)
{
    DWORD dwReturn;
    
    dwReturn = GetModuleFileName(hModule, pszFileName, nSize);

    _tcslwr(pszFileName);

    return (dwReturn);
}
