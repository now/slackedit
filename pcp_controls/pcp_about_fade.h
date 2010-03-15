/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_about_fade.h
 * Created    : 01/04/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:45:30
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_ABOUT_FADE_H
#define __PCP_ABOUT_FADE_H

#include "pcp_controls.h"

typedef struct tagFADECTRLPAGE
{
	int			nFadePercent;
	int			nCurrentPercentage;
	int			nFadeOutWaitTime;
	int			nFadeInWaitTime;
	int			nCurrentWaitTime;
	int			nPercentageDelta;
	LPTSTR		pszPageText;
	RECT		rcText;
	COLORREF	crBackground;
	COLORREF	crFadeTo;
	COLORREF	crCurrent;
} FADECTRLPAGE, *PFADECTRLPAGE;

#define ABOUT_FADE_CLASS				_T("PCP Fader Control")

#define FC_DEFAULT_PERCENTAGE			100
#define FC_DEFAULT_FADEIN_WAITTIME		750
#define FC_DEFAULT_FADEOUT_WAITTIME		3500
#define FC_DEFAULT_PERCENTAGE_INCREMENT	5

#define FC_FIRST				(WM_USER + 150)
#define FC_ADDPAGE				(FC_FIRST)
#define FC_REMOVEPAGE			(FC_FIRST + 1)
#define FC_GETCURRENTPAGE		(FC_FIRST + 2)	
#define FC_ACTIVATE				(FC_FIRST + 3)
#define FC_DEACTIVATE			(FC_FIRST + 4)
#define FC_TOGGLEACTIVE			(FC_FIRST + 5)

#define FadeCtrl_AddPage(hwndCtl, pfcp)			((BOOL)SNDMSG((hwndCtl), FC_ADDPAGE, 0, (LPARAM)(PFADECTRLPAGE)(pfcp)))
#define FadeCtrl_RemovePage(hwndCtl, iIndex)	((BOOL)SNDMSG((hwndCtl), FC_REMOVEPAGE, (WPARAM)(INT)(iIndex), 0L))
#define FadeCtrl_GetCurrentPage(hwndCtl)		((PFADECTRLPAGE)SNDMSG((hwndCtl), FC_GETCURRENTPAGE, 0, 0L))
#define FadeCtrl_Activate(hwndCtl)				((BOOL)SNDMSG((hwndCtl), FC_ACTIVATE, 0, 0L))
#define FadeCtrl_Deactivate(hwndCtl)			((BOOL)SNDMSG((hwndCtl), FC_DEACTIVATE, 0, 0L))
#define FadeCtrl_ToggleActive(hwndCtl)			((BOOL)SNDMSG((hwndCtl), FC_TOGGLEACTIVE, 0, 0L))

FOREXPORT BOOL FadeCtrl_RegisterControl(HMODULE hModule);
FOREXPORT void FadeCtrl_UnregisterControl(HMODULE hModule);



#endif /* __PCP_ABOUT_FADE_H */
