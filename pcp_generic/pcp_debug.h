/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_debug.h
 * Created    : not known (before 12/20/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:31:24
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_DEBUG_H
#define __PCP_DEBUG_H

#ifdef _DEBUG

#ifdef PCP_GENERIC_EXPORTS
__declspec(dllexport) VOID Debug_OutputString(LPCTSTR pszFormat, ...);
__declspec(dllexport) VOID Debug_AssertFail(LPCTSTR pszFile, INT nLine, LPCTSTR pszExpression);
#else
__declspec(dllimport) VOID Debug_OutputString(LPCTSTR pszFormat, ...);
__declspec(dllimport) VOID Debug_AssertFail(LPCTSTR pszFile, INT nLine, LPCTSTR pszExpression);
#endif /* PCPGENERIC_EXPORTS */

#define TRACE		Debug_OutputString
#define ASSERT(x)	if (!(x)) Debug_AssertFail(__FILE__, __LINE__, #x)
#define VERIFY(x)	ASSERT(x)

#else /* DEBUG */

__inline void Trace(LPCTSTR pszFormat, ...) { }
#define TRACE		1 ? (void)0 : Trace
#define ASSERT(x)	(void)0
#define VERIFY(x)	(x)

#endif /* _DEBUG */

#endif /* __PCP_DEBUG_H */
