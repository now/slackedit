/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_syntax.c
 * Created    : not known (before 12/28/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:39:20
 * Comments   : Generic syntax hiliting for pcp_edit 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* pcp_edit */
#include "pcp_edit_view.h"

/* pcp_generic */
#include <pcp_string.h>

static BOOL Edit_Syntax_IsNumber(LPCTSTR pszChars, int nLength)
{
    int i;

    if (nLength > 2 && pszChars[0] == _T('0') && pszChars[1] == _T('x'))
    {
        int i;

        for (i = 2; i < nLength; i++)
        {
            if (isdigit(pszChars[i]) || (pszChars[i] >= _T('A') && pszChars[i] <= _T('F')) ||
                (pszChars[i] >= _T('a') && pszChars[i] <= _T('f')))
                continue;

            return (FALSE);
        }
      
        return (TRUE);
    }

    if (!isdigit(pszChars[0]))
        return (FALSE);

    for (i = 1; i < nLength; i++)
    {
        if (!isdigit(pszChars[i]) && pszChars[i] != _T('+') &&
                pszChars[i] != _T('-') && pszChars[i] != _T('.') && pszChars[i] != _T('e') &&
                pszChars[i] != _T('E'))
            return (FALSE);
    }

    return (TRUE);
}

BOOL Edit_Syntax_IsKeywordGeneric(register LPTSTR apszKeywords[], LPCTSTR pszChars, int nLength, BOOL bCase)
{
    int i;

    if (apszKeywords == NULL)
        return (FALSE);

    for (i = 0; apszKeywords[i] != NULL; i++)
    {
        if (String_NumEqual(apszKeywords[i], pszChars, nLength, bCase) && apszKeywords[i][nLength] == _T('\0'))
            return (TRUE);
    }

    return (FALSE);
}

static BOOL Edit_Syntax_IsPrefix(LPTEXTDEF lptd, LPCTSTR pszChars)
{
    int i;

    if (lptd->apszPrefixes == NULL)
        return (FALSE);

    for (i = 0; lptd->apszPrefixes[i] != NULL; i++)
    {
        if (String_NumEqual(lptd->apszPrefixes[i], pszChars, _tcslen(lptd->apszPrefixes[i]), lptd->bCase))
            return (TRUE);
    }

    return (FALSE);
}

static __inline BOOL Edit_Syntax_IsKeyword(LPTEXTDEF lptd, LPCTSTR pszChars, int nLength)
{
    return (Edit_Syntax_IsKeywordGeneric(lptd->apszKeywords, pszChars, nLength, lptd->bCase));
}

static __inline BOOL Edit_Syntax_IsUser1Keyword(LPTEXTDEF lptd, LPCTSTR pszChars, int nLength)
{
    return (Edit_Syntax_IsKeywordGeneric(lptd->apszUser1Keywords, pszChars, nLength, lptd->bCase));
}

static __inline BOOL Edit_Syntax_IsUser2Keyword(LPTEXTDEF lptd, LPCTSTR pszChars, int nLength)
{
    return (Edit_Syntax_IsKeywordGeneric(lptd->apszUser2Keywords, pszChars, nLength, lptd->bCase));
}

static __inline BOOL Edit_Syntax_IsOperator(LPTEXTDEF lptd, const TCHAR cChar)
{
    if (lptd->pszOperators == NULL)
        return (FALSE);

    return (_tcschr(lptd->pszOperators, cChar) != NULL);
}

#define DEFINE_BLOCK(pos, colorindex)   \
ASSERT((pos) >= 0 && (pos) <= nLength);\
if (pBuf != NULL)\
{\
    if (*piActualItems == 0 || pBuf[*piActualItems - 1].iCharPos <= (pos))\
    {\
        pBuf[*piActualItems].iCharPos = (pos);\
        pBuf[*piActualItems].iColorIndex = (colorindex);\
        *piActualItems = *piActualItems + 1;\
    }\
}

#define COOKIE_COMMENT          0x0001
#define COOKIE_PREPROCESSOR     0x0002
#define COOKIE_EXT_COMMENT      0x0004
#define COOKIE_STRING           0x0008
#define COOKIE_CHAR             0x0010
#define COOKIE_USER1            0x0020
#define COOKIE_USER2            0x0040
#define COOKIE_PREFIXED         0x0080
#define COOKIE_DISABLED         0x0100

DWORD Edit_Syntax_ParseLineGeneric(LPEDITVIEW lpew, DWORD dwCookie, int nLineIndex, LPTEXTBLOCK pBuf, int *piActualItems)
{
    int nLength             = Edit_View_GetLineLength(lpew, nLineIndex);
    LPCTSTR pszChars        = Edit_View_GetLineChars(lpew, nLineIndex);
    LPTEXTDEF lptd          = lpew->lptdCurSourceDef;
    BOOL bFirstChar         = (dwCookie & ~(COOKIE_EXT_COMMENT)) == 0;
    BOOL bRedefineBlock     = TRUE;
    BOOL bWasCommentStart   = FALSE;
    BOOL bDecIndex          = FALSE;
    int nIdentBegin         = -1;
    int i;
    int nCommentLen;

    if (nLength <= 0)
        return (dwCookie & COOKIE_EXT_COMMENT);

    for (i = 0; ; i++)
    {
        if (bRedefineBlock)
        {
            int nPos = i;

            if (bDecIndex)
                nPos--;
            
            if (dwCookie & (COOKIE_COMMENT | COOKIE_EXT_COMMENT))
            {
                DEFINE_BLOCK(nPos, COLORINDEX_COMMENT);
            }
            else if (dwCookie & (COOKIE_CHAR | COOKIE_STRING))
            {
                DEFINE_BLOCK(nPos, COLORINDEX_STRING);
            }
            else if (dwCookie & COOKIE_PREPROCESSOR)
            {
                DEFINE_BLOCK(nPos, COLORINDEX_PREPROCESSOR);
            }
            else if (dwCookie & COOKIE_PREFIXED)
            {
                DEFINE_BLOCK(nPos, COLORINDEX_PREFIXED);
            }
            else
            {
                if (Edit_Syntax_IsOperator(lptd, pszChars[nPos]))
                {
                    DEFINE_BLOCK(nPos, COLORINDEX_OPERATOR);

                    bRedefineBlock = TRUE;
                    bDecIndex = TRUE;

                    goto out;
                }
                else
/*              if ((xisalnum(pszChars[nPos]) ||
                    pszChars[nPos] == _T('.') &&
                    nPos > 0 && (!xisalfa(pszChars[nPos - 1]) &&
                    !xisalfa(pszChars[nPos + 1]))))*/
                {
                    DEFINE_BLOCK(nPos, COLORINDEX_NORMALTEXT);
                }
/*              else
                {
                    DEFINE_BLOCK(nPos, COLORINDEX_OPERATOR);

                    bRedefineBlock = TRUE;
                    bDecIndex = TRUE;

                    goto out;
                }*/
            }
            
            bRedefineBlock = FALSE;
            bDecIndex = FALSE;
        }
out:
        
        if (i >= nLength)
            break;

        // Line Comment ? [//]
        if (dwCookie & COOKIE_COMMENT)
        {
            if ((!lptd->bCommentFirstRealChar) ||
                (lptd->bCommentFirstRealChar && bFirstChar))
            {
                DEFINE_BLOCK(i, COLORINDEX_COMMENT);
                dwCookie |= COOKIE_COMMENT;

                break;
            }
        }
        
        //  String constant ? ["..."]
        if (dwCookie & COOKIE_STRING)
        {
            if (pszChars[i] == lptd->cString && (i == 0 || i == 1 && pszChars[i - 1] != lptd->cEscape || i >= 2 && (pszChars[i - 1] != lptd->cEscape || pszChars[i - 1] == lptd->cEscape && pszChars[i - 2] == lptd->cEscape)))
            {
                dwCookie &= ~(COOKIE_STRING);
                bRedefineBlock = TRUE;
            }

            // Nasty but functions...just a good proof that
            // this whole function needs rewriting
            if (lptd->bPrefixesInStrings)
            {
                if (bRedefineBlock && (dwCookie & COOKIE_PREFIXED))
                    dwCookie |= COOKIE_STRING;

                if (dwCookie & COOKIE_PREFIXED)
                {
                    if (!xisalnum(pszChars[i]))
                    {
                        dwCookie &= ~(COOKIE_PREFIXED);
                        bRedefineBlock = TRUE;
                        // We have to reevaluate the last char
                        i--;
                    }
                    else
                    {
                        DEFINE_BLOCK(i, COLORINDEX_PREFIXED);
                    }
                }
                else
                {
                    if (Edit_Syntax_IsPrefix(lptd, pszChars + i))
                    {
                        DEFINE_BLOCK(i, COLORINDEX_PREFIXED);
                        dwCookie |= COOKIE_PREFIXED;
                    }
                }
            }

            continue;
        }
        
        //  Char constant ? ['...']
        if (dwCookie & COOKIE_CHAR)
        {
            if (pszChars[i] == lptd->cChar && (i == 0 || i == 1 && pszChars[i - 1] != lptd->cEscape || i >= 2 && (pszChars[i - 1] != lptd->cEscape || pszChars[i - 1] == lptd->cEscape && pszChars[i - 2] == lptd->cEscape)))
            {
                dwCookie &= ~(COOKIE_CHAR);
                bRedefineBlock = TRUE;
            }

            continue;
        }
        
        //  Extended comment End ? [*/]
        if (dwCookie & COOKIE_EXT_COMMENT)
        {
            if (((i > (lptd->nCloseCommentLen - 1) && lptd->nCloseCommentLen > 0 && String_NumEqual((pszChars + i - lptd->nCloseCommentLen), lptd->szCloseComment, lptd->nCloseCommentLen, lptd->bCase) && !bWasCommentStart) || (i >= (lptd->nCloseCommentLen - 1) && lptd->nCloseCommentLen > 0 && String_NumEqual((pszChars + i - (lptd->nCloseCommentLen - 1)), lptd->szCloseComment, lptd->nCloseCommentLen, lptd->bCase))) ||
                ((i > (lptd->nCloseComment2Len - 1) && lptd->nCloseComment2Len > 0 && String_NumEqual((pszChars + i - lptd->nCloseComment2Len), lptd->szCloseComment2, lptd->nCloseComment2Len, lptd->bCase) && !bWasCommentStart) || (i >= (lptd->nCloseComment2Len - 1) && lptd->nCloseComment2Len > 0 && String_NumEqual((pszChars + i - (lptd->nCloseComment2Len - 1)), lptd->szCloseComment2, lptd->nCloseComment2Len, lptd->bCase))))
            {
                dwCookie &= ~(COOKIE_EXT_COMMENT);
                bRedefineBlock = TRUE;
            }

            bWasCommentStart = FALSE;

            continue;
        }

        if (dwCookie & COOKIE_PREFIXED)
        {
            if (!xisalnum(pszChars[i]))
            {
                dwCookie &= ~(COOKIE_PREFIXED);
                bRedefineBlock = TRUE;
                // We have to reevaluate the last char
                i--;
            }
            else
            {
                DEFINE_BLOCK(i, COLORINDEX_PREFIXED);
            }

            continue;
        }

        if (dwCookie & COOKIE_DISABLED)
        {
            if (pszChars[i] == lptd->cEnable && (i == 0 || i == 1 && pszChars[i - 1] != lptd->cEscape || i >= 2 && (pszChars[i - 1] != lptd->cEscape || pszChars[i - 1] == lptd->cEscape && pszChars[i - 2] == lptd->cEscape)))
            {
                dwCookie &= ~(COOKIE_DISABLED);
                bRedefineBlock = TRUE;
            }

            continue;
        }

        // if the line end char is there than the next char will be the first
        // in that pseudoline
        if (pszChars[i] == lptd->cLineEnd)
        {
            bFirstChar = TRUE;

            goto ProcessWord;
//          continue;
        }

        // Start Processing

        // if the char is a linecontinuation char then assume error in user code
        // or already parsed from previous line
        // Just break because it does so in MSDEV damnit! =P
        if (pszChars[i] == lptd->cLineContinuation)
        {
            DEFINE_BLOCK(i, COLORINDEX_NORMALTEXT);

            break;
        }

        // Line Comment [//]
        if (((i >= ((nCommentLen = lptd->nLineCommentLen) - 1) && lptd->nLineCommentLen > 0 && String_NumEqual((pszChars + i - (lptd->nLineCommentLen - 1)), lptd->szLineComment, lptd->nLineCommentLen, lptd->bCase))) ||
            (i >= ((nCommentLen = lptd->nLineComment2Len) - 1) && lptd->nLineComment2Len > 0 && String_NumEqual((pszChars + i - (lptd->nLineComment2Len - 1)), lptd->szLineComment2, lptd->nLineComment2Len, lptd->bCase)))
        {
            if ((!lptd->bCommentFirstRealChar) ||
                (lptd->bCommentFirstRealChar && bFirstChar))
            {
                DEFINE_BLOCK(i - (nCommentLen - 1), COLORINDEX_COMMENT);
                dwCookie |= COOKIE_COMMENT;

                break;
            }
        }
        
        //  Preprocessor directive [#]
        if (dwCookie & COOKIE_PREPROCESSOR)
        {
            if (((i >= ((nCommentLen = lptd->nOpenCommentLen) - 1) && lptd->nOpenCommentLen > 0 && String_NumEqual((pszChars + i - (lptd->nOpenCommentLen - 1)), lptd->szOpenComment, lptd->nOpenCommentLen, lptd->bCase))) ||
                (i >= ((nCommentLen = lptd->nOpenComment2Len) - 1) && lptd->nOpenComment2Len > 0 && String_NumEqual((pszChars + i - (lptd->nOpenComment2Len - 1)), lptd->szOpenComment2, lptd->nOpenComment2Len, lptd->bCase)))
            {
                DEFINE_BLOCK(i - (nCommentLen - 1), COLORINDEX_COMMENT);
                dwCookie |= COOKIE_EXT_COMMENT;
            }

            continue;
        }

        //  Normal text
        // String ["..."]
        if (pszChars[i] == lptd->cString)
        {
            if (i > 0 && pszChars[i - 1] == lptd->cEscape)
                continue;

            DEFINE_BLOCK(i, COLORINDEX_STRING);
            dwCookie |= COOKIE_STRING;

            continue;
        }
        // Char ['...']
        if (pszChars[i] == lptd->cChar)
        {
            if (i > 0 && pszChars[i - 1] == lptd->cEscape)
                continue;

            DEFINE_BLOCK(i, COLORINDEX_STRING);
            dwCookie |= COOKIE_CHAR;

            continue;
        }

        // Open Comment [/*]
        if (((i >= ((nCommentLen = lptd->nOpenCommentLen) - 1) && lptd->nOpenCommentLen > 0 && String_NumEqual((pszChars + i - (lptd->nOpenCommentLen - 1)), lptd->szOpenComment, lptd->nOpenCommentLen, lptd->bCase))) ||
            (i >= ((nCommentLen = lptd->nOpenComment2Len) - 1) && lptd->nOpenComment2Len > 0 && String_NumEqual((pszChars + i - (lptd->nOpenComment2Len - 1)), lptd->szOpenComment2, lptd->nOpenComment2Len, lptd->bCase)))
        {
            if ((!lptd->bCommentFirstRealChar) ||
                (lptd->bCommentFirstRealChar && bFirstChar))
            {
                DEFINE_BLOCK(i - (nCommentLen - 1), COLORINDEX_COMMENT);
                dwCookie |= COOKIE_EXT_COMMENT;
                bWasCommentStart = TRUE;
            }

            continue;
        }
        
        bWasCommentStart = FALSE;
        
        if (bFirstChar)
        {
            // Preprocessor [#]
            if (pszChars[i] == lptd->cPreProcessor)
            {
                DEFINE_BLOCK(i, COLORINDEX_PREPROCESSOR);
                dwCookie |= COOKIE_PREPROCESSOR;

                continue;
            }

            // if it is whitespace then still available
            if (!_istspace(pszChars[i]))
                bFirstChar = FALSE;
        }

        // FIXME: does this work with lptd->cEscape?

        if (Edit_Syntax_IsPrefix(lptd, pszChars + i))
        {
            DEFINE_BLOCK(i, COLORINDEX_PREFIXED);
            dwCookie |= COOKIE_PREFIXED;

            continue;
        }

        if (pszChars[i] == lptd->cDisable)
        {
            if (i > 0 && pszChars[i - 1] == lptd->cEscape)
                continue;

            DEFINE_BLOCK(i, COLORINDEX_NORMALTEXT);
            dwCookie |= COOKIE_DISABLED;

            continue;
        }

        //  We don't need to extract keywords,
        //  for faster parsing skip the rest of loop
        if (pBuf == NULL)
            continue;

        if (xisalnum(pszChars[i]) || pszChars[i] == _T('.') && i > 0 && (!xisalpha(pszChars[i - 1]) && !xisalpha(pszChars[i + 1])))
        {
            if (nIdentBegin == -1)
                nIdentBegin = i;
        }
        else
        {
ProcessWord:
            if (nIdentBegin >= 0)
            {
                if (Edit_Syntax_IsKeyword(lptd, pszChars + nIdentBegin, i - nIdentBegin))
                {
                    DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
                }
                else if (Edit_Syntax_IsUser1Keyword(lptd, pszChars + nIdentBegin, i - nIdentBegin))
                {
                    DEFINE_BLOCK(nIdentBegin, COLORINDEX_USER1);
                }
                else if (Edit_Syntax_IsUser2Keyword(lptd, pszChars + nIdentBegin, i - nIdentBegin))
                {
                    DEFINE_BLOCK(nIdentBegin, COLORINDEX_USER2);
                }
                else if (Edit_Syntax_IsNumber(pszChars + nIdentBegin, i - nIdentBegin))
                {
                    DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
                }
                else
                {
                    BOOL bFunction = FALSE;
                    int j;

                    for (j = i; j < nLength; j++)
                    {
                        if (!_istspace(pszChars[j]))
                        {
                            if (pszChars[j] == lptd->cFunctionBegin)
                                bFunction = TRUE;

                            break;
                        }
                    }

                    if (bFunction)
                    {
                        DEFINE_BLOCK(nIdentBegin, COLORINDEX_FUNCNAME);
                    }
                }

                bRedefineBlock = TRUE;
                bDecIndex = TRUE;
                nIdentBegin = -1;
            }
            else if (Edit_Syntax_IsOperator(lptd, pszChars[i]))
            {
                DEFINE_BLOCK(i, COLORINDEX_OPERATOR);
                bRedefineBlock = TRUE;
                bDecIndex = TRUE;
            }
        }
    }
    
    if (nIdentBegin >= 0)
    {
        if (Edit_Syntax_IsKeyword(lptd, pszChars + nIdentBegin, i - nIdentBegin))
        {
            DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
        }
        else if (Edit_Syntax_IsUser1Keyword(lptd, pszChars + nIdentBegin, i - nIdentBegin))
        {
            DEFINE_BLOCK(nIdentBegin, COLORINDEX_USER1);
        }
        else if (Edit_Syntax_IsUser2Keyword(lptd, pszChars + nIdentBegin, i - nIdentBegin))
        {
            DEFINE_BLOCK(nIdentBegin, COLORINDEX_USER2);
        }
        else if (Edit_Syntax_IsNumber(pszChars + nIdentBegin, i - nIdentBegin))
        {
            DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
        }
        else
        {
            BOOL bFunction = FALSE;
            int j;
            
            for (j = i; j < nLength; j++)
            {
                if (!_istspace(pszChars[j]))
                {
                    if (pszChars[j] == lptd->cFunctionBegin)
                        bFunction = TRUE;

                    break;
                }
            }

            if (bFunction)
            {
                DEFINE_BLOCK(nIdentBegin, COLORINDEX_FUNCNAME);
            }
        }

    }

    if (pszChars[nLength - 1] != lptd->cLineContinuation)
    {
        dwCookie &= ~(COOKIE_COMMENT);
        dwCookie &= ~(COOKIE_STRING);
        dwCookie &= ~(COOKIE_CHAR);
        dwCookie &= ~(COOKIE_PREFIXED);
    }

    return (dwCookie);
}
