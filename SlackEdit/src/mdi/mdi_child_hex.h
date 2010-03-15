/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_mdi_child_hex.h
 * Created    : 03/09/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:16:02
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __SE_MDI_CHILD_HEX_H
#define __SE_MDI_CHILD_HEX_H

LRESULT CALLBACK MDI_Child_Hex_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void MDI_Child_Hex_EnableToolbarButtons(HWND hwnd, BOOL bLast);

#endif /* __SE_MDI_CHILD_HEX_H */
