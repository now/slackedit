/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : font.c
 * Created    : not known
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:05:17
 * Comments   : 
 *              
 *              
 ****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* SlackEdit */
#include "../slack_main.h"
#include "settings.h"
#include "settings_font.h"

/* pcp_generic */
#include <pcp_registry.h>

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

static void Font_Save(void);
static BOOL CALLBACK Font_EnumProc(HWND hwnd, LPARAM lParam);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

static LOGFONT lf;
static HFONT hFontEdit = NULL;
static CHOOSEFONT cf;

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

BOOL Font_ChooseFont(void)
{
    INITSTRUCT(cf, TRUE);
    cf.hwndOwner      = g_hwndMain;
    cf.lpLogFont      = &lf;
    cf.Flags          = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_FIXEDPITCHONLY | CF_FORCEFONTEXIST;

    return (ChooseFont(&cf));
}

void Font_InitializeFirst()
{
    HKEY hKeyRoot, hKeySub;

    RegOpenKey(REG_ROOT, REG_SLACKEDIT, &hKeyRoot);
    RegOpenKey(hKeyRoot, _T("Font Settings"), &hKeySub);

    INITSTRUCT(lf, FALSE);
    lf.lfHeight         = Registry_GetDW(hKeySub, _T("Height"), -12);
    lf.lfWeight         = Registry_GetDW(hKeySub, _T("Weight"), FW_NORMAL);
    lf.lfCharSet        = (UCHAR)Registry_GetDW(hKeySub, _T("CharSet"), DEFAULT_CHARSET);
    lf.lfPitchAndFamily = (UCHAR)Registry_GetDW(hKeySub, _T("PitchAndFamily"), DEFAULT_PITCH);
    Registry_GetSZ(hKeySub, _T("FaceName"), lf.lfFaceName, _T("Courier"), LF_FACESIZE);

    hFontEdit = CreateFontIndirect(&lf);

    RegCloseKey(hKeySub);
    RegCloseKey(hKeyRoot);
}

void Font_Initialize(HWND hwndEdit)
{
    // Out own way of setting it (pcp_edit_View expects LPARAM to be a LPLOGFONT)
    SendMessage(hwndEdit, WM_SETFONT, 0, (LPARAM)&lf);
//  Window_SetFont(hwndEdit, TRUE, &lf);
}

void Font_SetFont(void)
{
    HFONT hFontNew;

    hFontNew = CreateFontIndirect(&lf);

    DeleteObject(hFontEdit);
    hFontEdit = hFontNew;

    EnumChildWindows(g_hwndMain, Font_EnumProc, (LPARAM)&lf);

    Font_Save();
}

HFONT Font_GetFont(void)
{
    return (hFontEdit);
}

void Font_Deinitialize(void)
{
    Font_Save();
    DeleteObject(hFontEdit);
}

static void Font_Save(void)
{
    HKEY hKeyRoot;

    RegCreateKey(REG_ROOT, (REG_SLACKEDIT _T("\\Font Settings")), &hKeyRoot);

    Registry_SetDW(hKeyRoot, _T("Height"), lf.lfHeight);
    Registry_SetDW(hKeyRoot, _T("Weight"), lf.lfWeight);
    Registry_SetDW(hKeyRoot, _T("CharSet"), lf.lfCharSet);
    Registry_SetDW(hKeyRoot, _T("PitchAndFamily"), lf.lfPitchAndFamily);
    Registry_SetSZ(hKeyRoot, _T("FaceName"), lf.lfFaceName);

    RegCloseKey(hKeyRoot);
}

static BOOL CALLBACK Font_EnumProc(HWND hwnd, LPARAM lParam)
{
    SendMessage(hwnd, WM_MYFONTCHANGE, 0, lParam);

    return (TRUE);
}
