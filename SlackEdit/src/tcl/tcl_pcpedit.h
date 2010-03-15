/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pp_tcl_pcpedit.h
 * Created    : not known (before 12/21/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:09:20
 * Comments   : Tcl PCPEdit command implementations 
 *              
 *              
 *****************************************************************/

#ifndef __PP_TCL__PCPEDIT_H
#define __PP_TCL__PCPEDIT_H

int MyTcl_GetSelText(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_ReplaceSel(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_GetSelection(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_SetSelection(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_GetLineCount(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_GetLine(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_GetLineLength(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);
int MyTcl_Syntax(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[]);

#endif /* __PP_TCL__PCPEDIT_H */
