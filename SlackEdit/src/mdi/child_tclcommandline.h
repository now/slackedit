/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : tclcmd.h
 * Created    : not known (before 12/22/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:18:40
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __TCLCMD_H
#define __TCLCMD_H

#define IDC_CBO_TCL	2008

BOOL TclCmd_Create(HWND hwndParent);
BOOL TclCmd_EndEdit(PNMCBEENDEDIT pnmcbee);

#endif /* __TCLCMD_H */
