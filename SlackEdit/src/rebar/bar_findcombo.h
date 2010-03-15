/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : findcombo.h
 * Created    : not known (before 06/22/00)
 * Owner      : pcppopper
 * Revised on : 07/04/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __FINDCBO_H
#define __FINDCBO_H

HWND FindCombo_Create(HWND hwndParent);
HWND FindCombo_Destroy(HWND hwndCombo);

void QuickSearch_EndEdit(PNMCBEENDEDIT pnmh);

#endif /* __FINDCBO_H */
