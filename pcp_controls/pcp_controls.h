/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_controls.h
 * Created    : not known (before 12/01/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:45:02
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_CONTROLS_H
#define __PCP_CONTROLS_H

#ifdef FOREXPORT
#undef FOREXPORT
#endif /* FOREXPORT */

#ifdef PCP_CONTROLS_EXPORTS
#define FOREXPORT __declspec(dllexport)
#else
#define FOREXPORT __declspec(dllimport)
#endif /* PCP_CONTROLS_EXPORTS */

FOREXPORT HINSTANCE PCPControls_GetHandle(void);

#endif /* __PCP_CONTROLS_H */
