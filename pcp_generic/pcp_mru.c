/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_mru.c
 * Created    : not known (before 12/03/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:29:34
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"
#include "pcp_definitions.h"
#include <commctrl.h>

#include "pcp_mru.h"
#include "pcp_combobox.h"
#include "pcp_string.h"
#include "pcp_path.h"

static HKEY hKeyRoot = NULL;

PROCCREATEMRULIST   lpfnCreateMRUList;
PROCFREEMRULIST     lpfnFreeMRUList;
PROCADDMRUSTRING    lpfnAddMRUString;
PROCADDMRUDATA      lpfnAddMRUData;
PROCDELMRUSTRING    lpfnDelMRUString;
PROCENUMMRULIST     lpfnEnumMRUList;
PROCFINDMRUSTRING   lpfnFindMRUString;
PROCFINDMRUDATA     lpfnFindMRUData;

BOOL MRU_Initialize(void)
{
    HMODULE hComCtl32 = GetModuleHandle("COMCTL32.DLL");

    lpfnCreateMRUList   = (PROCCREATEMRULIST)GetProcAddress(hComCtl32, (LPCTSTR)151);
    lpfnFreeMRUList     = (PROCFREEMRULIST)GetProcAddress(hComCtl32, (LPCTSTR)152);
    lpfnAddMRUString    = (PROCADDMRUSTRING)GetProcAddress(hComCtl32, (LPCTSTR)153);
    lpfnAddMRUData      = (PROCADDMRUDATA)GetProcAddress(hComCtl32, (LPCTSTR)167);
    lpfnDelMRUString    = (PROCDELMRUSTRING)GetProcAddress(hComCtl32, (LPCTSTR)156);
    lpfnEnumMRUList     = (PROCENUMMRULIST)GetProcAddress(hComCtl32, (LPCTSTR)154);
    lpfnFindMRUString   = (PROCFINDMRUSTRING)GetProcAddress(hComCtl32, (LPCTSTR)155);
    lpfnFindMRUData     = (PROCFINDMRUDATA)GetProcAddress(hComCtl32, (LPCTSTR)169);

    return (TRUE);
}

HKEY MRU_SetRoot(HKEY hKey, LPCTSTR pszRoot)
{
    HKEY hKeyOld = hKeyRoot;

    if (hKeyOld != NULL)
        RegCloseKey(hKeyOld);

    RegCreateKey(hKey, pszRoot, &hKeyRoot);

    return (hKeyOld);
}

HKEY MRU_GetRoot(void)
{
    return (hKeyRoot);
}

void MRU_SetComboBox(HWND hwndCbo, LPCTSTR pszSubKey, int nMaxItems)
{
    int i;
    CREATEMRULIST cml;
    HANDLE hList;
    TCHAR szBuffer[MAX_MRU];

    ASSERT(nMaxItems <= 29);

    INITSTRUCT(cml, TRUE);
    cml.nMaxItems   = min(nMaxItems, 29);
    cml.dwFlags     = MRUF_STRING_LIST;
    cml.hKey        = hKeyRoot;
    cml.lpszSubKey  = pszSubKey;
    cml.lpfnCompare = NULL;

    hList = CreateMRUList(&cml);

    ComboBox_ResetContent(hwndCbo);

    for (i = 0; EnumMRUList(hList, i, szBuffer, MAX_MRU) != -1; i++)
        ComboBox_AddString(hwndCbo, szBuffer);

    FreeMRUList(hList);
}

void MRU_SetComboBoxEx(HWND hwndCbo, LPCTSTR pszSubKey, int nMaxItems)
{
    int i;
    CREATEMRULIST cml;
    HANDLE hList;
    TCHAR szBuffer[MAX_MRU];
    COMBOBOXEXITEM cbxi;

    ASSERT(nMaxItems <= 29);

    INITSTRUCT(cml, TRUE);
    cml.nMaxItems   = min(nMaxItems, 29);
    cml.dwFlags     = MRUF_STRING_LIST;
    cml.hKey        = hKeyRoot;
    cml.lpszSubKey  = pszSubKey;
    cml.lpfnCompare = NULL;

    hList = CreateMRUList(&cml);

    ComboBox_ResetContent(hwndCbo);

    cbxi.mask       = CBEIF_TEXT;
    cbxi.iItem      = -1;

    for (i = 0; EnumMRUList(hList, i, szBuffer, MAX_MRU) != -1; i++)
    {
        cbxi.pszText    = szBuffer;

        ComboBoxEx_InsertItem(hwndCbo, &cbxi);
    }

    FreeMRUList(hList);
}


void MRU_SetMenu(HMENU hMenu, LPCTSTR pszSubKey, int nMaxItems, UINT id)
{
    MENUITEMINFO mii;
    int i;
    TCHAR szMax[MAX_PATH] = _T("");
    TCHAR szBuffer[MAX_MRU];
    CREATEMRULIST cml;
    HANDLE hList;

    INITSTRUCT(cml, TRUE);
    cml.nMaxItems   = nMaxItems;
    cml.dwFlags     = MRUF_STRING_LIST;
    cml.hKey        = MRU_GetRoot();
    cml.lpszSubKey  = pszSubKey;
    cml.lpfnCompare = NULL;

    hList = CreateMRUList(&cml);

    mii.cbSize          = sizeof(mii);
    mii.fMask           = MIIM_TYPE | MIIM_ID;
    mii.fType           = MFT_STRING;

    for (i = 0; EnumMRUList(hList, i, szBuffer, MAX_MRU) != -1; i++)
    {
        TCHAR szItem[MAX_MRU + 10];

        Path_CompactPath(szMax, szBuffer, 50);

        if (i <= 9)
            wsprintf(szItem, _T("&%d. %s"), (i == 9) ? (0) : (i + 1), szMax);
        else
            wsprintf(szItem, _T("&%c. %s"), (TCHAR)(_T('a') - 10 + i), szMax);

        mii.wID             = id + i;
        mii.dwTypeData      = szItem;

        InsertMenuItem(hMenu, id + i, FALSE, &mii);
    }

    if (i == 0)
    {
        mii.fMask           |= MIIM_STATE;
        mii.wID             = id + i;
        mii.fState          = MF_GRAYED;
        mii.dwTypeData      = _T("Empty");

        InsertMenuItem(hMenu, id + i, FALSE, &mii);
    }
}

void MRU_Write(LPCTSTR pszSubKey, LPCTSTR pszString, int nMaxItems)
{
    CREATEMRULIST cml;
    HANDLE hList;

    ASSERT(nMaxItems <= 29);

    INITSTRUCT(cml, TRUE);
    cml.nMaxItems   = min(nMaxItems, 29);
    cml.dwFlags     = MRUF_STRING_LIST;
    cml.hKey        = hKeyRoot;
    cml.lpszSubKey  = pszSubKey;
    cml.lpfnCompare = NULL;

    hList = CreateMRUList(&cml);

    AddMRUString(hList, pszString);

    FreeMRUList(hList);
}

int CALLBACK MRUCompareString(LPCTSTR lpszString1, LPCTSTR lpszString2)
{
    return (_tcsicmp(lpszString1, lpszString2));
}

int CALLBACK MRUCompareData(LPCVOID lpData1, LPCVOID lpData2, DWORD cbData)
{
    return (1);
}

HANDLE CreateMRUList(LPCREATEMRULIST lpCreateInfo)
{
    return (lpfnCreateMRUList(lpCreateInfo));
}

void FreeMRUList(HANDLE hList)
{
    lpfnFreeMRUList(hList);
}

int AddMRUString(HANDLE hList, LPCTSTR lpszString)
{
    return (lpfnAddMRUString(hList, lpszString));
}

int AddMRUData(HANDLE hList, LPCVOID lpData, DWORD cbData)
{
    return (lpfnAddMRUData(hList, lpData, cbData));
}

BOOL DelMRUString(HANDLE hList, int nItemPos)
{
    return (lpfnDelMRUString(hList, nItemPos));
}

int EnumMRUList(HANDLE hList, int nItemPos, LPVOID lpBuffer, DWORD nBufferSize)
{
    return (lpfnEnumMRUList(hList, nItemPos, lpBuffer, nBufferSize));
}

int FindMRUString(HANDLE hList, LPCTSTR lpszString, LPINT lpRegNum)
{
    return (lpfnFindMRUString(hList, lpszString, lpRegNum));
}

int FindMRUData(HANDLE hList, LPCVOID lpData, DWORD cbData, LPINT lpRegNum)
{
    return (lpfnFindMRUData(hList, lpData, cbData, lpRegNum));
}
