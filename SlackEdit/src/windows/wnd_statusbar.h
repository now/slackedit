/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : statusbar.h
 * Created    : not known (before 01/23/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:15:07
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __STATUSBAR_H
#define __STATUSBAR_H

void MyStatusbar_Ready(HWND hwnd);
HWND MyStatusbar_Create(HWND hwndParent);
HWND MyStatusbar_Destroy(HWND hwndStatusbar);

HWND g_hwndStatusbarMain;

#endif /* __STATUSBAR_H */
