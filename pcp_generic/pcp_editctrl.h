/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_edit.h
 * Created    : not known (before 12/01/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:22:04
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_EDITCTRL_H
#define __PCP_EDITCTRL_H

#define Edit_ExSetSel(hwndCtl, lpchr)		((DWORD)SNDMSG((hwndCtl), EM_EXSETSEL, 0, (LPARAM)(CHARRANGE FAR *)(lpchr)))
#define Edit_ExGetSel(hwndCtl, lpchr)		((VOID)SNDMSG((hwndCtl), EM_EXGETSEL, 0, (LPARAM)(CHARRANGE FAR *)(lpchr)))
#define Edit_GetSelText(hwndCtl, lpBuf)		((DWORD)SNDMSG((hwndCtl), EM_GETSELTEXT, 0, (LPARAM)(LPTSTR)(lpBuf)))
#define Edit_GetTextRange(hwndCtl, lpRange)	((DWORD)SNDMSG((hwndCtl), EM_GETTEXTRANGE, 0, (LPARAM)(TEXTRANGE FAR *)(lpRange)))
#define Edit_ReplaceSel2(hwndCtl, fCanUndo, lpszReplace)	((VOID)SNDMSG((hwndCtl), EM_REPLACESEL, (BOOL)(fCanUndo), (LPARAM)(LPCTSTR)(lpszReplace)))
#define Edit_HideSel(hwndCtl, fHide, fChangeStyle)	((VOID)SNDMSG((hwndCtl), EM_HIDESELECTION, (BOOL)(fHide), (BOOL)(fChangeStyle)))
#define Edit_Redo(hwndCtl)					((DWORD)SNDMSG((hwndCtl), EM_REDO, 0, 0L))
#define Edit_Cut(hwndCtl)					((VOID)SNDMSG((hwndCtl), WM_CUT, 0, 0L))
#define Edit_StopGroup(hwndCtl)				((DWORD)SNDMSG((hwndCtl), EM_STOPGROUPTYPING, 0, 0L))
#define Edit_Paste(hwndCtl)					((VOID)SNDMSG((hwndCtl), WM_PASTE, 0, 0L))
#define Edit_CanRedo(hwndCtl)				((DWORD)SNDMSG((hwndCtl), EM_CANREDO, 0, 0L))
#define Edit_CanPaste(hwndCtl, uFormat)		((DWORD)SNDMSG((hwndCtl), EM_CANPASTE, (WPARAM)(UINT)(uFormat), 0L))
#define Edit_SetCharFormat(hwndCtl, uFlags, lpFmt)	((DWORD)SNDMSG((hwndCtl), EM_SETCHARFORMAT, (WPARAM)(UINT)(uFlags), (LPARAM)(CHARFORMAT FAR *)(lpFmt)))
#define Edit_DelSel(hwndCtl)				((VOID)SNDMSG((hwndCtl), WM_CLEAR, 0, 0L))
#define Edit_SetEventMask(hwndCtl, dwMask)	((DWORD)SNDMSG((hwndCtl), EM_SETEVENTMASK, 0, (LPARAM)(DWORD)(dwMask)))
#define Edit_ExLimitText(hwndCtl, cchTextMax)	((VOID)SNDMSG((hwndCtl), EM_EXLIMITTEXT, 0, (LPARAM)(DWORD)(cchTextMax)))
#define Edit_SetUndoLimit(hwndCtl, nMax)	((DWORD)SNDMSG((hwndCtl), EM_SETUNDOLIMIT, (WPARAM)(nMax), 0L))
#define Edit_AutoUrlDetect(hwndCtl, bEnable)	((DWORD)SNDMSG((hwndCtl), EM_AUTOURLDETECT, (WPARAM)(bEnable), 0L))
#define Edit_SetBkColor(hwndCtl, clr)		((DWORD)SNDMSG((hwndCtl), EM_SETBKGNDCOLOR, FALSE, (LPARAM)(COLORREF)(clr)))

#endif /* PCP_EDITCTRL_H */
