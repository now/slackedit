/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_listview.c
 * Created    : 07/01/00
 * Owner      : pcppopper
 * Revised on : 07/01/00
 * Comments   : 
 *              
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include "pcp_includes.h"
#include "pcp_definitions.h"

/* windows */
#include <commctrl.h>

/* pcp_generic */
#include "pcp_listview.h"
#include "pcp_string.h"

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

int ListView_InsertUniqueItem(HWND hwndList, LPLVITEM lplvItem)
{
    int i = 0;
    int cItems = ListView_GetItemCount(hwndList);
    LVITEM lvListItem;
    TCHAR szItem[MAX_PATH] = _T("");

    INITSTRUCT(lvListItem, FALSE);
    lvListItem.mask = LVIF_TEXT;
    lvListItem.pszText = szItem;

    for (i = 0; i < cItems; i++)
    {
        lvListItem.iItem = i;
        lvListItem.cchTextMax = MAX_PATH;

        ListView_GetItem(hwndList, &lvListItem);

        if (String_Equal(lvListItem.pszText, lplvItem->pszText, FALSE))
            return (-1);
    }

    return (ListView_InsertItem(hwndList, lplvItem));
}
