/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : clipboard_monitor.c
 * Created    : 01/05/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:33:59
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "clipboard_monitor.h"

/* pcp_generic */
#include <pcp_linkedlist.h>
#include <pcp_mem.h>
#include <pcp_string.h>
#include <pcp_path.h>

/* pcp_controls */
#include <pcp_menu.h>

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

typedef struct tagCLIPSTRUCT
{
    LPTSTR pszClip;
} CLIPSTRUCT, FAR *LPCLIPSTRUCT;

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

static PLINKEDLIST s_pllClips;
static TCHAR s_szClipFile[MAX_PATH];

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

PVOID ClipList_CreateData(PVOID pData)
{
    LPCLIPSTRUCT lpcs;

    lpcs = (LPCLIPSTRUCT)Mem_Alloc(sizeof(CLIPSTRUCT));
    lpcs->pszClip = (LPTSTR)Mem_AllocStr(_tcslen(pData) + SZ);
    _tcscpy(lpcs->pszClip, (LPTSTR)pData);

    return (lpcs);
}

INT ClipList_DeleteData(PVOID pData)
{
    LPCLIPSTRUCT lpcs = (LPCLIPSTRUCT)pData;

    Mem_Free(lpcs->pszClip);
    Mem_Free(lpcs);

    return (TRUE);
}

INT ClipList_CompareData(PVOID pFromNode, PVOID pFromCall, UINT uDataType)
{
    return (_tcscmp(((LPCLIPSTRUCT)pFromNode)->pszClip, (LPTSTR)pFromCall));
}

void Clip_Read(HMENU *hMenu)
{
    unsigned long lFileSize;
    DWORD dwClipLength = 0;
    FILE *f;
    TCHAR szFileName[MAX_PATH];

    s_pllClips = List_CreateList(ClipList_CreateData, ClipList_DeleteData,
                            NULL, ClipList_CompareData, MAX_CLIPS);

    Path_GetModuleFileName(g_hInstance, s_szClipFile, MAX_PATH);
    Path_GetModuleFileName(g_hInstance, szFileName, MAX_PATH);

    Path_RemoveFileName(s_szClipFile);
    _tcscat(s_szClipFile, _T("User\\"));
    _tcscat(s_szClipFile, Path_GetFileName(szFileName));
    Path_SwapExt(s_szClipFile, _T("clp"));

    if ((f = _tfopen(s_szClipFile, _T("r"))) == NULL)
    {
        Clip_RecreateMenu(hMenu);

        return;
    }

    fseek(f, 0, SEEK_END);
    lFileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    while (fread(&dwClipLength, sizeof(DWORD), 1, f))
    {
        LPTSTR pszBuffer = (LPTSTR)Mem_AllocStr(dwClipLength + SZ);

        fread(pszBuffer, sizeof(TCHAR), dwClipLength, f);
        List_AddNodeAtHead(s_pllClips, pszBuffer);
        Mem_Free(pszBuffer);
    }

    Clip_RecreateMenu(hMenu);

    fclose(f);
}

void Clip_Add(HWND hwnd, HMENU *hMenu)
{
    LPTSTR pszClipboard;
    HGLOBAL hglb;
    PLISTNODE pNode;

    if (!IsClipboardFormatAvailable(CF_TEXT))
        return;

    OpenClipboard(hwnd);

    hglb            = GetClipboardData(CF_TEXT);
    pszClipboard    = (LPTSTR)GlobalLock(hglb);

    if ((pNode = List_FindNode(s_pllClips, pszClipboard, 0)) != NULL)
    {
        // Item in list already so put it at the head
        List_DeleteNode(s_pllClips, pNode);
    }

    List_AddNodeAtHead(s_pllClips, pszClipboard);

    GlobalUnlock(hglb);
    CloseClipboard();

    Clip_RecreateMenu(hMenu);
}

void Clip_Save()
{
    PLISTNODE pNode = s_pllClips->pHeadNode, pTempNode;
    DWORD dwClipLength;
    FILE *f;

    f = fopen(s_szClipFile, _T("w"));

    while (pNode != NULL)
    {
        dwClipLength = _tcslen(((LPCLIPSTRUCT)pNode->pData)->pszClip);

        fwrite(&dwClipLength, sizeof(DWORD), 1, f);
        fwrite(((LPCLIPSTRUCT)pNode->pData)->pszClip, sizeof(TCHAR), dwClipLength, f);

        pTempNode = pNode->pNextNode;
        List_DeleteNode(s_pllClips, pNode);
        pNode = pTempNode;
    }

    fclose(f);
}

void Clip_RecreateMenu(HMENU *fMenu)
{
    MENUITEMINFO mii;
    PLISTNODE pNode = s_pllClips->pHeadNode;
    int i;
    int nMenuCount;

    if (IsMenu(*fMenu))
    {
        DestroyMenu(*fMenu);
    }

    *fMenu  = CreatePopupMenu();
    
    INITSTRUCT(mii, TRUE);
    mii.fMask           = MIIM_TYPE | MIIM_ID;
    mii.fType           = MFT_STRING;

    // Iterate through the whole list and add a menuitem for each one
    for (i = 0; pNode != NULL; i++)
    {
        TCHAR szMenu[64] = _T("");
        TCHAR szTemp[54] = _T("");

        _tcsncpy(szTemp, ((LPCLIPSTRUCT)pNode->pData)->pszClip, 50);
        if (_tcslen(szTemp) == 50)
            _tcscat(szTemp, _T("..."));

        _stprintf(szMenu, String_LoadString(IDS_FORMAT_GENERIC_LIST_MENUITEM),
                ((i + 1) == 10) ? 0 : i + 1, szTemp);

        mii.wID             = IDM_EDIT_FIRSTCLIP + i;
        mii.dwTypeData      = szMenu;
        InsertMenuItem(*fMenu, i, FALSE, &mii);

        pNode = pNode->pNextNode;
    }

    // Were any items inserted?
    if (i == 0)
    {
        mii.dwTypeData  = _T("No Recent Clips");
        InsertMenuItem(*fMenu, i, FALSE, &mii);
    }

    Menu_CreateMenu(*fMenu, (INT *)IDP_NONE);

    nMenuCount = GetMenuItemCount(*fMenu);

    for (i = 0; i < nMenuCount; i++)
        Menu_SetItemFlags(*fMenu, i, TRUE, ODMFT_DONTEXPANDTABS);
}

LPCTSTR ClipboardMonitor_GetClip(int nNode)
{
    PLISTNODE pNode = List_GetNode(s_pllClips, nNode);

    return (((LPCLIPSTRUCT)pNode->pData)->pszClip);
}
