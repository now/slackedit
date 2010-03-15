/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_registry.c
 * Created    : not known (before 09/30/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:24:14
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"
#include "pcp_registry.h"

LONG Registry_SetDW(HKEY hKey, LPTSTR lpValueName, DWORD dwData)
{
    return (RegSetValueEx(hKey, lpValueName, 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD)));
}

LONG Registry_SetSZ(HKEY hKey, LPTSTR lpValueName, LPTSTR lpData)
{
    return (RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (LPBYTE)lpData, _tcslen(lpData)));
}

DWORD Registry_GetDW(HKEY hKey, LPTSTR lpValueName, DWORD dwDefault)
{
    DWORD dwData, dwType = REG_DWORD, cbData = sizeof(DWORD);

    if (RegQueryValueEx(hKey, lpValueName, NULL, &dwType,
            (LPBYTE)&dwData, &cbData) != ERROR_SUCCESS || dwType != REG_DWORD)
        return (dwDefault);
    else
        return (dwData);
}

BOOL Registry_GetSZ(HKEY hKey, LPTSTR lpValueName, LPTSTR lpData, LPTSTR pszDefault, DWORD dwMax)
{
    DWORD dwType = REG_SZ;

    if (RegQueryValueEx(hKey, lpValueName, NULL, &dwType,
            (LPBYTE)lpData, &dwMax) != ERROR_SUCCESS || dwType != REG_SZ)
    {
        ASSERT(pszDefault != NULL);

        _tcscpy(lpData, pszDefault);

        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
