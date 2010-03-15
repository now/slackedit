/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_tip_of_da_day.h
 * Created    : 01/04/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:44:43
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_TIP_OF_DA_DAY_H
#define __PCP_TIP_OF_DA_DAY_H

#include "pcp_controls.h"

FOREXPORT void TipOfDay_SetTipFileName(LPCTSTR pszFileName);
FOREXPORT void TipOfDay_SetBitmap(HBITMAP hBitmap);
FOREXPORT BOOL TipOfDay_CreateDialogBox(HWND hwndParent, BOOL bAtStartup);

#endif /* __PCP_TIP_OF_DA_DAY_H */
