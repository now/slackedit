/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_mdi.h
 * Created    : 01/30/00
 * Owner      : pcppopper
 * Revised on : 07/01/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_MDI_H
#define __PCP_MDI_H

#include "pcp_generic.h"

#define MDI_Activate(hwndCtl, hwndChildAct)				((VOID)SNDMSG((hwndCtl), WM_MDIACTIVATE, (WPARAM)(HWND)(hwndChildAct), 0L))
#define MDI_Cascade(hwndCtl, fuCascade)					((BOOL)SNDMSG((hwndCtl), WM_MDICASCADE, (WPARAM)(UINT)(fuCascade), 0L))
#define MDI_Create(hwndCtl, lpmdic)						((HWND)SNDMSG((hwndCtl), WM_MDICREATE, 0, (LPARAM)(LPMDICREATESTRUCT)(lpmdic)))
#define MDI_Destroy(hwndCtl, hwndChild)					((VOID)SNDMSG((hwndCtl), WM_MDIDESTROY, (WPARAM)(HWND)(hwndChild), 0L))
#define MDI_GetActive(hwndCtl, lpfMaximized)			((HWND)SNDMSG((hwndCtl), WM_MDIGETACTIVE, 0, (LPARAM)(LPBOOL)(lpfMaximized)))
#define MDI_IconArrange(hwndCtl)						((VOID)SNDMSG((hwndCtl), WM_MDIICONARRANGE, 0, 0L))
#define MDI_Maximize(hwndCtl, hwndMax)					((VOID)SNDMSG((hwndCtl), WM_MDIMAXIMIZE, (WPARAM)(HWND)(hwndMax), 0L))
#define MDI_Next(hwndCtl, hwndChild, fNext)				((HWND)SNDMSG((hwndCtl), WM_MDINEXT, (WPARAM)(HWND)(hwndChild), (LPARAM)(BOOL)(fNext)))
#define MDI_RefreshMenu(hwndCtl)						((HMENU)SNDMSG((hwndCtl), WM_MDIREFRESHMENU, 0, 0L))
#define MDI_Restore(hwndCtl, hwndRes)					((VOID)SNDMSG((hwndCtl), WM_MDIRESTORE, (WPARAM)(HWND)(hwndRes), 0L))
#define MDI_SetMenu(hwndCtl, hmenuFrame, hmenuWindow)	((HMENU)SNDMSG((hwndCtl, WM_MDISETMENU, (WPARAM)(HMENU)(hmenuFrame), (LPARAM)(HMENU)(hmenuWindow)))
#define MDI_Tile(hwndCtl, fuTile)						((BOOL)SNDMSG((hwndCtl), WM_MDITILE, (WPARAM)(UINT)(fuTile), 0L))

FOREXPORT int MDI_GetCount(HWND hwnd);

#endif /* __PCP_MDI_H */
