/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_timer.c
 * Created    : 07/06/00
 * Owner      : pcppopper
 * Revised on : 07/06/00
 * Comments   : more or less a rip from glib
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"

#include "pcp_timer.h"
#include "pcp_mem.h"
#include "limits.h"

/****************************************************************
 * Type Definitions
 ****************************************************************/

/****************************************************************
 * Function Definitions
 ****************************************************************/

/****************************************************************
 * Global Variables
 ****************************************************************/

/****************************************************************
 * Function Implementations
 ****************************************************************/

PTIMER Timer_Create(void)
{
    PTIMER pTimer;

    pTimer = (PTIMER)Mem_Alloc(sizeof(TIMER));
    pTimer->fActive = TRUE;
    pTimer->dwStart = GetTickCount();

    return (pTimer);
}

void Timer_Destroy(PTIMER pTimer)
{
    ASSERT(pTimer != NULL);

    Mem_Free(pTimer);
}

void Timer_Start(PTIMER pTimer)
{
    ASSERT(pTimer != NULL);

    pTimer->fActive = TRUE;
    pTimer->dwStart = GetTickCount();
}

void Timer_Stop(PTIMER pTimer)
{
    ASSERT(pTimer != NULL);

    pTimer->dwEnd   = GetTickCount();
    pTimer->fActive = FALSE;
}

__inline double Timer_StopAndCalculate(PTIMER pTimer, LPDWORD lpdwms)
{
    Timer_Stop(pTimer);
    return (Timer_CalculateElapsed(pTimer, lpdwms));
}

void Timer_Reset(PTIMER pTimer)
{
    ASSERT(pTimer != NULL);

    pTimer->dwStart = GetTickCount();
}

double Timer_CalculateElapsed(PTIMER pTimer, LPDWORD lpdwms)
{
    double dTotal;

    ASSERT(pTimer != NULL);

    if (pTimer->fActive)
        pTimer->dwEnd   = GetTickCount();

    // Have we wrapped? (every 49.7 days according to docs
    if (pTimer->dwEnd < pTimer->dwStart)
        dTotal = ((ULONG_MAX - (pTimer->dwStart - pTimer->dwEnd)) / 1000.0);
    else
        dTotal = ((pTimer->dwEnd - pTimer->dwStart) / 1000.0);

    if (lpdwms != NULL)
    {
/*      if (pTimer->dwEnd < pTimer->dwStart)
            *lpdwms = ((ULONG_MAX - (pTimer->dwStart - pTimer->dwEnd) % 1000) * 1000);
        else
            *lpdwms = (((pTimer->dwEnd - pTimer->dwStart) % 1000) * 1000);
*/
        if (pTimer->dwEnd < pTimer->dwStart)
            *lpdwms = (ULONG_MAX - (pTimer->dwStart - pTimer->dwEnd));
        else
            *lpdwms = (pTimer->dwEnd - pTimer->dwStart);

    }

    return (dTotal);
}
