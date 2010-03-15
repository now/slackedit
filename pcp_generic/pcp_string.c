/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_string.c
 * Created    : not known (before 07/17/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:24:59
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"
#include "pcp_string.h"

#include "pcp_string.h"
#include "pcp_mem.h"
#include "pcp_debug.h"

int xisspecial(int c);

/*
    { _T("left_button"),    VK_LBUTTON },
    { _T("right_button"),   VK_RBUTTON },
    { _T("cancel"),         VK_CANCEL },
    { _T("middle_button"),  VK_MBUTTON },
*/

struct tagKEYVALUE
{
    LPCTSTR pszName;
    WORD    wValue;
} s_aKeyValues[] =
{
    { _T("backspace"),      VK_BACK },
    { _T("tab"),            VK_TAB },
    { _T("clear"),          VK_CLEAR },
    { _T("return"),         VK_RETURN },
    { _T("enter"),          VK_RETURN },
    { _T("escape"),         VK_ESCAPE },
    { _T("space"),          VK_SPACE },
    { _T("prior"),          VK_PRIOR },
    { _T("page_up"),        VK_PRIOR },
    { _T("next"),           VK_NEXT },
    { _T("page_down"),      VK_NEXT },
    { _T("end"),            VK_END },
    { _T("home"),           VK_HOME },
    { _T("left"),           VK_LEFT },
    { _T("up"),             VK_UP },
    { _T("right"),          VK_RIGHT },
    { _T("down"),           VK_DOWN },
    { _T("select"),         VK_SELECT },
    { _T("print"),          VK_PRINT },
    { _T("execute"),        VK_EXECUTE },
    { _T("snapshot"),       VK_SNAPSHOT },
    { _T("print_screen"),   VK_SNAPSHOT },
    { _T("insert"),         VK_INSERT },
    { _T("ins"),            VK_INSERT },
    { _T("delete"),         VK_DELETE },
    { _T("del"),            VK_DELETE },
    { _T("help"),           VK_HELP },
    { _T("0"),              _T('0') },
    { _T("1"),              _T('1') },
    { _T("2"),              _T('2') },
    { _T("3"),              _T('3') },
    { _T("4"),              _T('4') },
    { _T("5"),              _T('5') },
    { _T("6"),              _T('6') },
    { _T("7"),              _T('7') },
    { _T("8"),              _T('8') },
    { _T("9"),              _T('9') },
    { _T("a"),              _T('A') },
    { _T("b"),              _T('B') },
    { _T("c"),              _T('C') },
    { _T("d"),              _T('D') },
    { _T("e"),              _T('E') },
    { _T("f"),              _T('F') },
    { _T("g"),              _T('G') },
    { _T("h"),              _T('H') },
    { _T("i"),              _T('I') },
    { _T("j"),              _T('J') },
    { _T("k"),              _T('K') },
    { _T("l"),              _T('L') },
    { _T("m"),              _T('M') },
    { _T("n"),              _T('N') },
    { _T("o"),              _T('O') },
    { _T("p"),              _T('P') },
    { _T("q"),              _T('Q') },
    { _T("r"),              _T('R') },
    { _T("s"),              _T('S') },
    { _T("t"),              _T('T') },
    { _T("u"),              _T('U') },
    { _T("v"),              _T('V') },
    { _T("w"),              _T('W') },
    { _T("x"),              _T('X') },
    { _T("y"),              _T('Y') },
    { _T("z"),              _T('Z') },
    { _T("numpad0"),        VK_NUMPAD0 },
    { _T("numpad1"),        VK_NUMPAD1 },
    { _T("numpad2"),        VK_NUMPAD2 },
    { _T("numpad3"),        VK_NUMPAD3 },
    { _T("numpad4"),        VK_NUMPAD4 },
    { _T("numpad5"),        VK_NUMPAD5 },
    { _T("numpad6"),        VK_NUMPAD6 },
    { _T("numpad7"),        VK_NUMPAD7 },
    { _T("numpad8"),        VK_NUMPAD8 },
    { _T("numpad9"),        VK_NUMPAD9 },
    { _T("multiply"),       VK_MULTIPLY },
    { _T("*"),              VK_MULTIPLY },
    { _T("add"),            VK_ADD },
    { _T("+"),              VK_ADD },
    { _T("subtract"),       VK_SUBTRACT },
    { _T("-"),              VK_SUBTRACT },
    { _T("decimal"),        VK_DECIMAL },
    { _T("divide"),         VK_DIVIDE },
    { _T("/"),              VK_DIVIDE },
    { _T("f1"),             VK_F1 },
    { _T("f2"),             VK_F2 },
    { _T("f3"),             VK_F3 },
    { _T("f4"),             VK_F4 },
    { _T("f5"),             VK_F5 },
    { _T("f6"),             VK_F6 },
    { _T("f7"),             VK_F7 },
    { _T("f8"),             VK_F8 },
    { _T("f9"),             VK_F9 },
    { _T("f10"),            VK_F10 },
    { _T("f11"),            VK_F11 },
    { _T("f12"),            VK_F12 },
    { _T("f13"),            VK_F13 },
    { _T("f14"),            VK_F14 },
    { _T("f15"),            VK_F15 },
    { _T("f16"),            VK_F16 },
    { _T("f17"),            VK_F17 },
    { _T("f18"),            VK_F18 },
    { _T("f19"),            VK_F19 },
    { _T("f20"),            VK_F20 },
    { _T("f21"),            VK_F21 },
    { _T("f22"),            VK_F22 },
    { _T("f23"),            VK_F23 },
    { _T("f24"),            VK_F24 },
    { _T("numlock"),        VK_NUMLOCK },
    { _T("num_lock"),       VK_NUMLOCK },
    { _T("scroll"),         VK_SCROLL },
    { _T("scroll_lock"),    VK_SCROLL },
    { (LPTSTR)NULL,         0 },
};

static HINSTANCE s_hStringResource = NULL;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

void String_TrimChar(LPTSTR pszStr, TCHAR chStrip)
{
    int i, j;

    for (i = j = 0; pszStr[i] != '\0'; i++)
        if (pszStr[i] != chStrip)
            pszStr[j++] = pszStr[i];

    pszStr[j] = '\0';
}

void String_Trim(LPTSTR pszStr, LPTSTR pszStrip)
{
    int i, j, k;
    BOOL fFound = FALSE;

    for (i = j = 0; pszStr[i] != '\0'; i++)
    {
        for (k = 0; pszStrip[k] != '\0'; k++)
        {
            if (pszStr[i] == pszStrip[k])
            {
                fFound = TRUE;
                break;
            }
        }

        if (!fFound)
            pszStr[j++] = pszStr[i];
        else
            fFound = FALSE;
    }

    pszStr[j] = '\0';
}

void String_TrimRight(LPTSTR pszStr, TCHAR chStrip)
{
    LPTSTR p = pszStr;
    int i = _tcslen(p) - SZ;

    if ((int)chStrip == -1)
    {
        while (_istspace(p[i]))
            i--;
    }
    else
    {
        while (p[i] == chStrip)
            i--;
    }

    *(p + i + SZ) = '\0';
}

void String_TrimLeft(LPTSTR pszStr, TCHAR chStrip)
{
    int i = 0, iLen;

    if ((int)chStrip == -1)
    {
        while (_istspace(pszStr[i]))
            i++;
    }
    else
    {
        while (pszStr[i] == chStrip)
            i++;
    }

    iLen = _tcslen(pszStr + i);
    memmove(pszStr, (pszStr + i), iLen);
    pszStr[iLen] = '\0';
}

BOOL String_Cut(LPTSTR pszStr, int nStart, int nLen)
{
    LPTSTR psz;

    if (nStart + nLen > (int)_tcslen(pszStr))
        nLen = _tcslen(pszStr) - nStart - 1;

    psz = (LPTSTR)(pszStr + nStart);
    _tcscpy(psz, (psz + nLen));

    return (TRUE);
}


LPTSTR String_Right(LPCTSTR pszStr, int nCount)
{
    if (nCount < 0)
        nCount = 0;

    if (nCount >= (int)_tcslen(pszStr))
        return ((LPTSTR)pszStr);
    else
        return ((LPTSTR)pszStr + _tcslen(pszStr) - nCount);
}

LPTSTR String_Left(LPCTSTR pszStr, int nCount)
{
    if (nCount < 0)
        nCount = 0;

    if (nCount >= (int)_tcslen(pszStr))
    {
        return ((LPTSTR)pszStr);
    }
    else
    {
        LPTSTR pszReturn = (LPTSTR)Mem_AllocStr(nCount);
        _tcsncpy(pszReturn, pszStr, nCount);

        return (pszReturn);
    }
}

int String_ReplaceChar(LPTSTR pszStr, TCHAR chReplace, TCHAR chWith)
{
    int i = 0;
    int nCount = 0;

    while (pszStr[i] != _T('\0'))
    {
        if (pszStr[i] == chReplace)
        {
            nCount++;
            pszStr[i] = chWith;
        }

        i++;
    }

    return (nCount);
}

LPTSTR String_Mid(LPCTSTR pszSrc, int iFirst, int iCount)
{
    int iLen = _tcslen(pszSrc);
    LPTSTR pszDst = NULL;

    if (iFirst < 0)
        iFirst = 0;
    if (iCount < 0)
        iCount = 0;

    if (iFirst + iCount > iLen)
        iCount = iLen - iFirst;
    if (iFirst > iLen)
        iCount = 0;

    if ((iFirst == 0) && ((iFirst + iCount) == iLen))
        return (String_Duplicate(pszSrc));

    pszDst = (LPTSTR)Mem_AllocStr(iCount - iFirst + SZ);
    memcpy(pszDst, (pszSrc + iFirst), (iCount - iFirst));
    memcpy((pszDst + (iCount - iFirst) - 1), _T('\0'), 1);

    return (pszDst);
}

void String_SetAt(LPTSTR pszSrc, int iIndex, TCHAR ch)
{
    ASSERT(iIndex >= 0);
    ASSERT(iIndex < (int)_tcslen(pszSrc));

    pszSrc[iIndex] = ch;
}

BOOL String_Insert(LPTSTR *pszInto, LPCTSTR pszAdd, int iIndex)
{
    int iInsertLength;
    int iNewLength;

    if (iIndex < 0)
        iIndex = 0;

    iInsertLength   = _tcslen(pszAdd);
    iNewLength      = _tcslen(*pszInto);

    if (iInsertLength > 0)
    {
        if (iIndex > iNewLength)
            iIndex = iNewLength;

        iNewLength += iInsertLength;
        
        *pszInto = (LPTSTR)Mem_ReAllocStr(*pszInto, iNewLength + SZ);

        memcpy(*pszInto + iIndex + iInsertLength, *pszInto + iIndex,
                (iNewLength - iIndex - iInsertLength + 1) * sizeof(TCHAR));
        memcpy(*pszInto + iIndex, pszAdd, iInsertLength * sizeof(TCHAR));
    }

    return (TRUE);
}

HINSTANCE String_SetResourceHandle(HINSTANCE hResource)
{
    HINSTANCE hTempInstance = s_hStringResource;

    s_hStringResource = hResource;

    // return the old handle
    return (hTempInstance);
}

HINSTANCE String_GetResourceHandle(void)
{
    return (s_hStringResource);
}

LPTSTR String_LoadString(UINT uID)
{
    static TCHAR szString[1024];

    LoadString(s_hStringResource, uID, szString, 1024);

    return (szString);
}

LPTSTR String_Duplicate(LPCTSTR pszSrc)
{
    LPTSTR pszDst = NULL;

    if (pszSrc != NULL)
    {
        pszDst = (LPTSTR)Mem_AllocStr(_tcslen(pszSrc) + SZ);

        if (pszDst != NULL)
            _tcscpy(pszDst, pszSrc);
    }

    return (pszDst);
}

void String_ProcessHotkey(LPCTSTR pszHotkey, LPACCEL lpAccel)
{
    LPTSTR pszCopy;
    LPTSTR psz;
    int i;

    ZeroMemory(lpAccel, sizeof(ACCEL));
    lpAccel->fVirt = FVIRTKEY | FNOINVERT;

    pszCopy = String_Duplicate(pszHotkey);
    psz = _tcstok(pszCopy, _T("+-"));

    do
    {
        if (String_Equal(psz, _T("control"), FALSE) ||
            String_Equal(psz, _T("ctrl"), FALSE))
        {
            lpAccel->fVirt |= FCONTROL;
        }
        else if (String_Equal(psz, _T("alt"), FALSE))
        {
            lpAccel->fVirt |= FALT;
        }
        else if (String_Equal(psz, _T("shift"), FALSE))
        {
            lpAccel->fVirt |= FSHIFT;
        }
        else
        {
            break;
        }
    } while ((psz = _tcstok(NULL, _T("+-"))) != NULL);

    if (psz == NULL || *psz == _T('\0'))
    {
        lpAccel->key = 0;

        return;
    }

    for (i = 0; s_aKeyValues[i].pszName != NULL; i++)
    {
        if (String_Equal(s_aKeyValues[i].pszName, psz, FALSE))
            break;
    }

    lpAccel->key = s_aKeyValues[i].wValue;

    Mem_Free(pszCopy);
}

COLORREF String_MakeRGB(LPCTSTR pszRGB)
{
    int r = 0, g = 0, b = 0;
    TCHAR szHex[3];
    LPTSTR pszStop;

    if(_tcslen(pszRGB) != 6)
        return (-1);

    ZeroMemory(szHex, sizeof(szHex));

    _tcsncpy(szHex, pszRGB, 2);
    if (ishex(szHex))
        r = _tcstol(szHex, &pszStop, 16);
    else
        return (-1);

    _tcsncpy(szHex, (pszRGB + 2), 2);
    if (ishex(szHex))
        g = _tcstol(szHex, &pszStop, 16);
    else
        return (-1);

    _tcsncpy(szHex, (pszRGB + 4), 2);
    if (ishex(szHex))
        b = _tcstol(szHex, &pszStop, 16);
    else
        return (-1);

    return (RGB(r, g, b));
}

int String_CountSubStrings(LPCTSTR pszSource, LPCTSTR pszSubString)
{
    LPCTSTR p = pszSource;
    int nCnt = 0, nLength = _tcslen(pszSubString);

    while ((p = _tcsstr(p, pszSubString)) != NULL)
    {
        nCnt++;
        p += nLength;
    }

    return (nCnt);
}


int String_CountSubChars(LPCTSTR pszSource, TCHAR chSubChar)
{
    LPCTSTR p = pszSource;
    int nCnt = 0;

    while ((p = _tcschr(p, chSubChar)) != NULL)
    {
        nCnt++;
        p++;
    }

    return (nCnt);
}

__inline BOOL String_Equal(LPCTSTR pszEins, LPCTSTR pszZwei, BOOL bCase)
{
    return ((bCase) ? ((_tcscmp(pszEins, pszZwei) == 0) ? TRUE : FALSE) : ((_tcsicmp(pszEins, pszZwei) == 0) ? TRUE : FALSE));
}

__inline BOOL String_NumEqual(LPCTSTR pszEins, LPCTSTR pszZwei, size_t nCount, BOOL bCase)
{
    return ((bCase) ? ((_tcsncmp(pszEins, pszZwei, nCount) == 0) ? TRUE : FALSE) : ((_tcsnicmp(pszEins, pszZwei, nCount) == 0) ? TRUE : FALSE));
}

__inline int xisspecial(int c)
{
    return ((unsigned)c > (unsigned)_T('\x7f') || c == _T('_'));
}

__inline int xisalpha(int c)
{
  return _istalpha(c) || xisspecial(c);
}

__inline int xisalnum(int c)
{
  return _istalnum(c) || xisspecial(c);
}

BOOL ishex(LPTSTR pszString)
{
    int i;

    for (i = 0; pszString[i] != _T('\0'); i++)
    {
        if ((pszString[i] <=  _T('9') && pszString[i] >= _T('0')) || (pszString[i] <= _T('F') && pszString[i] >= _T('A')) || (pszString[i] <= _T('f') && pszString[i] >= _T('a')))
            continue;
        else
            return (FALSE);
    }

    return (TRUE);
}

INT bracetype(TCHAR ch)
{
    static LPCTSTR braces = _T("{([<>])}");
    LPCTSTR pos = _tcschr(braces, ch);

    return (pos ? pos - braces + 1 : 0);
}

__inline INT bracetypesz(LPCTSTR psz)
{
    if (psz[1])
        return (0);
  
    return (bracetype(*psz));
}

INT bracebuddy(TCHAR ch)
{
    static LPCTSTR braces = _T("{([<>])}");
    LPCTSTR pos = _tcschr(braces, ch);

    return ((pos) ? (_tcslen(braces) - (pos - braces)) : 0);
}

__inline bracebuddysz(LPCTSTR psz)
{
    if (psz[1])
        return (0);

    return (bracebuddy(*psz));
}

BOOL String_IsSpace(LPCTSTR psz)
{
    for ( ; *psz; psz++)
    {
        if (!_istspace(*psz))
            return (FALSE);
    }

    return (TRUE);
}
