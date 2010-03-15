/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_filenotify.c
 * Created    : 01/07/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:38:09
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"
#include "pcp_definitions.h"
#include <process.h>

#include "pcp_mem.h"
#include "pcp_string.h"
#include "pcp_path.h"

#include "pcp_filenotify.h"

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

PVOID FileNotifyList_CreateData(PVOID pData);
INT FileNotifyList_DeleteData(PVOID pData);
INT FileNotifyList_CompareData(PVOID pFromNode, PVOID pFromCall, UINT uDataType);

DWORD WINAPI FileNotify_MonitorProc(LPVOID lpParameter);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

PVOID FileNotifyList_CreateData(PVOID pData)
{
    LPFILENOTIFYDATA lpfnd, lpfndTemp = (LPFILENOTIFYDATA)pData;

    lpfnd   = (LPFILENOTIFYDATA)Mem_Alloc(sizeof(FILENOTIFYDATA));
    *lpfnd  = *lpfndTemp;
    lpfnd->pszFileName = Mem_AllocStr(_tcslen(lpfndTemp->pszFileName));
    _tcscpy(lpfnd->pszFileName, lpfndTemp->pszFileName);

    return (lpfnd);
}

INT FileNotifyList_DeleteData(PVOID pData)
{
    LPFILENOTIFYDATA lpfnd = (LPFILENOTIFYDATA)pData;

    Mem_Free(lpfnd->pszFileName);
    Mem_Free(lpfnd);

    return (TRUE);
}

INT FileNotifyList_CompareData(PVOID pFromNode, PVOID pFromCall, UINT uDataType)
{
    return (_tcsicmp(((LPFILENOTIFYDATA)pFromNode)->pszFileName, (LPTSTR)pFromCall));
}

BOOL FileNotify_CreateNotifier(LPFILENOTIFYSTRUCT *lpfns, FILENOTIFYCALLBACK lpfnCallback)
{
    *lpfns                      = (LPFILENOTIFYSTRUCT)Mem_Alloc(sizeof(FILENOTIFYSTRUCT));
    (*lpfns)->hEvent                = CreateEvent(NULL, TRUE, FALSE, NULL);
    (*lpfns)->lpfnNotifyCallback    = lpfnCallback;
    (*lpfns)->pfndList          = List_CreateList(FileNotifyList_CreateData,
                                                FileNotifyList_DeleteData,
                                                NULL,
                                                FileNotifyList_CompareData, 0);

    return (TRUE);
}

void FileNotify_DestroyNotifier(LPFILENOTIFYSTRUCT lpfns)
{
    FileNotify_StopMonitoring(lpfns);
    CloseHandle(lpfns->hEvent);
    List_DestroyList(lpfns->pfndList);
    Mem_Free(lpfns);
}

void FileNotify_AddFile(LPFILENOTIFYSTRUCT lpfns, LPCTSTR pszFileName, DWORD dwNotifyFilter)
{
    PLISTNODE pNode = List_FindNode(lpfns->pfndList, (LPTSTR)pszFileName, 0);
    FILENOTIFYDATA fnd;
    HANDLE hFile;

    if (pNode != NULL)
        return;

    fnd.pszFileName = (LPTSTR)pszFileName;
    hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        GetFileTime(hFile, NULL, NULL, &fnd.ftModified);
        fnd.dwFileSize  = GetFileSize(hFile, NULL);
        CloseHandle(hFile);
    }
    else
    {
        fnd.bExists = FALSE;
//      return;
    }

    fnd.dwNotifyFilter = (dwNotifyFilter) ? dwNotifyFilter : FN_NOTIFY_DEFAULT;

    List_AddNodeAtTail(lpfns->pfndList, &fnd);
}

void FileNotify_RemoveFile(LPFILENOTIFYSTRUCT lpfns, LPCTSTR pszFileName)
{
    PLISTNODE pNode = List_FindNode(lpfns->pfndList, (LPTSTR)pszFileName, 0);

    if (pNode == NULL)
        return;

    List_DeleteNode(lpfns->pfndList, pNode);

    if (List_GetNodeCount(lpfns->pfndList) == 0)
        FileNotify_StopMonitoring(lpfns);
}

BOOL FileNotify_StartMonitoring(LPFILENOTIFYSTRUCT lpfns)
{
    if (lpfns->bMonitoring)
    {
        if (!FileNotify_StopMonitoring(lpfns))
            return (FALSE);
    }

    if ((lpfns->hThread = BEGINTHREADEX(0, 0, FileNotify_MonitorProc, lpfns, 0, &lpfns->dwThreadID)) != NULL)
    {
        lpfns->bMonitoring = TRUE;

        return (TRUE);
    }
    else
    {
        lpfns->bMonitoring  = FALSE;

        return (FALSE);
    }
}

BOOL FileNotify_StopMonitoring(LPFILENOTIFYSTRUCT lpfns)
{
    if (lpfns->bMonitoring)
    {
        if (lpfns->hThread != NULL)
        {
            // Signal to the thread that we're not monitoring any more (checked every 50 ms)
            SetEvent(lpfns->hEvent);
            // Wait for the thread to recieve the info and to terminate
            WaitForSingleObject(lpfns->hThread, INFINITE);
            // Reset the event so that it can be used again
            ResetEvent(lpfns->hEvent);
            // Make sure to close the handle as to not leak resources
            CloseHandle(lpfns->hThread);
            lpfns->hThread  = NULL;

            lpfns->bMonitoring = FALSE;

            return (TRUE);
        }
        else
        {
            lpfns->bMonitoring = FALSE;

            return (FALSE);
        }
    }
    else
    {
        return (FALSE);
    }
}

DWORD WINAPI FileNotify_MonitorProc(LPVOID lpParameter)
{
    LPFILENOTIFYSTRUCT lpfns    = (LPFILENOTIFYSTRUCT)lpParameter;
    int nFileCount              = List_GetNodeCount(lpfns->pfndList);
    HANDLE *ahChangeHandles     = (HANDLE *)_alloca(sizeof(HANDLE) * (nFileCount + 1));
    int i;

    for ( ; ; )
    {
        PLISTNODE pNode = lpfns->pfndList->pHeadNode;
        LPFILENOTIFYDATA lpfnd;
        int j = 0;
        DWORD dwWaitObj;
        HANDLE hFile;
        TCHAR szDir[MAX_PATH];

        ahChangeHandles[0]  = lpfns->hEvent;

        for (i = 0; i < nFileCount; i++, pNode = pNode->pNextNode)
        {
            HANDLE hFileChange;

            if (pNode == NULL)
                break;

            ASSERT(pNode != NULL);

            lpfnd = (LPFILENOTIFYDATA)pNode->pData;

            _tcsncpy(szDir, lpfnd->pszFileName, MAX_PATH);
            Path_RemoveFileName(szDir);
            Path_VerifyBackslash(szDir, FALSE);
            hFileChange = FindFirstChangeNotification(szDir, FALSE, lpfnd->dwNotifyFilter);
            
            // FindFirstChangeNotification returns INVALID_HANDLE_VALUE on error
            if (hFileChange != INVALID_HANDLE_VALUE)
            {
                ahChangeHandles[++j] = hFileChange;
            }
        }

        dwWaitObj = WaitForMultipleObjects(j + 1, ahChangeHandles, FALSE, INFINITE);

        for (i = 0; i < j; i++)
            VERIFY(FindCloseChangeNotification(ahChangeHandles[i + 1]));

        if (dwWaitObj == WAIT_OBJECT_0)
            break;

        lpfnd = (LPFILENOTIFYDATA)(List_GetNode(lpfns->pfndList, (dwWaitObj - WAIT_OBJECT_0 - 1)))->pData;

        ASSERT(lpfnd != NULL);

        _tcsncpy(szDir, lpfnd->pszFileName, MAX_PATH);
        Path_RemoveFileName(szDir);

        for (pNode = lpfns->pfndList->pHeadNode ; pNode != NULL; pNode = pNode->pNextNode)
        {
            TCHAR szDirNode[MAX_PATH];

            lpfnd = (LPFILENOTIFYDATA)pNode->pData;
            _tcsncpy(szDirNode, lpfnd->pszFileName, MAX_PATH);
            Path_RemoveFileName(szDirNode);

            if (String_Equal(szDir, szDirNode, FALSE))
            {
                // To allow for directories this must be changed...later...
                hFile = CreateFile(lpfnd->pszFileName, GENERIC_READ, FILE_SHARE_READ,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                if (hFile != INVALID_HANDLE_VALUE)
                {
                    FILETIME ftModified;
                    DWORD dwFileSize;
                    DWORD dwChange = FN_NONE;

                    VERIFY(GetFileTime(hFile, NULL, NULL, &ftModified));
                    dwFileSize = GetFileSize(hFile, NULL);
                    ASSERT(dwFileSize != (DWORD)-1);

                    CloseHandle(hFile);

                    if (!lpfnd->bExists)
                    {
                        lpfnd->bExists  = TRUE;
                        dwChange |= FN_CREATED;
                    }
                    else
                    {
                        if (CompareFileTime(&lpfnd->ftModified, &ftModified) < 0)
                        {
                            lpfnd->ftModified = ftModified;
                            dwChange |= FN_CHANGED_TIME;
                        }

                        if (lpfnd->dwFileSize != dwFileSize)
                        {
                            lpfnd->dwFileSize = dwFileSize;
                            dwChange |= FN_CHANGED_SIZE;
                        }
                    }

                    if (dwChange)
                        lpfns->lpfnNotifyCallback(lpfnd, dwChange);

                    hFile = CreateFile(lpfnd->pszFileName, GENERIC_READ, FILE_SHARE_READ,
                                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                    if (hFile == INVALID_HANDLE_VALUE) // The proc removed the file
                    {
                        lpfnd->bExists  = FALSE;
                    }
                    else // Update the data after the proc!
                    {
                        VERIFY(GetFileTime(hFile, NULL, NULL, &ftModified));
                        dwFileSize = GetFileSize(hFile, NULL);
                        ASSERT(dwFileSize != (DWORD)-1);

                        CloseHandle(hFile);

                        if (dwChange & FN_CHANGED_TIME)
                            lpfnd->ftModified   = ftModified;

                        if (dwChange & FN_CHANGED_SIZE)
                            lpfnd->dwFileSize   = dwFileSize;

                        if (dwChange & FN_CREATED)
                        {
                            lpfnd->ftModified = ftModified;
                            lpfnd->dwFileSize = dwFileSize;
                        }
                    }
                }
                else
                {
                    lpfns->lpfnNotifyCallback(lpfnd, FN_DELETED);
                    lpfnd->bExists  = FALSE;
                }
            }
        }
    }

    return (0);
}
