/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : file_interface.h
 * Created    : 08/15/00
 * Owner      : pcppopper
 * Revised on : 08/15/00
 * Comments   : 
 *              
 *              
 ****************************************************************/

#ifndef __FILE_INTERFACE_H
#define __FILE_INTERFACE_H

/* fileinfo file types and masks */
#define FILE_TYPE_MASK_FIRST	0x0001
#define FILE_TYPE_MASK_EDIT		0x0002
#define FILE_TYPE_NOTHING		0x0004
#define FILE_TYPE_TEXT			0x0008
#define FILE_TYPE_HEX			0x0010

#define FILEISSOMETHING(lpFileView) (!((((LPFILEVIEW)(lpFileView))->dwFlags) & FILE_TYPE_MASK_EDIT) && !((((LPFILEVIEW)lpFileView)->dwFlags) & FILE_TYPE_NOTHING))

typedef struct tagFILEVIEW
{
	/* File Info */
	LPTSTR	pszFileName;
	DWORD	dwFileType;

	/* View Info */
	HWND	hwndView;
	HWND	hwndEdit;

	/* Edit Interface Pointer */
	LPEDITINTERFACE	lpEditInterface;
} FILEVIEW, *LPFILEVIEW;

typedef struct tagOPENSAVEFILEINFO
{
	DWORD	dwFileType;
	int		nLineFeedMode;
	BOOL	bBackup;
	BOOL	bUpdateFileView;
} OPENSAVEFILEINFO, *LPOPENSAVEFILEINFO;

enum
{
	FILE_FIND_BY_VIEW_HWND,
	FILE_FIND_BY_EDIT_HWND,
	FILE_FIND_BY_FILENAME,
};

BOOL FileInterface_InitializeFileHandling(void);
void FileInterface_DestroyFileHandling(void);

LPFILEINFO FileInterface_GetFileView(const LPVOID lpData, UINT uDataType);

BOOL FileInterface_ReloadFile(LPFILEINFO lpFileInfo);
BOOL FileInterface_OpenFile_WithDialog(void);
BOOL FileInterface_SaveFile_WithDialog(LPFILEINFO lpFileInfo);
BOOL FileInterface_OpenFile(LPCTSTR pszFileName, LPOPENSAVEFILEINFO lposfi);
BOOL FileInterface_SaveFile(LPFILEVIEW lpFileView, LPTSTR pszFileName, LPOPENSAVEFILEINFO lposfi);
BOOL FileInterface_AutoSaveFile(LPFILEVIEW lpFileView);
void FileInterface_CloseFile(LPFILEVIEW lpFileView);

#endif /* __FILE_INTERFACE_H */
