/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pcp_hexedit_child.c
 * Created    : 02/02/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:19:27
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* SlackEdit */
#include "../slack_main.h"
#include "../controls/pcp_hexedit.h"

/* pcp_generic */
#include <pcp_rect.h>

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

void HexEdit_Create(HWND hwndOwner, LPTSTR pszFile)
{
    HWND hwndEdit;
    RECT rc;

    GetClientRect(hwndOwner, &rc);

    hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, WC_HEXEDIT, _T(""),
                WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_OVERLAPPED,
                0, 0, Rect_Width(&rc), Rect_Height(&rc),
                hwndOwner, (HMENU)IDC_EDIT_MDICHILD, g_hInstance, pszFile);

    ASSERT(hwndEdit != NULL);

//  lpfnOldWndProc = SubclassWindow(hwndEdit, HexEdit_WndProc);
}
