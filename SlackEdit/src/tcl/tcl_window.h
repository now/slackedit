/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pp_tcl_window.h
 * Created    : not known	(before 12/21/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:08:34
 * Comments   : Tcl window command implementations 
 *              				 (stuff that deals with SlackEdit itself)
 *              
 *****************************************************************/

#ifndef __PP_TCL__WINDOW_H
#define __PP_TCL__WINDOW_H

int MyTcl_OpenFileCmd(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_GetFileName(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);

#endif /* __PP_TCL__WINDOW_H */
