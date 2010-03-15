/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_browse.h
 * Created    : not known (before 12/22/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:32:06
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_BROWSE_H
#define __PCP_BROWSE_H

#include "pcp_generic.h"

typedef BOOL (*BROWSEMULTIPLEFILECALLBACK)(LPCTSTR pszFileName, LPARAM lParam);

typedef struct tagBROWSEINFOEX
{
	DWORD	dwFlags;
	HWND	hwndOwner;
	LPTSTR	pszStartPath;
	LPTSTR	pszPath;
	LPTSTR	pszFilter;
	LPTSTR	pszTitle;
	LPTSTR	pszFile;
	BROWSEMULTIPLEFILECALLBACK	MultipleFileCallback;
} BROWSEINFOEX, *LPBROWSEINFOEX;

FOREXPORT BOOL Browse_ForPath(LPBROWSEINFOEX lpbix);
FOREXPORT BOOL Browse_ForFile(LPBROWSEINFOEX lpbix);
FOREXPORT void Browse_HandleMultipleFileNames(LPCTSTR pszFileNames, BROWSEMULTIPLEFILECALLBACK MultipleFileCallback, LPARAM lParam);

#endif /* __PCP_BROWSE_H */
