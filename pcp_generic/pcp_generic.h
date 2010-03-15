/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_generic.h
 * Created    : not known (before 12/01/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:27:04
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_GENERIC_H
#define __PCP_GENERIC_H

#ifdef FOREXPORT
#undef FOREXPORT
#endif /* FOREXPORT */

#ifdef PCP_GENERIC_EXPORTS
#define FOREXPORT __declspec(dllexport)
#else
#define FOREXPORT __declspec(dllimport)
#endif /* PCP_GENERIC_EXPORTS */

FOREXPORT HINSTANCE PCPGeneric_GetHandle(void);

#endif /* __PCP_GENERIC_H */
