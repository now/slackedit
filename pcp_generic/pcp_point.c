/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_point.c
 * Created    : not known (before 01/24/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:30:53
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"
#include "pcp_definitions.h"

#include "pcp_point.h"

__inline BOOL Point_Equal(LPPOINT lpptDrei, LPPOINT lpptVier)
{
    return ((lpptDrei->x == lpptVier->x && lpptDrei->y == lpptVier->y));
}
