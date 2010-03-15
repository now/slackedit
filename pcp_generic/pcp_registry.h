/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_registry.h
 * Created    : not known (before 09/30/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:24:39
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __Registry_H
#define __Registry_H

#include "pcp_generic.h"

FOREXPORT LONG Registry_SetDW(HKEY hKey, LPTSTR lpValueName, DWORD dwData);
FOREXPORT LONG Registry_SetSZ(HKEY hKey, LPTSTR lpValueName, LPTSTR lpData);
FOREXPORT DWORD Registry_GetDW(HKEY hKey, LPTSTR lpValueName, DWORD dwDefault);
FOREXPORT BOOL Registry_GetSZ(HKEY hKey, LPTSTR lpValueName, LPTSTR lpData, LPTSTR pszDefault, DWORD dwMax);

#endif /* __Registry_H */
