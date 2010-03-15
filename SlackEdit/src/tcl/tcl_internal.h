/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : tcl_int.h
 * Created    : not known (before 09/05/99)
 * Owner      : pcppopper
 * Revised on : 07/05/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __TCL_INT_H
#define __TCL_INT_H

LPTSTR Tcl_Int_DoFileName(LPTSTR pszOldFile);

#define Tcl_Int_ReportAllocFailure(TclInterp)	\
{\
	TCHAR szLine[16];\
	Tcl_AppendResult((TclInterp), _T("HeapAlloc on line "),\
				_ltot(__LINE__, szLine, 10), _T(" of file "), __FILE__, _T(" failed"), (LPTSTR)NULL); \
}

#endif /* __TCL_INT_H */
