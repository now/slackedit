/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pp_tcl_shell.c
 * Created    : not known (before 12/21/99)
 * Owner      : pcppopper
 * Revised on : 07/06/00
 * Comments   : Tcl Shell command implementations 
 *              
 *              
 *****************************************************************/

#ifndef __PP_TCL_SHELL_H
#define __PP_TCL__SHELL_H

#include "../pcp_generic/pcp_linkedlist.h"

#define REG_COOKIES _T("TCL Script Cookies")

int MyTcl_MsgBox(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_ShellExecute(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_Browse(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_SetCookie(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_GetCookie(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_Clipboard(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
void MyTcl_ClipboardMonitorNotify(void);

PLINKEDLIST MyTcl_CreateClipboardMonitorList(void);

#endif /* __PP_TCL__SHELL_H */
