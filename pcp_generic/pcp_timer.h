/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_timer.h
 * Created    : 07/06/00
 * Owner      : pcppopper
 * Revised on : 07/06/00
 * Comments   : more or less a rip from glib
 *              
 *              
 ****************************************************************/

#ifndef __PCP_TIMER_H
#define __PCP_TIMER_H

typedef struct tagTIMER
{
	DWORD	dwStart;
	DWORD	dwEnd;
	BOOL	fActive;
} TIMER, *PTIMER;

FOREXPORT PTIMER Timer_Create(void);
FOREXPORT void Timer_Destroy(PTIMER pTimer);
FOREXPORT void Timer_Start(PTIMER pTimer);
FOREXPORT void Timer_Stop(PTIMER pTimer);
FOREXPORT __inline double Timer_StopAndCalculate(PTIMER pTimer, LPDWORD lpdwms);
FOREXPORT void Timer_Reset(PTIMER pTimer);
FOREXPORT double Timer_CalculateElapsed(PTIMER pTimer, LPDWORD lpdwms);

#endif /* __PCP_TIMER_H */
