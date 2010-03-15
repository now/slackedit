/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_rebar.h
 * Created    : 01/26/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:38:39
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_REBAR_H
#define __PCP_REBAR_H

#include "pcp_generic.h"
#include <commctrl.h>

#define Rebar_BeginDrag(hwndCtl, uBand, dwPos)			((VOID)SNDMSG((hwndCtl), RB_BEGINDRAG, (WPARAM)(UINT)(uBand), (LPARAM)(DWORD)(dwPos)))
#define Rebar_DeleteBand(hwndCtl, uBand)				((BOOL)SNDMSG((hwndCtl), RB_DELETEBAND, (WPARAM)(UINT)(uBand), 0L))
#define Rebar_DragMove(hwndCtl, dwPos)					((VOID)SNDMSG((hwndCtl), RB_DRAGMOVE, 0, (LPARAM)(DWORD)(dwPos)))
#define Rebar_EndDrag(hwndCtl)							((VOID)SNDMSG((hwndCtl), RB_ENDDRAG, 0, 0L))
#define Rebar_GetBandBorders(hwndCtl, uBand, lprc)		((VOID)SNDMSG((hwndCtl), RB_GETBANDBORDERS, (WPARAM)(UINT)(uBand), (LPARAM)(LPRECT)(lprc)))
#define Rebar_GetBandCount(hwndCtl)						((UINT)SNDMSG((hwndCtl), RB_GETBANDCOUNT, 0, 0L))
#define Rebar_GetBandInfo(hwndCtl, uBand, lprbbi)		((BOOL)SNDMSG((hwndCtl), RB_GETBANDINFO, (WPARAM)(UINT)(uBand), (LPARAM)(LPREBARBANDINFO)(lprbbi)))
#define Rebar_GetBarHeight(hwndCtl)						((UINT)SNDMSG((hwndCtl), RB_GETBARHEIGHT, 0, 0L))
#define Rebar_GetBarInfo(hwndCtl, lprbi)				((BOOL)SNDMSG((hwndCtl), RB_GETBARINFO, 0, (LPARAM)(LPREBARINFO)(lprbi)))
#define Rebar_GetBkColor(hwndCtl)						((COLORREF)SNDMSG((hwndCtl), RB_GETBKCOLOR, 0, 0L))
#define Rebar_GetColorScheme(hwndCtl, lpcs)				((BOOL)SNDMSG((hwndCtl), RB_GETCOLORSCHEME, 0, (LPARAM)(LPCOLORSCHEME)(lpcs)))
#define Rebar_GetDropTarget(hwndCtl, ppDropTarget)		((VOID)SNDMSG((hwndCtl), RB_GETDROPTARGET, 0, (LPARAM)(IDropTarget **)(ppDropTarget)))
#define Rebar_GetPalette(hwndCtl)						((HPALETTE)SNDMSG((hwndCtl), RB_GETPALETTE, 0, 0L))
#define Rebar_GetRect(hwndCtl, iBand, lprc)				((BOOL)SNDMSG((hwndCtl), RB_GETRECT, (WPARAM)(INT)(iBand), (LPARAM)(LPRECT)(lprc)))
#define Rebar_BeginDraw(hwndCtl, uBand, dwPos)			((VOID)SNDMSG((hwndCtl), RB_BEGINDRAG, (WPARAM)(UINT)(uBand), (LPARAM)(DWORD)(dwPos)))
#define Rebar_GetRowHeight(hwndCtl, uRow)				((UINT)SNDMSG((hwndCtl), RB_GETROWHEIGHT, (WPARAM)(UINT)(uRow), 0L))
#define Rebar_BeginDraw(hwndCtl, uBand, dwPos)			((VOID)SNDMSG((hwndCtl), RB_BEGINDRAG, (WPARAM)(UINT)(uBand), (LPARAM)(DWORD)(dwPos)))
#define Rebar_GetTextColor(hwndCtl)						((COLORREF)SNDMSG((hwndCtl), RB_GETTEXTCOLOR, 0, 0L))
#define Rebar_GetToolTips(hwndCtl)						((HWND)SNDMSG((hwndCtl), RB_GETTOOLTIPS, 0, 0L))
#define Rebar_GetUnicodeFormat(hwndCtl)					((BOOL)SNDMSG((hwndCtl), RB_GETUNICODEFORMAT, 0, 0L))
#define Rebar_HitTest(hwndCtl, lprbht)					((INT)SNDMSG((hwndCtl), RB_HITTEST, 0, (LPARAM)(LPRBHITTESTINFO)(lprbht)))
#define Rebar_IDToIndex(hwndCtl, uBandID)				((INT)SNDMSG((hwndCtl), RB_IDTOINDEX, (WPARAM)(UINT)(uBandID), 0L))
#define Rebar_InsertBand(hwndCtl, uIndex, lprbbi)		((BOOL)SNDMSG((hwndCtl), RB_INSERTBAND, (WPARAM)(UINT)(uIndex), (LPARAM)(LPREBARBANDINFO)(lprbbi)))
#define Rebar_MaximizeBand(hwndCtl, uBand, fIdeal)		((VOID)SNDMSG((hwndCtl), RB_MAXIMIZEBAND, (WPARAM)(UINT)(uBand), (LPARAM)(BOOL)(fIdeal)))
#define Rebar_MinimizeBand(hwndCtl, uBand)				((VOID)SNDMSG((hwndCtl), RB_MINIMIZEBAND, (WPARAM)(UINT)(uBand), 0L))
#define Rebar_MoveBand(hwndCtl, iFrom, iTo)				((BOOL)SNDMSG((hwndCtl), RB_MOVEBAND, (WPARAM)(UINT)(iFrom), (LPARAM)(UINT)(iTo)))
#define Rebar_SetBandInfo(hwndCtl, uBand, lprbbi)		((BOOL)SNDMSG((hwndCtl), RB_SETBANDINFO, (WPARAM)(UINT)(uBand), (LPARAM)(LPREBARBANDINFO)(lprbbi)))
#define Rebar_SetBarInfo(hwndCtl, lprbi)				((BOOL)SNDMSG((hwndCtl), RB_SETBARINFO, 0, (LPARAM)(LPREBARINFO)(lprbi)))
#define Rebar_SetBkColor(hwndCtl, clrBk)				((COLORREF)SNDMSG((hwndCtl), RB_SETBKCOLOR, 0, (LPARAM)(COLORREF)(clrBk)))
#define Rebar_SetColorScheme(hwndCtl, lpcs)				((VOID)SNDMSG((hwndCtl), RB_SETCOLORSCHEME, 0, (LPARAM)(LPCOLORSCHEME)(lpcs)))
#define Rebar_SetPalette(hwndCtl, hPal)					((HPALETTE)SNDMSG((hwndCtl), RB_SETPALETTE, 0, (LPARAM)(HPALETTE)(hPalette)))
#define Rebar_SetParent(hwndCtl, hwndParent)			((HWND)SNDMSG((hwndCtl), RB_SETPARENT, (WPARAM)(HWND)(hwndParent), 0L))
#define Rebar_SetTextColor(hwndCtl, clrText)			((COLORREF)SNDMSG((hwndCtl), RB_SETTEXTCOLOR, 0, (LPARAM)(COLORREF)(clrText)))
#define Rebar_SetToolTips(hwndCtl, hwndToolTip)			((VOID)SNDMSG((hwndCtl), RB_SETTOOLTIPS, (WPARAM)(HWND)(hwndToolTip), 0L))
#define Rebar_SetUnicodeFormat(hwndCtl, fUnicode)		((BOOL)SNDMSG((hwndCtl), RB_SETUNICODEFORMAT, (WPARAM)(BOOL)(fUnicode), 0L))
#define Rebar_ShowBand(hwndCtl, iBand, fShow)			((BOOL)SNDMSG((hwndCtl), RB_SHOWBAND, (WPARAM)(INT)iBand, (LPARAM)(BOOL)(fShow)))
#define Rebar_SizeToRect(hwndCtl, lprc)					((BOOL)SNDMSG((hwndCtl), RB_SIZETORECT, 0, (LPARAM)(LPRECT)(lprc)))


#endif /* __PCP_REBAR_H */
