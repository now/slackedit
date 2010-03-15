/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit.c
 * Created    : not known (before 01/24/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:43:07
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* resources */
#include "resource.h"

/* pcp_generic */
#include <pcp_string.h>

/* pcp_edit */
#include "pcp_edit.h"
#include "pcp_textedit_interface.h"

/****************************************************************
 * Definitions
 ****************************************************************/

/****************************************************************
 * Function Definitions
 ****************************************************************/

/****************************************************************
 * Global Variables
 ****************************************************************/

/****************************************************************
 * Function Implementations
 ****************************************************************/

LPEDITINTERFACE PCP_Edit_GetInterface(HWND hwnd)
{
    LPTEXTEDITINTERFACE lpteInterface;

    lpteInterface = TextEdit_Internal_GetInterface(hwnd);

    if (lpteInterface != NULL)
    {
        return ((LPEDITINTERFACE)lpteInterface);
    }
/*  else
    {
        LPHEXEDITINTERFACE lpheInterface;

        lpheInterface = PCP_Edit_HexEdit_Internal_GetInterface(hwnd);

        if (lpheInterface != NULL)
            return ((LPEDITINTERFACE)lpheInterface->lpeInterface);
    }
*/
    ASSERT(FALSE);

    return (NULL);
}

BOOL PCP_Edit_GetActionDescription(int nAction, LPTSTR pszDesc)
{
    HINSTANCE hOldInstance = String_SetResourceHandle(PCP_Edit_Dll_GetHandle());

    switch (nAction)
    {
        case PE_ACTION_UNKNOWN:
            _tcscpy(pszDesc, String_LoadString(IDS_EDITOP_UNKNOWN));
        break;
        case PE_ACTION_PASTE:
            _tcscpy(pszDesc, String_LoadString(IDS_EDITOP_PASTE));
        break;
        case PE_ACTION_DELSEL:
            _tcscpy(pszDesc, String_LoadString(IDS_EDITOP_DELSELECTION));
        break;
        case PE_ACTION_CUT:
            _tcscpy(pszDesc, String_LoadString(IDS_EDITOP_CUT));
        break;
        case PE_ACTION_TYPING:
            _tcscpy(pszDesc, String_LoadString(IDS_EDITOP_TYPING));
        break;
        case PE_ACTION_BACKSPACE:
            _tcscpy(pszDesc, String_LoadString(IDS_EDITOP_BACKSPACE));
        break;
        case PE_ACTION_INDENT:
            _tcscpy(pszDesc, String_LoadString(IDS_EDITOP_INDENT));
        break;
        case PE_ACTION_DRAGDROP:
            _tcscpy(pszDesc, String_LoadString(IDS_EDITOP_DRAGDROP));
        break;
        case PE_ACTION_REPLACE:
            _tcscpy(pszDesc, String_LoadString(IDS_EDITOP_REPLACE));
        break;
        case PE_ACTION_DELETE:
            _tcscpy(pszDesc, String_LoadString(IDS_EDITOP_DELETE));
        break;
        case PE_ACTION_AUTOINDENT:
            _tcscpy(pszDesc, String_LoadString(IDS_EDITOP_AUTOINDENT));
        break;
    }

    String_SetResourceHandle(hOldInstance);

    return (TRUE);
}
