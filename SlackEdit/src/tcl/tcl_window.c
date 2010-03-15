/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pp_tcl_window.c
 * Created    : not known   (before 12/21/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:08:18
 * Comments   : Tcl window command implementations 
 *                               (stuff that deals with SlackEdit itself)
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* windows */
#include <tcl.h>

/* SlackEdit */
#include "../slack_main.h"
#include "../parsing/file_actions.h"

/* pcp_edit */
#include <pcp_textedit.h>

/****************************************************************
 * Function Implementation                                      *
 ****************************************************************/

int MyTcl_OpenFileCmd(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    if (objc < 2)
    {
        Tcl_WrongNumArgs(TclInterp, 1, objv, _T("<file>"));

        return (TCL_ERROR);
    }

    File_Open(Tcl_GetStringFromObj(objv[1], NULL), CRLF_STYLE_AUTOMATIC);

    return (TCL_OK);
}

int MyTcl_GetFileName(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{
    FILEINFO fi;

    Main_GetFileInfo(MDI_MyGetActive(FALSE), &fi);

    Tcl_SetResult(TclInterp, fi.szFileName, TCL_VOLATILE);

    return (TCL_OK);
}

int MyTcl_Window(ClientData clData, Tcl_Interp *TclInterp, int objc, Tcl_Obj *const objv[])
{


    return (TCL_OK);
}
