/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_filenotify.h
 * Created    : 01/07/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:38:13
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_FILENOTIFY_H
#define __PCP_FILENOTIFY_H

#include "pcp_generic.h"
#include "pcp_linkedlist.h"

#define FN_NONE			0x00000000
#define FN_CHANGED_TIME	0x00000001
#define FN_CHANGED_SIZE	0x00000002
#define FN_CHANGED_NAME	0x00000004
#define FN_DELETED		0x00000008
#define FN_CREATED		0x0000000F

#define FN_NOTIFY_DEFAULT (FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_FILE_NAME)

typedef struct tagFILENOTIFYDATA
{
	LPTSTR				pszFileName;
	DWORD				dwNotifyFilter;
	FILETIME			ftModified;
	DWORD				dwFileSize;
	BOOL				bExists;
} FILENOTIFYDATA, *LPFILENOTIFYDATA;

typedef void (*FILENOTIFYCALLBACK)(LPFILENOTIFYDATA lpfnd, DWORD dwChange);

typedef struct tagFILENOTIFYSTRUCT
{
	HANDLE		hThread;
	HANDLE		hEvent;
	DWORD		dwThreadID;
	BOOL		bMonitoring;
	PLINKEDLIST	pfndList;
	FILENOTIFYCALLBACK	lpfnNotifyCallback;
} FILENOTIFYSTRUCT, *LPFILENOTIFYSTRUCT;


FOREXPORT BOOL FileNotify_CreateNotifier(LPFILENOTIFYSTRUCT *lpfns, FILENOTIFYCALLBACK lpfnCallback);
FOREXPORT void FileNotify_DestroyNotifier(LPFILENOTIFYSTRUCT lpfns);
FOREXPORT void FileNotify_AddFile(LPFILENOTIFYSTRUCT lpfns, LPCTSTR pszFileName, DWORD dwNotifyFilter);
FOREXPORT void FileNotify_RemoveFile(LPFILENOTIFYSTRUCT lpfns, LPCTSTR pszFileName);
FOREXPORT BOOL FileNotify_StartMonitoring(LPFILENOTIFYSTRUCT lpfns);
FOREXPORT BOOL FileNotify_StopMonitoring(LPFILENOTIFYSTRUCT lpfns);
FOREXPORT void FileNotify_DeleteNotifier(LPFILENOTIFYSTRUCT lpfns);

#endif /* __PCP_FILENOTIFY_H */
