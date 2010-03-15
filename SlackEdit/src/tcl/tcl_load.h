/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : tcl_load.h
 * Created    : 07/19/00
 * Owner      : pcppopper
 * Revised on : 07/19/00
 * Comments   : 
 *              
 *              
 ****************************************************************/

#ifndef __TCL_LOAD_H
#define __TCL_LOAD_H

BOOL MyTcl_LoadLibrary(LPCTSTR pszBigVersion, LPCTSTR pszSmallVersion, BOOL bExact);
BOOL MyTcl_FreeLibrary(void);

#endif /* __TCL_LOAD_H */
