/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pp_tcl.h
 * Created    : not known	(before 07/24/00)
 * Owner      : pcppopper
 * Revised on : 07/06/00
 * Comments   :  
 *              
 *              
 *****************************************************************/

#ifndef __PP_TCL_H
#define __PP_TCL_H

/* pcp_generic */
#include <pcp_linkedlist.h>
#include <pcp_hashtable.h>

/* tcl */
#include <tcl.h>

enum
{
	SLC_TCLINTERP,
	SLC_FILENAME,
};

typedef struct tagTCLHOTKEY
{
	BYTE	fVirt;
	WORD	wKey;
	BOOL	bCommand;
	LPTSTR	pszScript;
	UINT	uCmd;
} TCLHOTKEY, *PTCLHOTKEY;

typedef struct tagTCLCLIPBOARDMONITORDATA
{
	LPTSTR	pszScript;
	UINT	uID;
} TCLCLIPBOARDMONITORDATA, *PTCLCLIPBOARDMONITORDATA;

typedef struct tagTCLSCRIPT
{
	Tcl_Interp	*TclInterp;
	TCHAR		szFileName[MAX_PATH];
	PHASHTABLE	pHotkeyTable;
	PLINKEDLIST	pClipboardMonitorDataList;
} TCLSCRIPT, *PTCLSCRIPT;

PLINKEDLIST g_TclScriptsList;

BOOL MyTcl_Init(void);
void MyTcl_OpenFile(void);

int MyTcl_LoadFile(LPTSTR pszFile, BOOL bComplain);
int MyTcl_Eval(Tcl_Interp *TclInterp, LPTSTR pszScript, BOOL bComplain);

#endif /* __PP_TCL_H */
