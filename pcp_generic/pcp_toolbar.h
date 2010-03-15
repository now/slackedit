/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_toolbar.h
 * Created    : 01/26/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:38:26
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_TOOLBAR_H
#define __PCP_TOOLBAR_H

#include "pcp_generic.h"
#include <commctrl.h>
	
#define Toolbar_AddBitmap(hwndCtl, nButtons, lptbab)				((INT)SNDMSG((hwndCtl), TB_ADDBITMAP, (WPARAM)(nButtons), (LPARAM)(LPTBADDBITMAP)(lptbab)))
#define Toolbar_AddButtons(hwndCtl, uNumButtons, lpButtons)			((BOOL)SNDMSG((hwndCtl), TB_ADDBUTTONS, (WPARAM)(UINT)(uNumButtons), (LPARAM)(LPTBBUTTON)(lpButtons)))
#define Toolbar_AddString(hwndCtl, hinst, idString)					((INT)SNDMSG((hwndCtl), TB_ADDSTRING, (WPARAM)(HINSTANCE)(hinst), (LPARAM)MAKELONG((idString), 0)))
#define Toolbar_AutoSize(hwndCtl)									((VOID)SNDMSG((hwndCtl), TB_AUTOSIZE, 0, 0L))
#define Toolbar_ButtonCount(hwndCtl)								((INT)SNDMSG((hwndCtl), TB_BUTTONCOUNT, 0, 0L))
#define Toolbar_ButtonStructSize(hwndCtl, cb)						((VOID)SNDMSG((hwndCtl), TB_BUTTONSTRUCTSIZE, (WPARAM)(cb), 0L))
#define Toolbar_ChangeBitmap(hwndCtl, idButton, iBitmap)			((BOOL)SNDMSG((hwndCtl), TB_CHANGEBITMAP, (WPARAM)(idButton), MAKELPARAM((iBitmap), 0)))
#define Toolbar_CheckButton(hwndCtl, idButton, fCheck)				((BOOL)SNDMSG((hwndCtl), TB_CHECKBUTTON, (WPARAM)(idButton), MAKELPARAM((fCheck), 0)))
#define Toolbar_CommandToIndex(hwndCtl, idButton)					((INT)SNDMSG((hwndCtl), TB_COMMANDTOINDEX, (WPARAM)(idButton), 0L))
#define Toolbar_Customize(hwndCtl)									((VOID)SNDMSG((hwndCtl), TB_CUSTOMIZE, 0, 0L))
#define Toolbar_DeleteButton(hwndCtl, idButton)						((BOOL)SNDMSG((hwndCtl), TB_DELETEBUTTON, (WPARAM)(idButton), 0L))
#define Toolbar_EnableButton(hwndCtl, idButton, fEnable)			((BOOL)SNDMSG((hwndCtl), TB_ENABLEBUTTON, (WPARAM)(idButton), (LPARAM)MAKELONG((fEnable), 0)))
#define Toolbar_GetAnchorHighLight(hwndCtl)							((BOOL)SNDMSG((hwndCtl), TB_GETANCHORHIGHLIGHT, 0, 0L))
#define Toolbar_GetBitmap(hwndCtl, idButton)						((INT)SNDMSG((hwndCtl), TB_GETBITMAP, (WPARAM)(idButton), 0L))
#define Toolbar_GetBitmapFlags(hwndCtl)								((DWORD)SNDMSG((hwndCtl), TB_GETBITMAPFLAGS, 0, 0L))
#define Toolbar_GetButton(hwndCtl, idButton, lpButton)				((BOOL)SNDMSG((hwndCtl), TB_GETBUTTON, (WPARAM)(idButton), (LPARAM)(LPTBBUTTON)(lpButton)))
#define Toolbar_GetButtonInfo(hwndCtl, iID, lptbbi)					((INT)SNDMSG((hwndCtl), TB_GETBUTTONINFO, (WPARAM)(INT)(iID), (LPARAM)(LPTBBUTTONINFO)(lptbbi)))
#define Toolbar_GetButtonSize(hwndCtl)								((DWORD)SNDMSG((hwndCtl), TB_GETBUTTONSIZE, 0, 0L))
#define Toolbar_GetButtonText(hwndCtl, idButton, lpszText)			((DWORD)SNDMSG((hwndCtl), TB_GETBUTTONTEXT, (WPARAM)(idButton), (LPARAM)(LPTSTR)(lpszText)))
#define Toolbar_GetColorScheme(hwndCtl, lpcs)						((BOOL)SNDMSG((hwndCtl), TB_GETCOLORSCHEME, 0, (LPARAM)(LPCOLORSCHEME)(lpcs)))
#define Toolbar_GetDisabledImageList(hwndCtl)						((HIMAGELIST)SNDMSG((hwndCtl), TB_GETDISABLEDIMAGELIST, 0, 0L))
#define Toolbar_GetExtendedStyle(hwndCtl)							((DWORD)SNDMSG((hwndCtl), TB_GETEXTENDEDSTYLE, 0, 0L))
#define Toolbar_GetHotImageList(hwndCtl)							((HIMAGELIST)SNDMSG((hwndCtl), TB_GETHOTIMAGELIST, 0, 0L))
#define Toolbar_GetHotItem(hwndCtl)									((INT)SNDMSG((hwndCtl), TB_GETHOTITEM, 0, 0L))
#define Toolbar_GetImageList(hwndCtl)								((HIMAGELIST)SNDMSG((hwndCtl), TB_GETIMAGELIST, 0, 0L))
#define Toolbar_GetInsertMark(hwndCtl, lptbim)						((BOOL)SNDMSG((hwndCtl), TB_GETINSERTMARK, 0, (LPARAM)(LPTBINSERTMARK)(lptbim)))
#define Toolbar_GetInsertMarkColor(hwndCtl)							((COLORREF)SNDMSG((hwndCtl), TB_GETINSERTMARKCOLOR, 0, 0L))
#define Toolbar_GetItemRect(hwndCtl, iButton, lprc)					((BOOL)SNDMSG((hwndCtl), TB_GETITEMRECT, (LPARAM)(iButton), (LPARAM)(LPRECT)(lprc)))
#define Toolbar_GetMaxSize(hwndCtl, lpSize)							((BOOL)SNDMSG((hwndCtl), TB_GETMAXSIZE, 0, (LPARAM)(LPSIZE)(lpSize)))
#define Toolbar_Customize(hwndCtl)									((VOID)SNDMSG((hwndCtl), TB_CUSTOMIZE, 0, 0L))
#define Toolbar_GetObject(hwndCtl, iid, ppvObject)					((HRESULT)SNDMSG((hwndCtl), TB_GETOBJECT, (WPARAM)(REFIID)(iid), (LPARAM)(LPVOID *)(ppvObject)))
#define Toolbar_GetPadding(hwndCtl)									((DWORD)SNDMSG((hwndCtl), TB_GETPADDING, 0, 0L))
#define Toolbar_GetRect(hwndCtl, iID, lprc)							((BOOL)SNDMSG((hwndCtl), TB_GETRECT, (WPARAM)(INT)(iID), (LPARAM)(LPRECT)(lprc)))
#define Toolbar_GetRows(hwndCtl)									((INT)SNDMSG((hwndCtl), TB_GETROWS, 0, 0L))
#define Toolbar_GetState(hwndCtl, idButton)							((DWORD)SNDMSG((hwndCtl), TB_GETSTATE, (WPARAM)(idButton), 0L))
#define Toolbar_GetStyle(hwndCtl)									((DWORD)SNDMSG((hwndCtl), TB_GETSTYLE, 0, 0L))
#define Toolbar_GetTextRows(hwndCtl)								((INT)SNDMSG((hwndCtl), TB_GETTEXTROWS, 0, 0L))
#define Toolbar_GetToolTips(hwndCtl)								((HWND)SNDMSG((hwndCtl), TB_GETTOOLTIPS, 0, 0L))
#define Toolbar_GetUnicodeFormat(hwndCtl)							((BOOL)SNDMSG((hwndCtl), TB_GETUNICODEFORMAT, 0, 0L))
#define Toolbar_HideButton(hwndCtl, idButton, fShow)				((BOOL)SNDMSG((hwndCtl), TB_HIDEBUTTON, (WPARAM)(idButton), (LPARAM)MAKELONG((fShow), 0)))
#define Toolbar_HitTest(hwndCtl, pptHitTest)						((INT)SNDMSG((hwndCtl), TB_HITTEST, 0, (LPARAM)(LPPOINT)(pptHitTest)))
#define Toolbar_Indeterminate(hwndCtl, idButton, fIndeterminate)	((BOOL)SNDMSG((hwndCtl), TB_INDETERMINATE, (WPARAM)(idButton), (LPARAM)MAKELONG((fIndeterminate), 0)))
#define Toolbar_InsertButton(hwndCtl, idButton, lpButton)			((BOOL)SNDMSG((hwndCtl), TB_INSERTBUTTON, (WPARAM)(idButton), (LPARAM)(LPTBBUTTON)(lpButton)))
#define Toolbar_InsertMarkHitTest(hwndCtl, lppt, lptbim)			((BOOL)SNDMSG((hwndCtl), TB_INSERTMARKHITTEST, (WPARAM)(LPPOINT)(lppt), (LPARAM)(LPTBINSERTMARK)(lptbim)))
#define Toolbar_IsButtonChecked(hwndCtl, idButton)					((BOOL)SNDMSG((hwndCtl), TB_ISBUTTONCHECKED, (WPARAM)(idButton), 0L))
#define Toolbar_IsButtonEnabled(hwndCtl, idButton)					((BOOL)SNDMSG((hwndCtl), TB_ISBUTTONENABLED, (WPARAM)(idButton), 0L))
#define Toolbar_IsButtonHidden(hwndCtl, idButton)					((BOOL)SNDMSG((hwndCtl), TB_ISBUTTONHIDDEN, (WPARAM)(idButton), 0L))
#define Toolbar_IsButtonHighlighted(hwndCtl, idButton)				((BOOL)SNDMSG((hwndCtl), TB_ISBUTTONHIGHLIGHTED, (WPARAM)(idButton), 0L))
#define Toolbar_IsButtonIndeterminate(hwndCtl, idButton)			((BOOL)SNDMSG((hwndCtl), TB_ISBUTTONINDETERMINATE, (WPARAM)(idButton), 0L))
#define Toolbar_IsButtonPressed(hwndCtl, idButton)					((BOOL)SNDMSG((hwndCtl), TB_ISBUTTONPRESSED, (WPARAM)(idButton), 0L))
#define Toolbar_LoadImages(hwndCtl, iBitmapID, hinst)				((INT)SNDMSG((hwndCtl), TB_LOADIMAGES, (WPARAM)(INT)(iBitmapID), (LPARAM)(HINSTANCE)(hinst)))
#define Toolbar_MapAccelerator(hwndCtl, chAccel, pIDBtn)			((BOOL)SNDMSG((hwndCtl), TB_MAPACCELERATOR, (WPARAM)(TCHAR)(chAccel), (LPARAM)(LPUINT)(pIDBtn)))
#define Toolbar_MarkButton(hwndCtl, idButton, fHighlight)			((BOOL)SNDMSG((hwndCtl), TB_MARKBUTTON, (WPARAM)(idButton), (LPARAM)MAKELONG((fHighlight), 0)))
#define Toolbar_MoveButton(hwndCtl, uOldPos, uNewPos)				((BOOL)SNDMSG((hwndCtl), TB_MOVEBUTTON, (WPARAM)(UINT)(uOldPos), (LPARAM)(UINT)(uNewPos)))
#define Toolbar_PressButton(hwndCtl, idButton, fPress)				((BOOL)SNDMSG((hwndCtl), TB_PRESSBUTTON, (WPARAM)(idButton), (LPARAM)MAKELONG((fPress), 0)))
#define Toolbar_ReplaceBitmap(hwndCtl, ptbrb)						((BOOL)SNDMSG((hwndCtl), TB_REPLACEBITMAP, 0, (LPARAM)(LPTBREPLACEBITMAP)(ptbrb)))
#define Toolbar_SaveRestore(hwndCtl, fSave, ptbsp)					((VOID)SNDMSG((hwndCtl), TB_SAVERESTORE, (WPARAM)(BOOL)(fSave), (LPARAM)(TBSAVEPARAMS *)(ptbsp)))
#define Toolbar_SetAnchorHighlight(hwndCtl, fAnchor)				((BOOL)SNDMSG((hwndCtl), TB_SETANCHORHIGHLIGHT, (WPARAM)(BOOL)(fAnchor), 0L))
#define Toolbar_SetBitmapSize(hwndCtl, dxBitmap, dyBitmap)			((BOOL)SNDMSG((hwndCtl), TB_SETBITMAPSIZE, 0, (LPARAM)MAKELONG(dxBitmap, dyBitmap)))
#define Toolbar_SetButtonInfo(hwndCtl, iID, lptbbi)					((BOOL)SNDMSG((hwndCtl), TB_SETBUTTONINFO, (WPARAM)(INT)(iID), (LPARAM)(LPTBBUTTONINFO)(lptbbi)))
#define Toolbar_SetButtonSize(hwndCtl, dxButton, dyButton)			((BOOL)SNDMSG((hwndCtl), TB_ISBUTTONCHECKED, 0, (LPARAM)MAKELONG(dxButton, dyButton)))
#define Toolbar_SetButtonWidth(hwndCtl, cxMin, cxMax)				((BOOL)SNDMSG((hwndCtl), TB_SETBUTTONWIDTH, 0, (LPARAM)MAKELONG(cxMin, cxMax)))
#define Toolbar_SetCmdID(hwndCtl, index, cmdId)						((BOOL)SNDMSG((hwndCtl), TB_SETCMDID, (WPARAM)(UINT)(index), (LPARAM)(UINT)(cmdId)))
#define Toolbar_SetColorScheme(hwndCtl, lpcs)						((VOID)SNDMSG((hwndCtl), TB_SETCOLORSCHEME, 0, (LPARAM)(LPCOLORSCHEME)(lpcs)))
#define Toolbar_SetDisabledImageList(hwndCtl, himlNewDisabled)		((HIMAGELIST)SNDMSG((hwndCtl), TB_SETDISABLEDIMAGELIST, 0, (LPARAM)(HIMAGELIST)(hilmNewDisabled)))
#define Toolbar_SetDrawTextFlags(hwndCtl, dwMask, dwDTFlags)		((DWORD)SNDMSG((hwndCtl), TB_SETDRAWTEXTFLAGS, (WPARAM)(DWORD)(dwMask), (LPARAM)(DWORD)(dwDTFlags)))
#define Toolbar_SetExtendedStyle(hwndCtl, dwExStyle)				((DWORD)SNDMSG((hwndCtl), TB_SETEXTENDEDSTYLE, 0, (LPARAM)(DWORD)(dwExStyle)))
#define Toolbar_SetHotImageList(hwndCtl, himlNewHot)				((HIMAGELIST)SNDMSG((hwndCtl), TB_SETHOTIMAGELIST, 0, (LPARAM)(HIMAGELIST)(himlNewHot)))
#define Toolbar_SetHotItem(hwndCtl, iHot)							((INT)SNDMSG((hwndCtl), TB_SETHOTITEM, (WPARAM)(INT)(iHot), 0L))
#define Toolbar_SetImageList(hwndCtl, himlNew)						((HIMAGELIST)SNDMSG((hwndCtl), TB_SETIMAGELIST, 0, (LPARAM)(HIMAGELIST)(himlNew)))
#define Toolbar_SetIndent(hwndCtl, iIndent)							((BOOL)SNDMSG((hwndCtl), TB_SETINDENT, (WPARAM)(INT)(iIndent), 0L))
#define Toolbar_SetInsertMark(hwndCtl, lptbim)						((VOID)SNDMSG((hwndCtl), TB_SETINSERTMARK, 0, (LPARAM)(LPTBINSERTMARK)(lptbim)))
#define Toolbar_SetInsertMarkColor(hwndCtl, clrInsertMark)			((COLORREF)SNDMSG((hwndCtl), TB_SETINSERTMARKCOLOR, 0, (LPARAM)(COLORREF)(clrInsertMark)))
#define Toolbar_SetMaxTextRows(hwndCtl, iMaxRows)					((BOOL)SNDMSG((hwndCtl), TB_SETMAXTEXTROWS, (WPARAM)(INT)(iMaxRows), 0L))
#define Toolbar_SetPadding(hwndCtl, cx, cy)							((DWORD)SNDMSG((hwndCtl), TB_SETPADDING, 0, MAKELPARAM((cx), (cy))))
#define Toolbar_SetParent(hwndCtl, hwndParent)						((HWND)SNDMSG((hwndCtl), TB_SETPARENT, (WPARAM)(HWND)(hwndParent), 0L))
#define Toolbar_SetRows(hwndCtl, cRows, fLarger, lprc)				((VOID)SNDMSG((hwndCtl), TB_SETROWS, MAKEWPARAM(cRows, fLarger), (LPARAM)(LPRECT)(lprc)))
#define Toolbar_SetState(hwndCtl, idButton, fState)					((BOOL)SNDMSG((hwndCtl), TB_SETSTATE, (WPARAM)(idButton), (LPARAM)MAKELONG((fState), 0)))
#define Toolbar_SetStyle(hwndCtl, dwStyle)							((DWORD)SNDMSG((hwndCtl), TB_SETSTYLE, 0, (LPARAM)(DWORD)(dwStyle)))
#define Toolbar_SetToolTips(hwndCtl, hwndToolTip)					((HWND)SNDMSG((hwndCtl), TB_SETTOOLTIPS, (WPARAM)(HWND)(hwndToolTips), 0L))
#define Toolbar_SetUnicodeFormat(hwndCtl, fUnicode)					((BOOL)SNDMSG((hwndCtl), TB_SETUNICODEFORMAT, (WPARAM)(BOOL)(fUnicode), 0L))

#endif /* __PCP_TOOLBAR_H */
