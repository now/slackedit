/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pcp_hexedit.h
 * Created    : 01/31/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:13:44
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_HEXEDIT_H
#define __PCP_HEXEDIT_H

#define WC_HEXEDIT	_T("HexEditControl")

BOOL HexEdit_RegisterControl(HMODULE hModule);
void HexEdit_UnregisterControl(HMODULE hModule);

typedef enum tagEDITMODE
{  
	EDIT_NONE,
	EDIT_ASCII,
	EDIT_HIGH,
	EDIT_LOW
} EDITMODE;

typedef struct tagHEXEDIT
{
	HWND		hwnd;
	LPBYTE		pData;
	int			nLength;
	DWORD		dwFlags;
	BOOL		bShowAddress;
	BOOL		bShowHex;
	BOOL		bShowAscii;
	BOOL		bWideAddress;
	BOOL		bReadOnly;
	BOOL		bModified;
	int			nBytesPerLine;
	int			nTopLine;
	int			nOffsetChar;
	int			nScreenChars;
	int			nCurrentAddress;
	int			nScreenLines;
	EDITMODE	emCurrentMode;
	int			nSelStart;
	int			nSelEnd;
	HFONT		hFont;
	int			nLineHeight;
	int			nCharWidth;
	int			nOffsetHex;
	int			nOffsetAscii;
	int			nOffsetAddress;
	POINT		ptEditPos;
} HEXEDIT, *LPHEXEDIT;

BOOL HexEdit_SetModify(LPHEXEDIT lphe, BOOL bModified);
BOOL HexEdit_GetModify(LPHEXEDIT lphe);

#endif /* __PCP_HEXEDIT_H */
