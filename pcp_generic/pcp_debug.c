/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_debug.h
 * Created    : not known (before 12/20/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:31:15
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include "pcp_includes.h"

/* standard */
#include <stdarg.h>

/* pcp_generic */
#include "pcp_debug.h"

VOID Debug_OutputString(LPCTSTR pszFormat, ...)
{
    TCHAR szBuf[1024];
    va_list va_params;

    va_start(va_params, pszFormat);
    _vstprintf(szBuf, pszFormat, va_params);
    OutputDebugString(szBuf);
    va_end(va_params);
}

VOID Debug_AssertFail(LPCTSTR pszFile, INT nLine, LPCTSTR pszExpression)
{
//  TCHAR szMsg[256];

//  wsprintf(szMsg, _T("File %hs, Line %d : %hs"), pszFile, nLine, pszExpression);
    
    TRACE(_T("Assertion Failed! File %hs, Line %d : %hs\n"), pszFile, nLine, pszExpression);

    DebugBreak();

//  MessageBox(GetActiveWindow(), szMsg, _T("Assertions Failed!"), MB_OK | MB_ICONERROR);
}
