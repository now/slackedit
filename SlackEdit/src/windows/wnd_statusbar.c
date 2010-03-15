/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : statusbar.c
 * Created    : not known (before 01/02/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:14:55
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/*  windows */
#include <commctrl.h>

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"

/* pcp_generic */
#include <pcp_mem.h>
#include <pcp_string.h>

/* pcp_controls */
#include <pcp_statusbar.h>

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

HWND MyStatusbar_Create(HWND hwndParent)
{
    LPSTATUSBARPANE *alpsp = (LPSTATUSBARPANE *)Mem_Alloc(sizeof(LPSTATUSBARPANE) * 5);
    STATUSBARCREATEDATA sbcd;
    HWND hwndStatusbar;
    int i;
    
    for (i = 0; i < 5; i++)
    {
        alpsp[i] = (LPSTATUSBARPANE)Mem_Alloc(sizeof(STATUSBARPANE));

        alpsp[i]->fMask         = SF_ALL;
        alpsp[i]->nPane         = i;
        alpsp[i]->crBg          = SB_DEFAULT_CRBG;
        alpsp[i]->crFg          = SB_DEFAULT_CRFG;
        alpsp[i]->uDrawStyle    = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
        alpsp[i]->uOwnStyle     = SFS_BORDER | SFS_AUTOSIZE;
        alpsp[i]->nWidth        = 0;
        alpsp[i]->uStyle        = 0;
    }

    // Help pane
    alpsp[0]->pszText       = String_Duplicate(String_LoadString(IDS_STATUSBARPANE_READY));
    alpsp[0]->uDrawStyle    &= ~DT_CENTER;
    alpsp[0]->uStyle        = SBT_NOBORDERS;
    alpsp[0]->uOwnStyle     = SFS_BORDER;

    // Position Info Pane
    alpsp[1]->pszText       = _T("");

    // File Type Pane
    alpsp[2]->pszText       = String_Duplicate(String_LoadString(IDS_STATUSBARPANE_UNIX));

    // Insert Notification Pane
    alpsp[3]->pszText       = String_Duplicate(String_LoadString(IDS_STATUSBARPANE_INSERT));

    // Read-Only Notification Pane
    alpsp[4]->pszText       = String_Duplicate(String_LoadString(IDS_STATUSBARPANE_READONLY));
    

    INITSTRUCT(sbcd, FALSE);
    sbcd.bUseMenuFont   = TRUE;
    sbcd.hInstance      = g_hInstance;
    sbcd.hwndParent     = hwndParent;
    sbcd.uID            = IDC_STATUSBAR_MAIN;
    sbcd.nPanes         = 5;
    sbcd.alpsp          = alpsp;

    hwndStatusbar = Statusbar_Create(&sbcd);

    Mem_Free(alpsp[0]->pszText);
    Mem_Free(alpsp[2]->pszText);
    Mem_Free(alpsp[3]->pszText);
    Mem_Free(alpsp[4]->pszText);

    for (i = 0; i < 5; i++)
        Mem_Free(alpsp[i]);

    Mem_Free(alpsp);

    return (hwndStatusbar);
}

HWND MyStatusbar_Destroy(HWND hwndStatusbar)
{
    if (DestroyWindow(hwndStatusbar))
        return (NULL);
    else
        return (hwndStatusbar);
}

void MyStatusbar_Ready(HWND hwnd)
{
    STATUSBARPANE sp;

    if (!IsWindow(hwnd))
        return;

    INITSTRUCT(sp, FALSE);
    sp.fMask    = SF_TEXT | SF_CRBG | SF_CRFG;
    sp.pszText  = String_LoadString(IDS_STATUSBARPANE_READY);
    sp.crBg     = SB_DEFAULT_CRBG;
    sp.crFg     = SB_DEFAULT_CRFG;
    sp.nPane    = 0;

    Statusbar_SetPane(hwnd, &sp);
}
