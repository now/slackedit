/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : clipboard_monitor.h
 * Created    : 01/05/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:13:28
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __CLIPBOARD_MONITOR_H
#define __CLIPBOARD_MONITOR_H

// IDM_FIRST -1000 basically
#define IDM_EDIT_FIRSTCLIP	39000
#define MAX_CLIPS			10

void Clip_Read(HMENU *hMenu);
void Clip_Add(HWND hwnd, HMENU *hMenu);
void Clip_Save();
void Clip_RecreateMenu(HMENU *fMenu);
LPCTSTR ClipboardMonitor_GetClip(int nNode);

#endif /* __CLIPBOARD_MONITOR_H */
