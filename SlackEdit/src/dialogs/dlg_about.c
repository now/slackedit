/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : about.c
 * Created    : not known (before 01/26/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:36:00
 * Comments   : 
 *              
 *              
 *****************************************************************/

/*****************************************************************
 * Includes
 *****************************************************************/

/* pcp_generic */
#include "pcp_includes.h"
#include "pcp_definitions.h"

/* resources */
#include <slack_resource.h>

/* SlackEdit */
#include "../slack_main.h"
#include "dlg_about.h"
#include "../settings/settings.h"

/* pcp_generic */
#include "pcp_window.h"
#include "pcp_mem.h"
#include "pcp_rect.h"
#include "pcp_string.h"

/* pcp_controls */
#include "pcp_urlctrl.h"
#include "pcp_about_fade.h"

/*****************************************************************
 * Type Definitions
 *****************************************************************/

/* only use version_info.h if building a release */
#ifdef _DEBUG
#define BUILDCOUNTSTR _T("Debug")
#define BUILDDATE _T("NA")
#else /* !_DEBUG (== RELEASE) */
#include <version_info.h>
#endif /* _DEBUG */

/*****************************************************************
 * Function Definitions
 *****************************************************************/

static BOOL CALLBACK About_InfoCredz_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK About_General_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*****************************************************************
 * Global Variables
 *****************************************************************/

/*
 * s_hwndDlgAbout [internal]
 *
 * Handle of the about dialog. Used to save a handle to the
 * about dialog for Dialog_About_GetWindow()
 * and Dialog_About_Destroy()
 */
static HWND s_hwndDlgAbout = NULL;

/*****************************************************************
 * Function Implementations
 *****************************************************************/

/*
 * Dialog_About_Create() [external]
 *
 * Called to create the about dialog. Returns the handle to the
 * dialog created. This handle should only be used to check for
 * errors during creation. Call Dialog_About_GetWindow() to get
 * the handle for later use.
 */
HWND Dialog_About_Create(void)
{
    PROPSHEETPAGE psp[2];
    PROPSHEETHEADER psh;
    int i;

    for (i = 0; i < DIMOF(sps); i++)
    {
        INITSTRUCT(psp[i], TRUE);
        psp[i].hInstance    = g_hInstance;
    }

    psp[0].pszTemplate  = MAKEINTRESOURCE(IDD_ABOUT_GENERAL);
    psp[0].pfnDlgProc   = About_General_DlgProc;

    psp[1].pszTemplate  = MAKEINTRESOURCE(IDD_ABOUT_INFOCREDZ);
    psp[1].pfnDlgProc   = About_InfoCredz_DlgProc;

    INITSTRUCT(psh, TRUE);
    psh.dwFlags         = PSH_PROPSHEETPAGE | PSH_MODELESS | PSH_NOAPPLYNOW;
    psh.hwndParent      = Main_GetWindow();
    psh.hInstance       = Main_GetInstance();
    psh.pszCaption      = String_LoadString(IDS_TITLE_ABOUTDIALOG);
    psh.nPages          = DIMOF(sps);
    psh.ppsp            = (LPCPROPSHEETPAGE)&psp;

    s_hwndDlgAbout = (HWND)PropertySheet(&psh);

    return (s_hwndDlgAbout);
}

/*
 * Dialog_About_GetWindow() [external]
 *
 * Called to retrieve the handle to the about dialog. Returns
 * NULL if the about dialog doesn't exist.
 */
__inline HWND Dialog_About_GetWindow(void)
{
    return (s_hwndDlgAbout);
}

/*
 * Dialog_About_Destroy() [external]
 *
 * Called to destroy the about dialog. Returns TRUE if successful,
 * FALSE otherwise. Sets s_hwndDlgAbout to NULL if dialog destroyed.
 */
BOOL Dialog_About_Destroy(void)
{
    if (DestroyWindow(s_hwndDlgAbout))
    {
        s_hwndDlgAbout = NULL;

        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

/*
 * About_General_DlgProc() [internal]
 *
 * The DlgProc of the General about tab. Called by windows.
 */
static BOOL CALLBACK About_General_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        HWND hwndEmail;
        HWND hwndHomepage;
        HWND hwndSmartFTP;

        /*
         * could have used only one hwnd (hwndUrl or similar)
         * but this makes the code a bit clearer.
         */
        hwndEmail       = GetDlgItem(hwndDlg, IDC_URL_EMAIL);
        hwndHomepage    = GetDlgItem(hwndDlg, IDC_URL_HOMEPAGE);
        hwndSmartFTP    = GetDlgItem(hwndDlg, IDC_URL_WALTER);

        /* e-mail address (da.box@home.se) */
        UrlCtrl_SetUrl(hwndEmail, _T("mailto:da.box@home.se"));
        UrlCtrl_SetDescription(hwndEmail, _T("pcppoppers e-mail address"));
        UrlCtrl_SetBitmap(hwndEmail, UB_DEFAULTMAIL);

        /* Homepage (slackedit.sourceforge.net */
        UrlCtrl_SetUrl(hwndHomepage, _T("http://slackedit.sourceforge.net"));
        UrlCtrl_SetDescription(hwndHomepage, _T("SlackEdit's Homepage"));

        /* Mike Walter (WaLtI) info. */
        UrlCtrl_SetUrl(hwndSmartFTP, _T("http://www.smartftp.com"));
        UrlCtrl_SetDescription(hwndSmartFTP, _T("SmartFTP's Homepage (Mike Walter)"));

        Window_SetText(GetDlgItem(hwndDlg, IDC_EDIT_VERSION), _T("1.0b4"));
        Window_SetText(GetDlgItem(hwndDlg, IDC_EDIT_BUILDCOUNT), (BUILDCOUNTSTR _T(" / ") BUILDDATE));

        /* adjust the dialogs position */
        if (g_WindowSettings.bAboutCentered)
            Window_CenterWindow(GetParent(hwndDlg));
        else
            SetWindowPos(GetParent(hwndDlg), NULL,
                    g_WindowSettings.nAboutXPos, g_WindowSettings.nAboutYPos,
                    0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
    return (TRUE);
    case WM_NOTIFY:
        /* the following is used by the propsheet control */
        switch (((LPNMHDR)lParam)->code) 
        {
        case PSN_SETACTIVE:
            SetWindowLong(hwndDlg, DWL_MSGRESULT, 0);
        return (TRUE);
        case PSN_KILLACTIVE:
            SetWindowLong(hwndDlg, DWL_MSGRESULT, FALSE);
        return (TRUE);
        }
    break;
    }

    return (FALSE);
}

/*
 * About_InfoCredz_DlgProc() [internal]
 *
 * The DlgProc of the Info And Credits about tab. Called by windows.
 */
static BOOL CALLBACK About_InfoCredz_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        LPTSTR  pszTextResource;
        LPTSTR  pszPage;
        FADECTRLPAGE fcp;
        HGLOBAL hResource;
        HWND    hwndFader;

        hwndFader = GetDlgItem(hwndDlg, IDC_FADER_ABOUT);

        /* load the ascii picture resource */
        hResource = LoadResource(g_hInstance, 
                        FindResource(g_hInstance,
                            MAKEINTRESOURCE(IDR_ABOUTASCII), _T("ASCIIPIC")));
        pszTextResource = (LPTSTR)LockResource(hResource);

        INITSTRUCT(fcp, FALSE);
        fcp.crBackground        = RGB(0, 0, 0);
        fcp.crFadeTo            = RGB(40, 125, 205);
        fcp.nFadeInWaitTime     = -1;
        fcp.nFadeOutWaitTime    = -1;
        fcp.nFadePercent        = -1;
        fcp.nPercentageDelta    = -1;

        /* the ascii pics pages are split by the  character */
        for (pszPage = _tcstok(pszTextResource, _T(""));
            pszPage != NULL;
            pszPage = _tcstok(NULL, _T("")))
        {
                fcp.pszPageText = pszPage;
                FadeCtrl_AddPage(hwndFader, &fcp);
        }

        /* clean up */
        UnlockResource(hResource);
        FreeResource(hResource);

        FadeCtrl_Activate(hwndFader);
    }
    return (TRUE);
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) 
        {
        case PSN_SETACTIVE:
            FadeCtrl_Activate(hwndFader);
            SetWindowLong(hwndDlg, DWL_MSGRESULT, 0);
        return (TRUE);
        case PSN_KILLACTIVE:
            FadeCtrl_Deactivate(hwndFader);
            SetWindowLong(hwndDlg, DWL_MSGRESULT, FALSE);
        return (TRUE);
        }
    break;
    }

    return (FALSE);
}
