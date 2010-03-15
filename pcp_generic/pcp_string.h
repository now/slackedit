/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_string.h
 * Created    : not known (before 07/17/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:25:33
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PP_STRING_H
#define __PP_STRING_H

#include "pcp_generic.h"

FOREXPORT void String_TrimChar(LPTSTR lpString, TCHAR chStrip);
FOREXPORT void String_Trim(LPTSTR pszStr, LPTSTR pszStrip);
FOREXPORT void String_TrimLeft(LPTSTR pszStr, TCHAR chStrip);
FOREXPORT void String_TrimRight(LPTSTR pszStr, TCHAR chStrip);
FOREXPORT BOOL String_Cut(LPTSTR pszStr, int nStart, int nLen);
FOREXPORT int String_ReplaceChar(LPTSTR pszStr, TCHAR chReplace, TCHAR chWith);
FOREXPORT BOOL String_Insert(LPTSTR *pszInto, LPCTSTR pszAdd, int iIndex);
FOREXPORT LPTSTR String_Duplicate(LPCTSTR pszSrc);
FOREXPORT LPTSTR String_Mid(LPCTSTR pszSrc, int iFirst, int iCount);
FOREXPORT void String_SetAt(LPTSTR pszSrc, int iIndex, TCHAR ch);

FOREXPORT LPTSTR String_Right(LPCTSTR pszStr, int nCount);
FOREXPORT LPTSTR String_Left(LPCTSTR pszStr, int nCount);

FOREXPORT HINSTANCE String_SetResourceHandle(HINSTANCE hResource);
FOREXPORT HINSTANCE String_GetResourceHandle(void);
FOREXPORT LPTSTR String_LoadString(UINT uID);
FOREXPORT void String_ProcessHotkey(LPCTSTR pszHotkey, LPACCEL lpAccel);

FOREXPORT int String_CountSubStrings(LPCTSTR pszSource, LPCTSTR pszSubString);
FOREXPORT int String_CountSubChars(LPCTSTR pszSource, TCHAR chSubChar);

FOREXPORT __inline BOOL String_Equal(LPCTSTR pszEins, LPCTSTR pszZwei, BOOL bCase);
FOREXPORT __inline BOOL String_NumEqual(LPCTSTR pszEins, LPCTSTR pszZwei, size_t nCount, BOOL bCase);

FOREXPORT COLORREF String_MakeRGB(LPCTSTR pszRGB);
FOREXPORT BOOL ishex(LPTSTR pszString);

FOREXPORT __inline int xisalpha(int c);
FOREXPORT __inline int xisalnum(int c);
FOREXPORT INT bracetype(TCHAR ch);
FOREXPORT __inline INT bracetypesz(LPCTSTR sz);
FOREXPORT INT bracebuddy(TCHAR ch);
FOREXPORT __inline bracebuddysz(LPCTSTR psz);

FOREXPORT BOOL String_IsSpace(LPCTSTR psz);

#endif /* __PP_STRING_H */
