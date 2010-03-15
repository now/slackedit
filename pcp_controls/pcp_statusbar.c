/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_statusbar.c
 * Created    : 01/23/00
 * Owner      : pcppopper
 * Revised on : 07/16/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* windows */
#include <commctrl.h>

/* pcp_controls */
#include "pcp_statusbar.h"

/* pcp_generic */
#include <pcp_mem.h>
#include <pcp_rect.h>
#include <pcp_string.h>

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

static void Statusbar_SetPaneWidth_Int(HWND hwnd, int nPane, int nWidth);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

HWND Statusbar_Create(LPSTATUSBARCREATEDATA lpsbcd)
{
    HWND hwndStatusbar;

    hwndStatusbar = CreateWindowEx(0, STATUSCLASSNAME, NULL,
                            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN |
                            WS_CLIPSIBLINGS | SBT_NOBORDERS | SBT_TOOLTIPS |
                            0x0000004E, //&(~SBARS_SIZEGRIP),
                            0, 0, 0, 0,
                            lpsbcd->hwndParent, (HMENU)lpsbcd->uID, lpsbcd->hInstance, NULL);

    ASSERT(hwndStatusbar != NULL);

    if (lpsbcd->bUseMenuFont)
    {
        LOGFONT lf;
        HFONT hFont;
        NONCLIENTMETRICS nm;

        INITSTRUCT(nm, TRUE);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0); 

        INITSTRUCT(lf, FALSE);
        lf = nm.lfMenuFont;

        hFont = CreateFontIndirect(&lf);

        SendMessage(hwndStatusbar, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        Statusbar_SetMinHeight(hwndStatusbar, lf.lfHeight);
        SendMessage(hwndStatusbar, WM_SIZE, 0, 0L);
    }

    if (lpsbcd->nPanes > 0)
    {
        int i;

        for (i = 0; i < lpsbcd->nPanes; i++)
            Statusbar_AddPane(hwndStatusbar, lpsbcd->alpsp[i]);
    }

    return (hwndStatusbar);
}

void Statusbar_AddPane(HWND hwnd, LPSTATUSBARPANE lpsp)
{
    LPSTATUSBARPANE lpsps;
    int *anPanes;
    int nCount;
    int nSillyModifier = 0;

    if (!IsWindow(hwnd))
        return;

    ASSERT(lpsp->fMask == SF_ALL);

    lpsps = (LPSTATUSBARPANE)Mem_Alloc(sizeof(STATUSBARPANE));
    lpsps->crBg = lpsp->crBg;
    lpsps->crFg = lpsp->crFg;
    ASSERT(lpsp->pszText != NULL);
    lpsps->pszText = String_Duplicate(lpsp->pszText);
    if (lpsp->pszTooltip != NULL)
        lpsps->pszTooltip = String_Duplicate(lpsp->pszTooltip);
    lpsps->uDrawStyle = lpsp->uDrawStyle;
    lpsps->uOwnStyle = lpsp->uOwnStyle;
    lpsps->uStyle = lpsp->uStyle;

    if (lpsps->uOwnStyle & SFS_AUTOSIZE)
    {
        SIZE size;
        HDC hdc;

        hdc = GetDC(hwnd);
        GetTextExtentPoint(hdc, lpsps->pszText, _tcslen(lpsps->pszText), &size);
        ReleaseDC(hwnd, hdc);

        lpsps->nWidth = size.cx + 4;
    }
    else
    {
        lpsps->nWidth = lpsp->nWidth;
    }

    nCount = Statusbar_GetParts(hwnd, 0, NULL);
    if (nCount == 1 && ((LPSTATUSBARPANE)Statusbar_GetText(hwnd, nCount - 1, NULL) == NULL))
        nSillyModifier = 1;
    anPanes = (int *)Mem_Alloc(sizeof(int) * (nCount + 1 - nSillyModifier));

    Statusbar_GetParts(hwnd, nCount, anPanes);
    anPanes[nCount - nSillyModifier] = (((nCount - nSillyModifier) > 0) ? anPanes[nCount - 1] : 0) + lpsps->nWidth;
    Statusbar_SetParts(hwnd, nCount + 1 - nSillyModifier, anPanes);
    Statusbar_SetText(hwnd, nCount - nSillyModifier, lpsps->uStyle | SBT_OWNERDRAW, lpsps);
    if (lpsps->pszTooltip != NULL)
        Statusbar_SetTipText(hwnd, nCount - nSillyModifier, lpsps->pszTooltip);

    Mem_Free(anPanes);
}

void Statusbar_SetPane(HWND hwnd, LPSTATUSBARPANE lpsp)
{
    LPSTATUSBARPANE lpspPane = NULL;

    if (!IsWindow(hwnd))
        return;

    ASSERT(lpsp->nPane < (int)Statusbar_GetParts(hwnd, 0, NULL));

    lpspPane = (LPSTATUSBARPANE)Statusbar_GetText(hwnd, lpsp->nPane, NULL);

    ASSERT(lpspPane != NULL);

    if (lpspPane == NULL)
        return;

    if (lpsp->fMask & SF_TEXT)
    {
        if (lpspPane->pszText != NULL)
            Mem_Free(lpspPane->pszText);
        lpspPane->pszText = Mem_AllocStr(_tcslen(lpsp->pszText));
        _tcscpy(lpspPane->pszText, lpsp->pszText);
    }
    if (lpsp->fMask & SF_TOOLTIP)
    {
        if (lpspPane->pszTooltip != NULL)
            Mem_Free(lpspPane->pszTooltip);
        lpspPane->pszTooltip = String_Duplicate(lpsp->pszTooltip);
    }
    if (lpsp->fMask & SF_CRBG)
        lpspPane->crBg          = lpsp->crBg;
    if (lpsp->fMask & SF_CRFG)
        lpspPane->crFg          = lpsp->crFg;
    if (lpsp->fMask & SF_OWNSTYLE)
        lpspPane->uOwnStyle     = lpsp->uOwnStyle;
    if (lpsp->fMask & SF_STYLE)
        lpspPane->uStyle        = lpsp->uStyle;
    if (lpsp->fMask & SF_DSTYLE)
        lpspPane->uDrawStyle    = lpsp->uDrawStyle;
    if (lpsp->fMask & SF_WIDTH)
    {
        lpspPane->nWidth = lpsp->nWidth;
        Statusbar_SetPaneWidth_Int(hwnd, lpsp->nPane, lpspPane->nWidth);
    }

    if (lpspPane->uOwnStyle & SFS_AUTOSIZE && lpsp->fMask & SF_TEXT)
    {
        SIZE size;
        HDC hdc;

        hdc = GetDC(hwnd);
        GetTextExtentPoint(hdc, lpspPane->pszText, _tcslen(lpspPane->pszText), &size);
        ReleaseDC(hwnd, hdc);

        lpspPane->nWidth = size.cx + 4;

        Statusbar_SetPaneWidth_Int(hwnd, lpsp->nPane, lpspPane->nWidth);
    }

    Statusbar_SetText(hwnd, lpsp->nPane, lpspPane->uStyle | SBT_OWNERDRAW, (LPTSTR)lpspPane);

    // Kinda dumb but...why not do it this way...will only cost some mem =)
    // Trim this shit some other time... ( the tooltip stuff i mean)
    if (lpsp->fMask & SF_TOOLTIP)
        Statusbar_SetTipText(hwnd, lpsp->nPane, lpspPane->pszTooltip);
}

void Statusbar_SetPaneText(HWND hwnd, int nPane, LPTSTR pszText)
{
    STATUSBARPANE sp;

    if (!IsWindow(hwnd))
        return;

    ASSERT(nPane < (int)Statusbar_GetParts(hwnd, 0, NULL));
    ASSERT(_tcslen(pszText) < SB_MAX_TEXT);

    INITSTRUCT(sp, FALSE);
    sp.fMask    = SF_TEXT | SF_CRBG | SF_CRFG;
    sp.crFg     = SB_DEFAULT_CRFG;
    sp.crBg     = SB_DEFAULT_CRBG;
    sp.pszText  = pszText;
    sp.nPane    = nPane;

    Statusbar_SetPane(hwnd, &sp);
}

BOOL Statusbar_GetPane(HWND hwnd, LPSTATUSBARPANE lpsp)
{
    LPSTATUSBARPANE lpspGet = NULL;

    if (!IsWindow(hwnd))
        return (FALSE);

    ASSERT(lpsp->nPane < (int)Statusbar_GetParts(hwnd, 0, NULL));

    lpspGet = (LPSTATUSBARPANE)Statusbar_GetText(hwnd, lpsp->nPane, NULL);

    if (lpsp->fMask & SF_TEXT)
        _tcscpy(lpsp->pszText, lpspGet->pszText);
    if (lpsp->fMask & SF_CRBG)
        lpsp->crBg = lpspGet->crBg;
    if (lpsp->fMask & SF_CRFG)
        lpsp->crFg = lpspGet->crFg;
    if (lpsp->fMask & SF_OWNSTYLE)
        lpsp->uOwnStyle = lpspGet->uOwnStyle;
    if (lpsp->fMask & SF_STYLE)
        lpsp->uStyle = lpspGet->uStyle;
    if (lpsp->fMask & SF_DSTYLE)
        lpsp->uDrawStyle = lpspGet->uDrawStyle;
    if (lpsp->fMask & SF_TOOLTIP)
        _tcscpy(lpsp->pszTooltip, lpspGet->pszTooltip);

    return (TRUE);
}

void Statusbar_Draw(const DRAWITEMSTRUCT *lpDrawItem)
{
    INT aBorders[3];
    RECT rc                 = lpDrawItem->rcItem;
    LPSTATUSBARPANE lpsp    = (LPSTATUSBARPANE)lpDrawItem->itemData;
    LOGBRUSH lb             = { BS_SOLID, lpsp->crBg, 0 };
    LOGPEN lp               = { PS_SOLID, 1, 1, lpsp->crBg };
    HDC hdcItem             = lpDrawItem->hDC;
    HBRUSH hBrushOld;
    HPEN hPenOld;
    int bkOld;
    COLORREF crTextOld;

    if (lb.lbColor == SB_DEFAULT_CRBG)
    {
        lb.lbColor = GetSysColor(COLOR_BTNFACE);
        lp.lopnColor = GetSysColor(COLOR_BTNFACE);
    }

    Statusbar_GetBorders(lpDrawItem->hwndItem, aBorders);

    hBrushOld   = SelectObject(hdcItem, CreateBrushIndirect(&lb));
    hPenOld     = SelectObject(hdcItem, CreatePenIndirect(&lp));

    bkOld       = SetBkMode(hdcItem, TRANSPARENT);

    if (lpsp->crFg == SB_DEFAULT_CRFG)
        crTextOld   = SetTextColor(hdcItem, GetSysColor(COLOR_BTNTEXT));
    else
        crTextOld   = SetTextColor(hdcItem, lpsp->crFg);

    Rectangle(hdcItem, rc.left, rc.top, rc.right, rc.bottom);

    if (lpsp->uOwnStyle & SFS_BORDER)
    {
        rc.left     += aBorders[1];
        rc.right    -= aBorders[1];
    }

    DrawText(hdcItem, lpsp->pszText, _tcslen(lpsp->pszText), &rc, lpsp->uDrawStyle);

    SetTextColor(hdcItem, crTextOld);
    SetBkMode(hdcItem, bkOld);

    DeleteObject(SelectObject(hdcItem, hBrushOld));
    DeleteObject(SelectObject(hdcItem, hPenOld));
}

static void Statusbar_SetPaneWidth_Int(HWND hwnd, int nPane, int nWidth)
{
    int i;
    int *anPanes;
    int nCount;
    int nDifference;

    if (!IsWindow(hwnd))
        return;

    ASSERT(nPane < (int)Statusbar_GetParts(hwnd, 0, NULL));

    nCount = Statusbar_GetParts(hwnd, 0, NULL);
    anPanes = (int *)Mem_Alloc(sizeof(int) * nCount);

    Statusbar_GetParts(hwnd, nCount, anPanes);

    nDifference = nWidth - (anPanes[nPane] - anPanes[nPane - 1]);

    for (i = (nPane - 1); i >= 0; i--)
        anPanes[i] -= nDifference;

    Statusbar_SetParts(hwnd, nCount, anPanes);

    Mem_Free(anPanes);
}

void Statusbar_OnSize(HWND hwnd, int nSizeType)
{
    RECT rc;
    int i;
    int *anPanes;
    int nCount;
    int nWidthLeft;
    LPSTATUSBARPANE lpsp;

    if (!IsWindow(hwnd))
        return;

    GetWindowRect(hwnd, &rc);
    nWidthLeft = Rect_Width(&rc) - ((nSizeType != SIZE_MAXIMIZED) ? 16 : 0);

    nCount = Statusbar_GetParts(hwnd, 0, NULL);
    anPanes = (int *)Mem_Alloc(sizeof(int) * nCount);

    i = nCount - 1;

    anPanes[i] = nWidthLeft;

    for (i = nCount - 1; i > 0; i--)
    {
        lpsp = (LPSTATUSBARPANE)Statusbar_GetText(hwnd, i, NULL);

        anPanes[i - 1] = nWidthLeft -= lpsp->nWidth;
    }

    anPanes[0] = nWidthLeft;

    Statusbar_SetParts(hwnd, nCount, anPanes);

    Mem_Free(anPanes);
}

void Statusbar_Destroy(HWND hwnd)
{
    HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
    int i;
    int nCount = Statusbar_GetParts(hwnd, 0, NULL);

    if (hFont != NULL)
        DeleteObject(hFont);

    for (i = 0; i < nCount; i++)
    {
        LPSTATUSBARPANE lpsp;
        
        lpsp = (LPSTATUSBARPANE)Statusbar_GetText(hwnd, i, NULL);

        if (Mem_Validate(lpsp))
        {
            if (Mem_Validate(lpsp->pszText))
                Mem_Free(lpsp->pszText);
            if (Mem_Validate(lpsp->pszTooltip))
                Mem_Free(lpsp->pszTooltip);

            Mem_Free(lpsp);
        }
    }
}
