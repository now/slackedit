/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_file.h
 * Created    : not known (before 12/23/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:13:01
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __File_H
#define __File_H

BOOL File_OpenByDialog(HWND hwnd);
void File_SaveByDialog(HWND hwnd, LPTSTR pszFile);
void File_Open(LPTSTR pszFile, int nLineFeedType);
BOOL File_Save(HWND hwndChild, LPTSTR pszFile, int nLineFeedType);
BOOL File_AutoSave(HWND hwndEdit);
void File_Close(HWND hwnd);
void File_Initialize(void);
void File_Denitialize(void);


#endif /* __File_H */
