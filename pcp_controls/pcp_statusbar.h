/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_statusbar.h
 * Created    : 01/23/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:45:43
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_STATUSBAR_H
#define __PCP_STATUSBAR_H

#include "pcp_controls.h"

#define SB_MAX_TEXT		512

#define SF_TEXT		0x00000001
#define SF_CRBG		0x00000002
#define SF_CRFG		0x00000004
#define SF_OWNSTYLE	0x00000008
#define SF_STYLE	0x00000010
#define SF_DSTYLE	0x00000020
#define SF_TOOLTIP	0x00000040
#define SF_WIDTH	0x00000080
#define SF_ALL		0x000000FF

#define SFS_BORDER		0x0001
#define SFS_AUTOSIZE	0x0002

#define SB_DEFAULT_CRBG		((COLORREF)-1)
#define SB_DEFAULT_CRFG		((COLORREF)-2)

typedef struct tagSTATUSBARPANE
{
	UINT		fMask;
	LPTSTR		pszText;
	LPTSTR		pszTooltip;
	COLORREF	crBg;
	COLORREF	crFg;
	BOOL		fBorder;
	UINT		uOwnStyle;
	UINT		uStyle;
	UINT		uDrawStyle;
	INT			nPane;
	INT			nWidth;
} STATUSBARPANE, FAR *LPSTATUSBARPANE;

typedef struct tagSTATUSBARCREATEDATA
{
	HINSTANCE		hInstance;
	HWND			hwndParent;
	UINT			uID;
	BOOL			bUseMenuFont;
	int				nPanes;
	LPSTATUSBARPANE *alpsp;
} STATUSBARCREATEDATA, *LPSTATUSBARCREATEDATA;

FOREXPORT HWND Statusbar_Create(LPSTATUSBARCREATEDATA lpsbcd);
FOREXPORT void Statusbar_AddPane(HWND hwnd, LPSTATUSBARPANE lpsp);
FOREXPORT void Statusbar_SetPane(HWND hwnd, LPSTATUSBARPANE lpsp);
FOREXPORT void Statusbar_Draw(const DRAWITEMSTRUCT *lpDrawItem);
FOREXPORT void Statusbar_SetPaneText(HWND hwnd, int nPane, LPTSTR pszText);
FOREXPORT void Statusbar_Destroy(HWND hwnd);
FOREXPORT BOOL Statusbar_GetPane(HWND hwnd, LPSTATUSBARPANE lpsp);
FOREXPORT void Statusbar_OnSize(HWND hwnd, int nSizeType);

#define Statusbar_GetBorders(hwndCtl, aBorders)						((BOOL)SNDMSG((hwndCtl), SB_GETBORDERS, 0, (LPARAM)(LPINT)(aBorders)))
#define Statusbar_GetIcon(hwndCtl, iPart)							((HICON)SNDMSG((hwndCtl), SB_GETICON, 0, (LPARAM)(INT)(iPart)))
#define Statusbar_GetParts(hwndCtl, nParts, aRightCoord)			((DWORD)SNDMSG((hwndCtl), SB_GETPARTS, (WPARAM)(nParts), (LPARAM)(LPINT)(aRightCoord)))
#define Statusbar_GetRect(hwndCtl, iPart, lprc)						((BOOL)SNDMSG((hwndCtl), SB_GETRECT, (WPARAM)(iPart), (LPARAM)(LPRECT)(lprc)))
#define Statusbar_GetText(hwndCtl, iPart, szText)					((DWORD)SNDMSG((hwndCtl), SB_GETTEXT, (WPARAM)(iPart), (LPARAM)(LPTSTR)(szText)))
#define Statusbar_GetTextLength(hwndCtl, iPart)						((DWORD)SNDMSG((hwndCtl), SB_GETTEXTLENGTH, (WPARAM)(iPart), 0L);
#define Statusbar_GetTipText(hwndCtl, iPart, nSize, lpszTooltip)	((VOID)SNDMSG((hwndCtl), SB_GETTIPTEXT, MAKEWPARAM((iPart), (nSize)), (LPARAM)(LPCTSTR)(lpszTooltip)))
#define Statusbar_GetUnicodeFormat(hwndCtl)							((BOOL)SNDMSG((hwndCtl), SB_GETUNICODEFORMAT, 0, 0L))
#define Statusbar_IsSimple(hwndCtl)									((BOOL)SNDMSG((hwndCtl, SB_ISSIMPLE, 0, 0L))
#define Statusbar_SetBkColor(hwndCtl, crlBk)						((COLORREF)SNDMSG((hwndCtl), SB_SETBKCOLOR, 0, (LPARAM)(COLORREF)(clrBk)))
#define Statusbar_SetIcon(hwndCtl, iPart, hIcon)					((BOOL)SNDMSG((hwndCtl), SB_SETICON, (WPARAM)(INT)(iPart), (LPARAM)(HICON)(hIcon)))
#define Statusbar_SetMinHeight(hwndCtl, minHeight)					((VOID)SNDMSG((hwndCtl), SB_SETMINHEIGHT, (WPARAM)(minHeight), 0L))
#define Statusbar_SetParts(hwndCtl, nParts, aWidths)				((BOOL)SNDMSG((hwndCtl), SB_SETPARTS, (WPARAM)(INT)(nParts), (LPARAM)(LPINT)(aWidths)))
#define Statusbar_SetText(hwndCtl, iPart, uType, szText)			((BOOL)SNDMSG((hwndCtl), SB_SETTEXT, (WPARAM)((iPart) | (uType)), (LPARAM)(LPTSTR)(szText)))
#define Statusbar_SetTipText(hwndCtl, iPart, lpszTooltip)			((VOID)SNDMSG((hwndCtl), SB_SETTIPTEXT, (WPARAM)(INT)(iPart), (LPARAM)(LPCTSTR)(lpszTooltip)))
#define Statusbar_SetUnicodeFormat(hwndCtl, fUnicode)				((BOOL)SNDMSG((hwndCtl), SB_SETUNICODEFORMAT, (WPARAM)(BOOL)(fUnicode), 0L))
#define Statusbar_Simple(hwndCtl, fSimple)							((BOOL)SNDMSG((hwndCtl), SB_SIMPLE, (WPARAM)(BOOL)(fSimple), 0L))

#endif /* __PCP_STATUSBAR_H */
