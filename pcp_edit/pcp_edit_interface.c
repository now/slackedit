/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_interface.c
 * Created    : 07/22/00
 * Owner      : pcppopper
 * Revised on : 07/22/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include <pcp_includes.h>

/* pcp_edit */
#include <pcp_edit_internal.h>

/****************************************************************
 * Type Definitions
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

LRESULT CALLBACK PCP_Edit_Interface_HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPEDITVIEW lpev = (LPEDITVIEW)GetWindowLong(hwnd, GWL_USERDATA);
    switch (uMsg)
    {
    case PEM_CANPASTE:
    return (Edit_View_TextInClipboard(lpev));
    case PEM_CANREDO:
    return (Edit_Buffer_CanRedo(lpev->lpes));
    case PEM_CANUNDO:
    return (Edit_Buffer_CanUndo(lpev->lpes));
    case PEM_GETSELTEXT:
    {
        POINT ptSelStart, ptSelEnd;

        Edit_View_GetSelection(lpev, &ptSelStart, &ptSelEnd);
        Edit_View_GetText(lpev, ptSelStart, ptSelEnd, (LPTSTR *)lParam);
    }
    return (TRUE);
    case PEM_GETSEL:
        Edit_View_GetSelection(lpev, (LPPOINT)wParam, (LPPOINT)lParam);
    return (TRUE);
    case PEM_GETSELXY:
        Edit_View_GetSelectionXY(lpev, (LPDWORD)wParam, (LPDWORD)lParam);
    return (TRUE);
    case PEM_SETSEL:
        Edit_View_SetCursorPos(lpev, *(LPPOINT)lParam);
        Edit_View_SetSelection(lpev, *(LPPOINT)wParam, *(LPPOINT)lParam);
        Edit_View_SetAnchor(lpev, *(LPPOINT)lParam);
        Edit_View_EnsureVisible(lpev, *(LPPOINT)lParam);
    return (TRUE);
    case PEM_SETSELXY:
        Edit_View_SetSelectionXY(lpev, wParam, lParam);
    return (TRUE);
    case PEM_REPLACESEL:
        Edit_View_ReplaceSel(lpev, (LPTSTR)lParam);
    return (TRUE);
    case PEM_LINECOUNT:
    return (Edit_View_GetLineCount(lpev));
    case PEM_LINELENGTH:
    return (Edit_View_GetLineLength(lpev, (INT)lParam));
    case PEM_GETLINE:
        return (_tcslen(_tcscpy((LPTSTR)lParam, Edit_View_GetLineChars(lpev, (INT)wParam))));
    return (TRUE);
    case PEM_GETCURSORPOS:
        *(LPPOINT)lParam = Edit_View_GetCursorPos(lpev);
    return (TRUE);
    case PEM_UNDO:
        Edit_View_OnUndo(lpev);
    return (TRUE);
    case PEM_REDO:
        Edit_View_OnRedo(lpev);
    return (TRUE);
    case PEM_GETMODIFY:
    return (lpev->lpes->bModified);
    case PEM_SETMODIFY:
    {
        BOOL fOld = lpev->lpes->bModified;

        lpev->lpes->bModified = (BOOL)lParam;

        return (fOld);
    }
    break; //already returned
    case PEM_LOADFILE:
        Edit_View_LoadFile(lpev, (LPCTSTR)lParam);
    return (0);
    case PEM_GETOVERWRITEMODE:
    return (lpev->bOvrMode);
    case PEM_SETOVERWRITEMODE:
    {
        BOOL bOldMode = lpev->bOvrMode;

        lpev->bOvrMode = !lpev->bOvrMode;

        Edit_View_UpdateCaret(lpev);

        return (bOldMode);
    }
    default:
    break;
    }

    DefWindowProc(lpev->hwnd, uMsg, wParam, lParam));
}
