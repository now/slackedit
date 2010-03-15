/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : tcl_load.c
 * Created    : 07/19/00
 * Owner      : pcppopper
 * Revised on : 07/19/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "../pcp_generic/pcp_includes.h"

/* tcl */
#include <tcl.h>

/****************************************************************
 * Type Definitions
 ****************************************************************/

typedef Tcl_Interp *(*LPFNTCL_CREATEINTERPPROC)();

/****************************************************************
 * Function Definitions
 ****************************************************************/

static BOOL MyTcl_ActuallyLoadLibrary(LPCTSTR pszLibrary);

/****************************************************************
 * Global Variables
 ****************************************************************/

static HINSTANCE s_hTclDll = NULL;

/****************************************************************
 * Function Implementations
 ****************************************************************/

BOOL MyTcl_LoadLibrary(LPCTSTR pszMajorVersion, LPCTSTR pszMinorVersion, BOOL bExact)
{
    TCHAR szPath[MAX_PATH];
    WIN32_FIND_DATA wfd;
    HANDLE hSearch;
    HKEY hTclRootKey = NULL;
    HKEY hTclSubKey = NULL;
    TCHAR szSubKey[MAX_PATH];
    TCHAR szLatestFoundVersion[MAX_PATH];
    LPTSTR psz;
    int nMax = MAX_PATH;

    if (bExact)
    {
        _stprintf(szPath, _T(".\\tcl%s%s.dll"), pszMajorVersion, pszMinorVersion);

        hSearch = FindFirstFile(szPath, &wfd);

        if (hSearch != INVALID_HANDLE_VALUE)
        {
            FindClose(hSearch);

            return (MyTcl_ActuallyLoadLibrary(szPath));
        }
    }
    else
    {
        _stprintf(szPath, _T(".\\tcl%s?.dll"), pszMajorVersion);

        hSearch = FindFirstFile(szPath, &wfd);

        if (hSearch != INVALID_HANDLE_VALUE)
        {
            _tcscpy(szPath, wfd.cFileName);

            for ( ; FindNextFile(hSearch, &wfd); )
            {
                if (_tcsicmp(szPath, wfd.cFileName) > 0)
                    _tcscpy(szPath, wfd.cFileName);
            }

            FindClose(hSearch);

            return (MyTcl_ActuallyLoadLibrary(szPath));
        }
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Scriptics\\Tcl"), 0,
                    KEY_ENUMERATE_SUB_KEYS, &hTclRootKey) != ERROR_SUCCESS)
    {
LoadFailure:
        if (hTclRootKey != NULL)
            RegCloseKey(hTclRootKey);

        if (hTclSubKey != NULL)
            RegCloseKey(hTclSubKey);

        _stprintf(szPath, _T("Failed to load Tcl%s.%s"), pszMajorVersion, pszMinorVersion);
        _tcscat(szPath, (bExact) ? _T(".\n\n") : _T(", or greater.\n\n"));
        _tcscat(szPath, _T("Please visit http://dev.scriptics.com for further details"));

        MessageBox(NULL, szPath, _T("Failed to load Tcl"), MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);

        return (FALSE);
    }

    _stprintf(szLatestFoundVersion, _T("%s.%s"), pszMajorVersion, pszMinorVersion);

    if (!bExact)
    {
        int nMax = MAX_PATH;
        int i;

        for (i = 0; RegEnumKeyEx(hTclRootKey, i, szSubKey, &nMax, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS; i++)
        {
            if (_tcsicmp(szLatestFoundVersion, szSubKey) > 0)
                _tcscpy(szLatestFoundVersion, szSubKey);

            nMax = MAX_PATH;
        }

        if (i == 0)
            goto LoadFailure;

    }

    if (RegOpenKeyEx(hTclRootKey, szLatestFoundVersion, 0, KEY_READ, &hTclSubKey) != ERROR_SUCCESS)
        goto LoadFailure;

    if (RegQueryValueEx(hTclSubKey, _T("Root"), NULL, NULL, szPath, &nMax) != ERROR_SUCCESS)
        goto LoadFailure;

    RegCloseKey(hTclSubKey);
    RegCloseKey(hTclRootKey);

    _tcscat(szPath, _T("\\bin\\tcl"));
    psz = _tcstok(szLatestFoundVersion, _T("."));
    do
    {
        _tcscat(szPath, psz);
    } while ((psz = _tcstok(NULL, _T("."))) != NULL);
    _tcscat(szPath, _T(".dll"));

    return (MyTcl_ActuallyLoadLibrary(szPath));
}

static BOOL MyTcl_ActuallyLoadLibrary(LPCTSTR pszLibrary)
{
    Tcl_Interp *TclInterp;
    LPFNTCL_CREATEINTERPPROC lpfnTcl_CreateInterp;
    TCHAR szApplication[MAX_PATH];

    s_hTclDll   = LoadLibrary(pszLibrary);

    if (s_hTclDll <= (HINSTANCE)HINSTANCE_ERROR)
    {
        MessageBox(NULL, _T("Failed to load Tcl."), _T("Oops!"), MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);

        return (FALSE);
    }

    lpfnTcl_CreateInterp = (LPFNTCL_CREATEINTERPPROC)GetProcAddress(s_hTclDll, _T("Tcl_CreateInterp"));

    if (lpfnTcl_CreateInterp == NULL)
    {
        MessageBox(NULL, _T("Failed to create a Tcl Interpreter."), _T("Oops!"), MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);

        return (FALSE);
    }

    TclInterp = lpfnTcl_CreateInterp();

    if (Tcl_InitStubs(TclInterp, _T("8.3"), 0) == NULL)
    {
        MessageBox(NULL, _T("Failed to initialize the Tcl Stubs interface."), _T("Oops!"), MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);

        return (FALSE);
    }

    GetModuleFileName(NULL, szApplication, MAX_PATH);
    Tcl_FindExecutable(szApplication);

    Tcl_DeleteInterp(TclInterp);

    return (TRUE);
}

BOOL MyTcl_FreeLibrary(void)
{
    BOOL bSuccess = FALSE;

    if (s_hTclDll != NULL)
        bSuccess = FreeLibrary(s_hTclDll);

    return (bSuccess);
}
