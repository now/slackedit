/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcpcontrols.h
 * Created    : not known (before 12/01/99)
 * Owner      : pcppopper
 * Revised on : 06/30/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_DEFINITIONS_H
#define __PCP_DEFINITIONS_H

#define DIMOF(Array) (sizeof(Array) / sizeof(Array[0]))

typedef unsigned (__stdcall *PTHREAD_START) (void *);

#define BEGINTHREADEX(lpsa, cbStack, lpStartAddr, \
	lpvThreadParm, fdwCreate, lpIDThread)         \
	((HANDLE)_beginthreadex(                      \
	(void *) (lpsa),                              \
	(unsigned) (cbStack),                         \
	(PTHREAD_START) (lpStartAddr),                \
	(void *) (lpvThreadParm),                     \
	(unsigned) (fdwCreate),                       \
	(unsigned *) (lpIDThread)))

#define HANDLE_DLG_MSG(hwnd, message, fn)                           \
	case (message): return (SetDlgMsgResult(hwnd, uMsg,            \
		HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

#define MB(string) MessageBox(GetActiveWindow(), string, NULL, MB_OK | MB_ICONEXCLAMATION)

#define IsLButtonDownEx() (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
#define IsMButtonDownEx() (GetAsyncKeyState(VK_MBUTTON) & 0x8000)
#define IsRButtonDownEx() (GetAsyncKeyState(VK_RBUTTON) & 0x8000)

#define IsCtrlDown()	(GetAsyncKeyState(VK_CONTROL) & 0x8000)
#define IsAltDown()		(GetAsyncKeyState(VK_MENU) & 0x8000)
#define IsShiftDown()	(GetAsyncKeyState(VK_SHIFT) & 0x8000)

#define IsKeyDown(key)	(GetAsyncKeyState((key)) & 0x8000)

/* gotta check that these are right */
#define SetBitPos(x, b)		((x) |= (1 << (b)))
#define TestBitPos(x, b)	((x) & (1 << (b)))
#define ClearBitPos(x, b)	((x) &= ~(1 << (b)))

#define SetBitVal(x, b)		((x) |= (b))
#define TestBitVal(x, b)	((x) & (b))
#define ClearBitVal(x, b)	((x) &= ~(b))

#define INITSTRUCT(structure, fInitSize)                           \
	(ZeroMemory(&(structure), sizeof(structure)),                  \
	fInitSize ? (*(int*) &(structure) = sizeof(structure)) : 0)

/* BOOL Cls_OnCopyData(HWND hwnd, HWND hwndSender, PCOPYDATASTRUCT pcds) */
#define HANDLE_WM_COPYDATA(hwnd, wParam, lParam, fn) \
	(LRESULT)(DWORD)(UINT)(BOOL)(fn)((hwnd), (HWND)(wParam), (PCOPYDATASTRUCT)(lParam))
#define FORWARD_WM_COPYDATA(hwnd, hwndSender, pcds, fn) \
	(BOOL)(DWORD)(fn)((hwnd), WM_COPYDATA, (WPARAM)(HWND)(hwndSender), (LPARAM)(PCOPYDATASTRUCT)(pcds))

#endif /* __PCP_DEFINITIONS_H */
