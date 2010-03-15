/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_window.h
 * Created    : not known (before 01/26/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:28:14
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_WINDOW_H
#define __PCP_WINDOW_H

#include "pcp_generic.h"

FOREXPORT void Window_CenterWindow(HWND hwnd);
FOREXPORT DWORD Window_ModifyStyle(HWND hwnd, DWORD dwRemove, DWORD dwAdd, UINT uFlags);
FOREXPORT DWORD Window_ModifyStyleEx(HWND hwnd, DWORD dwRemove, DWORD dwAdd, UINT uFlags);
FOREXPORT BOOL Window_UpdateLayout(HWND hwnd);

FOREXPORT void Window_ClientToScreenRect(HWND hwnd, LPRECT lprc);
FOREXPORT void Window_ScreenToClientRect(HWND hwnd, LPRECT lprc);

#define Window_EnableDlgItem(hwnd, nIDDlgItem, bEnable)	(EnableWindow(GetDlgItem((HWND)(hwnd), (INT)(nIDDlgItem)), (BOOL)(bEnable)))
#define Window_GetDlgItemTextLength(hwnd, nIDDlgItem)	(GetWindowTextLength(GetDlgItem((HWND)(hwnd), (INT)(nIDDlgItem))))

#define Window_SetFocusToDlgItem(hwnd, nIDDlgItem)		(SetFocus(GetDlgItem((hwnd), (nIDDlgItem))))

#define Window_Char(hwnd, chCharCode, nRepCnt, nFlags)	((DWORD)SNDMSG((hwnd), WM_CHAR, (WPARAM)(chCharCode), MAKELPARAM(nRepCnt), (nFlags))))
#define Window_Clear(hwnd)								((DWORD)SNDMSG((hwnd) WM_CLEAR, 0, 0L))
#define Window_Close(hwnd)								((DWORD)SNDMSG((hwnd), WM_CLOSE, 0, 0L))
#define Window_Command(hwnd, wID, wNotifyCode, hwndCtl)	((DWORD)SNDMSG((hwnd), WM_COMMAND, MAKEWPARAM(wID, wNotifyCode), (LPARAM)(HWND)(hwndCtl)))
#define Window_CopyData(hwnd, hwndSender, pcds)			((BOOL)SNDMSG((hwnd), WM_COPYDATA, (WPARAM)(HWND)(hwndSender), (LPARAM)(PCOPYDATASTRUCT)(pcds)))
#define Window_GetFont(hwnd)							((HFONT)SNDMSG((hwnd), WM_GETFONT, 0, 0L))
#define Window_GetIcon(hwnd, fType)						((HICON)SNDMSG((hwnd), WM_GETICON, (WPARAM)(INT)(fType), 0L))
#define Window_GetText(hwnd, cchMaxText, lpszText)		((DWORD)SNDMSG((hwnd), WM_GETTEXT, (WPARAM)(WORD)(cchMaxText), (LPARAM)(LPTSTR)(lpszText)))
#define Window_GetTextLength(hwnd)						((DWORD)SNDMSG((hwnd), WM_GETTEXTLENGTH, 0, 0L))
#define Window_KeyDown(hwnd, nVirtKey, nRepCnt, nFlags)	((DWORD)SNDMSG((hwnd), WM_KEYDOWN, (WPARAM)(INT)(nVirtKey), MAKELPARAM((nRepCnt), (nFlags))))
#define Window_KeyUp(hwnd, nVirtKey, nRepCnt, nFlags)	((DWORD)SNDMSG((hwnd), WM_KEYUP, (WPARAM)(INT)(nVirtKey), MAKELPARAM((nRepCnt), (nFlags))))
#define Window_Notify(hwnd, idCtrl, pnmh)				((DWORD)SNDMSG((hwnd), WM_NOTIFY, (WPARAM)(INT)(idCtrl), (LPARAM)(LPNMHDR)(pnmh)))
#define Window_SetFont(hwnd, hFont, fRedraw)			((DWORD)SNDMSG((hwnd), WM_SETFONT, (WPARAM)(HFONT)(hFont), MAKELPARAM(fRedraw, 0)))
#define Window_SetIcon(hwnd, fType, hIcon)				((HICON)SNDMSG((hwnd), WM_SETICON, (WPARAM)(INT)(fType), (LPARAM)(HICON)(hIcon)))
#define Window_SetBothIcons(hwnd, hIcon)				Window_SetIcon((hwnd), ICON_BIG, (hIcon)); \
														Window_SetIcon((hwnd), ICON_SMALL, (hIcon))
#define Window_SetRedraw(hwnd, fRedraw)					((DWORD)SNDMSG((hwnd), WM_SETREDRAW, (WPARAM)(INT)(fRedraw), 0L))
#define Window_SetText(hwnd, lpsz)						((DWORD)SNDMSG((hwnd), WM_SETTEXT, 0, (LPARAM)(LPCTSTR)(lpsz)))
#define Window_Size(hwnd, nWidth, nHeight)				((DWORD)SNDMSG((hwnd), WM_SIZE, SIZE_RESTORED, MAKELPARAM(nWidth, nHeight)))
#endif /* __PCP_WINDOW_H */
