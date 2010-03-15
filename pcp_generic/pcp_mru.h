/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_mru.h
 * Created    : not known (before 12/03/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:29:45
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_MRU_H
#define __PCP_MRU_H

#include "pcp_generic.h"

#define MRUF_STRING_LIST	0
#define MRUF_BINARY_LIST	1
#define MRUF_DELAYED_SAVE	2

#define MAX_MRU			2048
#define MAX_MRU_COUNT	29

typedef struct
{
  DWORD   cbSize;
  DWORD   nMaxItems;
  DWORD   dwFlags;
  HKEY    hKey;
  LPCTSTR  lpszSubKey;
  PROC    lpfnCompare;
} CREATEMRULIST, *LPCREATEMRULIST;

typedef HANDLE (WINAPI *PROCCREATEMRULIST)(LPCREATEMRULIST);
typedef void (WINAPI *PROCFREEMRULIST)(HANDLE);
typedef int (WINAPI *PROCADDMRUSTRING)(HANDLE, LPCTSTR);
typedef int (WINAPI *PROCADDMRUDATA)(HANDLE, LPCVOID, DWORD);
typedef BOOL (WINAPI *PROCDELMRUSTRING)(HANDLE, int);
typedef int (WINAPI *PROCENUMMRULIST)(HANDLE, int, LPVOID, DWORD);
typedef int (WINAPI *PROCFINDMRUSTRING)(HANDLE, LPCTSTR, LPINT);
typedef int (WINAPI *PROCFINDMRUDATA)(HANDLE, LPCVOID, DWORD, LPINT);

FOREXPORT HANDLE CreateMRUList(LPCREATEMRULIST lpCreateInfo);
FOREXPORT void FreeMRUList(HANDLE hList);
FOREXPORT int AddMRUString(HANDLE hList, LPCTSTR lpszString);
FOREXPORT int AddMRUData(HANDLE hList, LPCVOID lpData, DWORD cbData);
FOREXPORT BOOL DelMRUString(HANDLE hList, int nItemPos);
FOREXPORT int EnumMRUList(HANDLE hList, int nItemPos, LPVOID lpBuffer, DWORD nBufferSize);
FOREXPORT int FindMRUString(HANDLE hList, LPCTSTR lpszString, LPINT lpRegNum);
FOREXPORT int FindMRUData(HANDLE hList, LPCVOID lpData, DWORD cbData, LPINT lpRegNum);

FOREXPORT int CALLBACK MRUCompareString(LPCTSTR lpszString1, LPCTSTR lpszString2);
FOREXPORT int CALLBACK MRUCompareData(LPCVOID lpData1, LPCVOID lpData2, DWORD cbData);
FOREXPORT BOOL MRU_Initialize(void);
FOREXPORT HKEY MRU_SetRoot(HKEY hKey, LPCTSTR pszRoot);

FOREXPORT void MRU_SetComboBox(HWND hwndCbo, LPCTSTR pszSubKey, int nMaxItems);
FOREXPORT void MRU_SetComboBoxEx(HWND hwndCbo, LPCTSTR pszSubKey, int nMaxItems);
FOREXPORT void MRU_SetMenu(HMENU hMenu, LPCTSTR pszSubKey, int nMaxItems, UINT id);

FOREXPORT void MRU_Write(LPCTSTR pszSubKey, LPCTSTR pszString, int nMaxItems);
FOREXPORT HKEY MRU_GetRoot(void);

#endif /* __PCP_MRU_H */
