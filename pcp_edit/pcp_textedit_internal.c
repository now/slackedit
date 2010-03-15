/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_textedit_internal.c
 * Created    : 07/23/00
 * Owner      : pcppopper
 * Revised on : 07/23/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* pcp_generic */
#include <pcp_linkedlist.h>
#include <pcp_mem.h>
#include <pcp_string.h>

/* pcp_edit */
#include "pcp_textedit.h"
#include "pcp_textedit_view.h"
#include "pcp_textedit_internal.h"
#include "pcp_textedit_interface.h"

/****************************************************************
 * Type Definitions
 ****************************************************************/

enum
{
    SL_FIND_EXT,
    SL_FIND_FIRSTPLAIN
};

/****************************************************************
 * Function Definitions
 ****************************************************************/

static INT SyntaxList_CompareData(PVOID pFromNode, PVOID pFromCall, UINT uDataType);
static PVOID SyntaxList_CreateData(PVOID pData);
static INT SyntaxList_DeleteData(PVOID pData);

/****************************************************************
 * Global Variables
 ****************************************************************/

static PLINKEDLIST s_pSyntaxList = NULL;
static CRITICAL_SECTION s_SyntaxCriticalSection;

/****************************************************************
 * Function Implementations
 ****************************************************************/

BOOL TextEdit_Internal_RegisterControl(HMODULE hModule)
{
    WNDCLASSEX wcex;
    WNDCLASS wc;

    ZeroMemory(&wc, sizeof(WNDCLASS));
    wcex.cbSize = sizeof(WNDCLASSEX);

    if (GetClassInfoEx(hModule, WC_PCP_TEXTEDIT, &wcex) != 0)
        return (TRUE);

    wc.style            = CS_SAVEBITS | CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_GLOBALCLASS;
    wc.lpfnWndProc      = (WNDPROC)TextEdit_View_WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hModule;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_IBEAM);
    wc.hbrBackground    = (HBRUSH)GetSysColorBrush(COLOR_BTNFACE);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = WC_PCP_TEXTEDIT;

    if (!RegisterClass(&wc))
        return (FALSE);

    TextEdit_Internal_CreateSyntaxDefinitionsList();
    TextEdit_Internal_SetupInterface();

    return (TRUE);
}

void TextEdit_Internal_UnregisterControl(HMODULE hModule)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    if (GetClassInfoEx(hModule, WC_PCP_TEXTEDIT, &wcex) == FALSE)
        return;

    UnregisterClass(WC_PCP_TEXTEDIT, hModule);

    TextEdit_Internal_DeleteInterface();

    List_DestroyList(s_pSyntaxList);

    DeleteCriticalSection(&s_SyntaxCriticalSection);
}

void TextEdit_Internal_CreateSyntaxDefinitionsList(void)
{
    SYNTAXDEFINITION sd;

    InitializeCriticalSection(&s_SyntaxCriticalSection);

    s_pSyntaxList = List_CreateList(SyntaxList_CreateData, SyntaxList_DeleteData, NULL, SyntaxList_CompareData, LL_UNLIMITEDSIZE);

    INITSTRUCT(sd, FALSE);
    sd.bPlain           = TRUE;
    _tcscpy(sd.szName, _T("Plain"));
    _tcscpy(sd.szExts, _T("txt,diz,nfo"));
    sd.dwFlags          = SRCOPT_AUTOINDENT|SRCOPT_INSERTTABS|SRCOPT_BSATBOL|SRCOPT_SELMARGIN|SRCOPT_BRACEANSI|SRCOPT_EOLNDOS;
    sd.dwTabSize        = 4;
    sd.crBackground     = -1;
    sd.crComment        = -1;
    sd.crFunctionName   = -1;
    sd.crKeyword        = -1;
    sd.crNormalText     = -1;
    sd.crNumber         = -1;
    sd.crOperator       = -1;
    sd.crPreprocessor   = -1;
    sd.crPrefixed       = -1;
    sd.crSelBackground  = -1;
    sd.crSelMargin      = -1;
    sd.crSelText        = -1;
    sd.crString         = -1;
    sd.crUser1          = -1;
    sd.crUser2          = -1;
    sd.crWhiteSpace     = -1;

    TextEdit_Internal_AddSyntaxDefinition(&sd);
}

void TextEdit_Internal_AddSyntaxDefinition(LPSYNTAXDEFINITION lpsd)
{
    EnterCriticalSection(&s_SyntaxCriticalSection);

    List_AddNodeAtTail(s_pSyntaxList, lpsd);

    LeaveCriticalSection(&s_SyntaxCriticalSection);
}

LPSYNTAXDEFINITION TextEdit_Internal_GetSyntaxDefintion(LPCTSTR pszExt)
{
    PLISTNODE pNode;
    BOOL bSuccess = FALSE;

    EnterCriticalSection(&s_SyntaxCriticalSection);

    if (pszExt == NULL)
    {
        pNode = List_FindNode(s_pSyntaxList, NULL, SL_FIND_FIRSTPLAIN);
    }
    else
    {
        pNode = List_FindNode(s_pSyntaxList, pszExt, SL_FIND_EXT);

        if (pNode == NULL)
            pNode = List_FindNode(s_pSyntaxList, NULL, SL_FIND_FIRSTPLAIN);
    }

    LeaveCriticalSection(&s_SyntaxCriticalSection);

    ASSERT(pNode != NULL);

    return ((LPSYNTAXDEFINITION)pNode->pData);
}

PVOID SyntaxList_CreateData(PVOID pData)
{
    LPSYNTAXDEFINITION lpsdDst = (LPSYNTAXDEFINITION)Mem_Alloc(sizeof(SYNTAXDEFINITION));
    LPSYNTAXDEFINITION lpsdSrc = (LPSYNTAXDEFINITION)pData;

    *lpsdDst = *lpsdSrc;

    return (lpsdDst);
}

INT SyntaxList_DeleteData(PVOID pData)
{
    LPSYNTAXDEFINITION lpsd = (LPSYNTAXDEFINITION)pData;
    int i;

    for (i = 0; lpsd->apszIndent[i] != NULL; i++)
        Mem_Free(lpsd->apszIndent[i]);

    for (i = 0; lpsd->apszKeywords[i] != NULL; i++)
        Mem_Free(lpsd->apszKeywords[i]);

    for (i = 0; lpsd->apszPrefixes[i] != NULL; i++)
        Mem_Free(lpsd->apszPrefixes[i]);

    for (i = 0; lpsd->apszUnIndent[i] != NULL; i++)
        Mem_Free(lpsd->apszUnIndent[i]);

    for (i = 0; lpsd->apszUser1Keywords[i] != NULL; i++)
        Mem_Free(lpsd->apszUser1Keywords[i]);

    for (i = 0; lpsd->apszUser2Keywords[i] != NULL; i++)
        Mem_Free(lpsd->apszUser2Keywords[i]);

    return (TRUE);
}

INT SyntaxList_CompareData(const PVOID pFromNode, const PVOID pFromCall, UINT uDataType)
{
    const LPSYNTAXDEFINITION lpsd = (const LPSYNTAXDEFINITION)pFromNode;
    LPTSTR pszPattern;
    LPTSTR pszPart;
    LPCTSTR pszExt = (LPCTSTR)pFromCall;

    if (uDataType == SL_FIND_EXT)
    {
        ASSERT(pszExt != NULL);
        pszPattern = String_Duplicate(lpsd->szExts);

        pszPart = _tcstok(pszPattern, _T(","));

        do
        {
            if (String_Equal(pszPart, pszExt, FALSE))
            {
                Mem_Free(pszPattern);

                return (0);
            }
        }
        while ((pszPart = _tcstok(NULL, _T(","))) != NULL);

        Mem_Free(pszPattern);
    }
    else if (uDataType == SL_FIND_FIRSTPLAIN)
    {
        if (lpsd->bPlain)
            return (0);
    }
    else
    {
        ASSERT(FALSE);
    }

    return (-1);
}
