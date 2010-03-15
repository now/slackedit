/******************************************************************
 *                          FILE HEADER                           *
 ******************************************************************
 * Project    : SlackEdit
 *
 * File       : se_rebar.h
 * Created    : not known (before 06/24/00)
 * Owner      : pcppopper
 * Revised on : 07/05/00
 * Comments   : 
 *              
 *              
 ******************************************************************/

#ifndef __SE_REBAR_H
#define __SE_REBAR_H

HWND Rebar_Create(HWND hwndParent);
HWND Rebar_Destroy(HWND hwnd);

BOOL Rebar_ToggleItem(UINT uItem);

void Rebar_MyInsertBand(LPCTSTR pszTitle, HWND hwndItem, WORD wID, int cx, int cy);

#endif /* __SE_REBAR_H */
