/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_clipboard.h
 * Created    : not known (before 01/05/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:36:49
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_CLIPBOARD_H
#define __PCP_CLIPBOARD_H

#include "pcp_generic.h"

FOREXPORT void Clipboard_AppendText(LPCTSTR pszText, BOOL bPreendInstead);
FOREXPORT void Clipboard_SetText(LPCTSTR pszText);
FOREXPORT BOOL Clipboard_GetText(LPTSTR *pszText);

#endif /* __PCP_CLIPBOARD_H */
