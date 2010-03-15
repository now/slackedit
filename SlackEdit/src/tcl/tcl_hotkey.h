/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : tcl_hotkey.h
 * Created    : 07/16/00
 * Owner      : pcppopper
 * Revised on : 07/16/00
 * Comments   : 
 *              
 *              
 ****************************************************************/

#ifndef __TCL_HOTKEY_H
#define __TCL_HOTKEY_H

BOOL HotkeyTable_DeleteData(PVOID pKey, PVOID pData, PVOID pUserData);

INT HotkeyList_DeleteData(PVOID pData);

BOOL MyTcl_Hotkey_AddCommandValues(Tcl_Interp *TclInterp);
BOOL MyTcl_TranslateAccelerator(HWND hwnd, LPMSG lpMsg);

int MyTcl_Hotkey(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *CONST objv[]);

#endif /* __TCL_HOTKEY_H */
