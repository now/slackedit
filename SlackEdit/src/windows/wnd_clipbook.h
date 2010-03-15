/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : clipbook.h
 * Created    : not known (before 06/26/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:18:18
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __CLIPBOOK_H
#define __CLIPBOOK_H

LRESULT CALLBACK Clipbook_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND Clipbook_Create(HWND hwndParent);
HWND Clipbook_Destroy(HWND hwndClipbook);

#endif /* __CLIPBOOK_H */
