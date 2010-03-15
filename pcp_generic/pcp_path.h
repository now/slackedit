/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_path.h
 * Created    : 01/22/99
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:38:00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_PATH_H
#define __PCP_PATH_H

#include "pcp_generic.h"

FOREXPORT LPTSTR Path_VerifyBackslash(LPTSTR pszFileName, BOOL bWantBackslash);
FOREXPORT void Path_CompactPath(LPTSTR pszOut, LPCTSTR pszIn, int cchMax);
FOREXPORT LPTSTR Path_RemoveFileName(LPTSTR pszFileName);
FOREXPORT void Path_RemoveExt(LPTSTR pszFileName);
FOREXPORT LPTSTR Path_SwapExt(LPTSTR pszFileName, LPCTSTR pszNewExt);
FOREXPORT LPTSTR Path_GetExt(LPCTSTR pszFileName);
FOREXPORT LPTSTR Path_GetFileName(LPCTSTR pszFilePath);

FOREXPORT BOOL Path_HasRoot(LPCTSTR pszPath);
FOREXPORT BOOL Path_FileExists(LPCTSTR pszFileName);
FOREXPORT BOOL Path_DirExists(LPCTSTR pszDirName);

FOREXPORT __inline DWORD Path_GetModuleFileName(HMODULE hModule, LPTSTR pszFileName, DWORD nSize);

#endif /* __PCP_PATH_H */
