/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : main.c
 * Created    : not known
 * Owner      : pcppopper
 * Revised on : 07/02/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>
#include <pcp_definitions.h>

/* windows */
#include <time.h>
#include <commctrl.h>
#include <process.h>

#include <slack_resource.h>
#include <slack_picdef.h>

/* SlackEdit */
#include "slack_main.h"
#include "controls/clipboard_monitor.h"
#include "controls/pcp_hexedit.h"
#include "controls/qdcm.h"
#include "dialogs/dlg_advancedopen.h"
#include "dialogs/dlg_about.h"
#include "dialogs/dlg_favorites.h"
#include "dialogs/dlg_commandline.h"
#include "mdi/mdi_child_edit.h"
#include "mdi/mdi_child_hex.h"
#include "mdi/mdi_client.h"
#include "mdi/child_tclcommandline.h"
#include "parsing/file_actions.h"
#include "rebar/bar_main.h"
#include "rebar/bar_toolbar.h"
#include "rebar/bar_findcombo.h"
#include "rebar/bar_gotoline.h"
#include "settings/settings_font.h"
#include "settings/settings.h"
#include "subclasses/sub_pcp_edit.h"
#include "tcl/tcl_main.h"
#include "tcl/tcl_hotkey.h"
#include "tcl/tcl_load.h"
#include "tcl/tcl_shell.h"
#include "windows/wnd_statusbar.h"
#include "windows/wnd_windowbar.h"
#include "windows/wnd_clipbook.h"
#include "windows/wnd_output.h"

/* pcp_generic */
#include <pcp_combobox.h>
#include <pcp_mdi.h>
#include <pcp_mem.h>
#include <pcp_mru.h>
#include <pcp_path.h>
#include <pcp_rect.h>
#include <pcp_string.h>
#include <pcp_systray.h>
#include <pcp_timer.h>
#include <pcp_toolbar.h>
#include <pcp_window.h>

/* pcp_paint */
#include <pcp_paint.h>

/* pcp_controls */
#include <pcp_about_fade.h>
#include <pcp_basebar.h>
#include <pcp_menu.h>
#include <pcp_statusbar.h>
#include <pcp_tip_of_da_day.h>
#include <pcp_urlctrl.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define  SET_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define  CLEAR_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#else
#define  SET_CRT_DEBUG_FIELD(a)   ((void)0)
#define  CLEAR_CRT_DEBUG_FIELD(a) ((void)0)
#endif /* _DEBUG */

/****************************************************************
 * Type Definitions
 ****************************************************************/

/****************************************************************
 * Function Definitions
 ****************************************************************/

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
static BOOL Main_InitApplication(HINSTANCE hInstance);
static BOOL Main_InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nCmdShow);

static BOOL Main_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void Main_OnDestroy(HWND hwnd);
static void Main_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT uNotifyCode);
static void Main_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);
static void Main_OnClose(HWND hwnd);
static BOOL Main_OnQueryEndSession(HWND hwnd);
static void Main_OnInitMenuPopup(HWND hwnd, HMENU hMenuPopup, UINT uPos, BOOL fSystemMenu);
static void Main_OnChangeCBChain(HWND hwnd, HWND hwndRemove, HWND hwndNext);
static void Main_OnDrawClipboard(HWND hwnd);
static void Main_OnClose(HWND hwnd);
static BOOL Main_OnQueryEndSession(HWND hwnd);
static void Main_OnSize(HWND hwnd, UINT state, int cx, int cy);
static LRESULT Main_OnNotify(HWND hwnd, int id, LPNMHDR pnmh);
static void Main_OnDropFiles(HWND hwnd, HDROP hDrop);
static void Main_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpDrawItem);
static void Main_OnMenuSelect(HWND hwnd, HMENU hMenu, int item, HMENU hMenuPopup, UINT flags);
static void Main_OnCmdLine(HWND hwnd, LPTSTR lpCmdLine);
static void Main_OnSysCommand(HWND hwnd, UINT cmd, int x, int y);
static void Main_OnTrayNotify(HWND hwnd, UINT idCtl, UINT uNotifyCode);
static void Main_OnWindowPosChanged(HWND hwnd, const LPWINDOWPOS lpwpos);
static BOOL Main_OnCopyData(HWND hwnd, HWND hwndSender, PCOPYDATASTRUCT pcds);

static LRESULT CALLBACK Main_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef _DEBUG
int _cdecl Main_CrtAllocHook(int nAllocType, void *pvData, size_t nSize, int nBlockUse, long lRequest, const unsigned char *szFileName, int nLine);
#endif /* _DEBUG */

static DWORD WINAPI Main_TclLoadThread(LPVOID lpParameter);
static BOOL Main_CreateChildWindows(HWND hwnd);
void Main_WriteLogEntry(LPTSTR pszLogEntry);
static BOOL Main_ProcessCommandLine(HWND hwndMain, LPTSTR pszCommandLine);
void Main_FocusNext(BOOL bNext);

unsigned long Main_HandleException(HWND hwnd, LPMSG lpmsg, EXCEPTION_POINTERS *pExceptionInfo);

static void Main_CreateDirectories(void);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

const long magicDWord = 0x49474541;

#ifdef _DEBUG
const TCHAR g_szAppName[] = _T("SlackEdit");
#else
#include "../SlackEdit/version_info.h"
const TCHAR g_szAppName[] = (_T("SlackEdit ") BUILDCOUNTSTR);
#endif /* _DEBUG */

LPCTSTR g_pszEditChildClass = _T("Text0rChildClass");
LPCSTR g_pszHexChild = _T("Hex0rChildClass");

static HWND s_hwndNextViewer;

/****************************************************************
 * Function Implementation                                      *
 ****************************************************************/

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    DWORD dwTime, dwInit, dwLibs;
    MSG msg;
    HANDLE  hMutex;
    HANDLE  hTclThread;
    TCHAR szTimings[128];
    DWORD dwThreadID;
    PTIMER pTimerTotal = Timer_Create();
    PTIMER pTimer;

    g_hwndMain      = NULL;
    g_hwndMDIClient = NULL;

    msg.wParam = 0;

    // only in debug (check marco)
    SET_CRT_DEBUG_FIELD(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF);
#ifdef _DEBUG
    _CrtSetAllocHook(Main_CrtAllocHook);
#endif /* _DEBUG */

    hMutex = CreateMutex(NULL, TRUE, String_LoadString(IDS_MUTEXNAME));

    if ((hMutex != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS) &&
        !g_GlobalSettings.bMultipleInstances)
    {
        HWND hwndPrevInst = FindWindow(g_szAppName, NULL);
        COPYDATASTRUCT cds;

        cds.cbData = _tcslen(lpCmdLine) + SZ;
        cds.lpData = lpCmdLine;
        Window_CopyData(hwndPrevInst, NULL, &cds);

        if (IsMinimized(hwndPrevInst))
            ShowWindow(hwndPrevInst, SW_SHOWNORMAL);

        SetForegroundWindow(hwndPrevInst);
        BringWindowToTop(hwndPrevInst);

        CloseHandle(hMutex);

        return (0); // -1
    }

    g_hInstance = hInstance;
    g_hMainMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MAINMENU));

    String_SetResourceHandle(g_hInstance);
    Main_CreateDirectories();
    Config_Read();

    hTclThread = BEGINTHREADEX(NULL, 0, Main_TclLoadThread, NULL, 0, &dwThreadID);

    pTimer = Timer_Create();

    if (!Main_InitApplication(hInstance))
    {
        MessageBox(NULL, String_LoadString(IDS_ERROR_REGISTERWINDOW), g_szAppName, MB_OK);
        
        return (FALSE);
    }

    Timer_StopAndCalculate(pTimer, &dwLibs);
    Timer_Start(pTimer);

    if (!Main_InitInstance(hInstance, lpCmdLine, nCmdShow)) 
    {
        MessageBox(NULL, String_LoadString(IDS_ERROR_CREATEWINDOW), g_szAppName, MB_OK);

        return (FALSE);
    }

    Timer_StopAndCalculate(pTimer, &dwInit);
    Timer_StopAndCalculate(pTimerTotal, &dwTime);

    _stprintf(szTimings, String_LoadString(IDS_FORMAT_LOADTIMES), dwLibs, dwInit, dwTime);

    Statusbar_SetPaneText(g_hwndStatusbarMain, 0, szTimings);
    Main_WriteLogEntry(szTimings);

    if (g_GlobalSettings.bTipOfDaDay)
        g_GlobalSettings.bTipOfDaDay = TipOfDay_CreateDialogBox(g_hwndMain, g_GlobalSettings.bTipOfDaDay);

    WaitForSingleObject(hTclThread, INFINITE);
    CloseHandle(hTclThread);

    while (TRUE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            else
            {

#ifndef _DEBUG
                __try
                {
#endif /* _DEBUG */
                    if ((g_hwndDlgGotoline == NULL || !IsDialogMessage(g_hwndDlgGotoline, &msg)) &&
                    (g_hwndDlgAbout == NULL || !PropSheet_IsDialogMessage(g_hwndDlgAbout, &msg)) &&
                    !TranslateMDISysAccel(g_hwndMDIClient, &msg) &&
                    !MyTcl_TranslateAccelerator(g_hwndMain, &msg))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
#ifndef _DEBUG
                }
                __except (Main_HandleException(g_hwndMain, &msg, GetExceptionInformation()))
                {
                    continue;
                }
#endif /* _DEBUG */
            }
        }
        else if (g_hwndDlgAbout != NULL && PropSheet_GetCurrentPageHwnd(g_hwndDlgAbout) == NULL)
        {
            DestroyWindow(g_hwndDlgAbout);
            g_hwndDlgAbout = NULL;
        }
        else
        {
            WaitMessage();
        }
    }

    CloseHandle(hMutex);

    return (msg.wParam);
}

static BOOL Main_InitApplication(HINSTANCE hInstance)
{
    WNDCLASS wc;
    INITCOMMONCONTROLSEX icex;

    INITSTRUCT(icex, TRUE);
    icex.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_DATE_CLASSES |
            ICC_HOTKEY_CLASS | ICC_LISTVIEW_CLASSES | ICC_PAGESCROLLER_CLASS |
            ICC_PROGRESS_CLASS | ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES |
            ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES;

    InitCommonControlsEx(&icex);

    QDCM_Install(TRUE, FALSE, (DWORD)-1);
    Menu_Install(Toolbar_GetIcon, Toolbar_GetSysMenuIcon, Toolbar_GetOtherIcon); //To set up the hook b4 the main window is created
    Menu_AddImage(LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_TOOLBAR)), RGB(255, 0, 255));
    Menu_AddImage(LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SYSMENU)), RGB(255, 0, 255));
    Toolbar_InitDefButtons();
    Toolbar_InitAllButtons();
    MRU_Initialize();
    MRU_SetRoot(HKEY_CURRENT_USER, (REG_SLACKEDIT _T("\\MRU")));
    TipOfDay_SetTipFileName(_T("System\\SlackEdit.tip"));

    // Register the main class

    INITSTRUCT(wc, FALSE);
    wc.style            = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc      = Main_WndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_INACTIVEBORDER + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = g_szAppName;

    if (!RegisterClass(&wc))
        return (FALSE);

    // Register the edit child class

    wc.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDIT));
    wc.lpfnWndProc      = MDI_Child_Edit_WndProc;
    wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = g_szEditChild;

    if (!RegisterClass(&wc))
        return (FALSE);

    // Register the hex child class

    wc.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDIT));
    wc.lpfnWndProc      = MDI_Child_Hex_WndProc;
    wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = g_szHexChild;

    if (!RegisterClass(&wc))
        return (FALSE);

    // Register misc. controls

    if (!BaseBar_RegisterControl(hInstance))
        return (FALSE);

    if (!UrlCtrl_RegisterControl(hInstance))
        return (FALSE);

    if (!FadeCtrl_RegisterControl(hInstance))
        return (FALSE);

    if (!HexEdit_RegisterControl(hInstance))
        return (FALSE);

    // Hooray! we passed all the class registrations.

    return (TRUE);
}

static BOOL Main_InitInstance(HINSTANCE hInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    g_hwndMain = CreateWindowEx(WS_EX_ACCEPTFILES | WS_EX_WINDOWEDGE,
                            g_szAppName, g_szAppName,
                            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |
                            WS_OVERLAPPED | WS_CLIPSIBLINGS,
                            0, 0, 0, 0,
                            NULL, g_hMainMenu, hInstance, NULL);

    if (!g_hwndMain)
        return (FALSE);

        //This takes 213 ms (proved inside func)
    if (!Main_CreateChildWindows(g_hwndMain))
        return (FALSE);

    File_Initialize();

    // Takes X ms
    if (!(Main_ProcessCommandLine(g_hwndMain, lpCmdLine) && g_GlobalSettings.bOnlyReloadIfCommandLineEmpty))
    {
        if (g_GlobalSettings.bSaveOpenWindows)
            Config_LoadLastOpenWindows();
    }

    //Takes 40 ms
    if (g_WindowSettings.bMainMaximized)
        ShowWindow(g_hwndMain, SW_SHOWMAXIMIZED);
    else
        ShowWindow(g_hwndMain, nCmdShow);

    // This isn't nice! But right now I see no other solution =\...
    Window_UpdateLayout(g_hwndMain);

    return (TRUE);
}

static LRESULT CALLBACK Main_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_CREATE,             Main_OnCreate);
        HANDLE_MSG(hwnd, WM_COMMAND,            Main_OnCommand);
        HANDLE_MSG(hwnd, WM_DESTROY,            Main_OnDestroy);
        HANDLE_MSG(hwnd, WM_ACTIVATE,           Main_OnActivate);
        HANDLE_MSG(hwnd, WM_INITMENUPOPUP,      Main_OnInitMenuPopup);
        HANDLE_MSG(hwnd, WM_CHANGECBCHAIN,      Main_OnChangeCBChain);
        HANDLE_MSG(hwnd, WM_DRAWCLIPBOARD,      Main_OnDrawClipboard);
        HANDLE_MSG(hwnd, WM_CLOSE,              Main_OnClose);
        HANDLE_MSG(hwnd, WM_QUERYENDSESSION,    Main_OnQueryEndSession);
        HANDLE_MSG(hwnd, WM_SIZE,               Main_OnSize);
        HANDLE_MSG(hwnd, WM_NOTIFY,             Main_OnNotify);
        HANDLE_MSG(hwnd, WM_DROPFILES,          Main_OnDropFiles);
        HANDLE_MSG(hwnd, WM_DRAWITEM,           Main_OnDrawItem);
        HANDLE_MSG(hwnd, WM_MENUSELECT,         Main_OnMenuSelect);
        HANDLE_MSG(hwnd, WM_SYSCOMMAND,         Main_OnSysCommand);
        HANDLE_MSG(hwnd, WM_TRAYNOTIFY,         Main_OnTrayNotify);
        HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGED,   Main_OnWindowPosChanged);
        HANDLE_MSG(hwnd, WM_COPYDATA,           Main_OnCopyData);
/*  case WM_EXITMENULOOP:
    {
        BOOL bExists;
        MENUITEMINFO mii;
        HMENU hMenuPopup = Main_GetMainSubMenu(g_hwndMain, IDM_MAIN_WINDOW);

        INITSTRUCT(mii, TRUE);
        mii.fMask = MIIM_TYPE;

        bExists = GetMenuItemInfo(hMenuPopup, IDM_WINDOW_WINDOWLIST, FALSE, &mii);

        if (bExists)
        {
            DeleteMenu(hMenuPopup, IDM_WINDOW_WINDOWLIST, MF_BYCOMMAND);
            DeleteMenu(hMenuPopup, IDM_WINDOW_WINDOWLIST_SEPARATOR, MF_BYCOMMAND);
        }
    }
    break;*/
    }

    return (DefFrameProc(hwnd, g_hwndMDIClient, uMsg, wParam, lParam));
}

static BOOL Main_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    HMENU hMenu = GetMenu(hwnd), hSubMenu, hMenuPasteRecent;
    MENUITEMINFO mii;
    TCHAR szMenuText[MAX_PATH];

    Font_InitializeFirst();

//  Menubar_Create(hwnd, hMenu);

    hSubMenu            = GetSubMenu(hMenu, IDM_MAIN_FILE);

    INITSTRUCT(mii, TRUE);
    mii.fMask   = MIIM_SUBMENU;
    GetMenuItemInfo(hSubMenu, 14, TRUE, &mii);
    g_hMenuRecentFiles  = mii.hSubMenu;
    MRU_SetMenu(g_hMenuRecentFiles, _T("File MRU"), g_MRUSettings.nFileMax, IDM_RECENTFILES);

    Clip_Read(&hMenuPasteRecent);
    hSubMenu            = GetSubMenu(hMenu, IDM_MAIN_EDIT);
    InsertMenu(hSubMenu, 6, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)hMenuPasteRecent, String_LoadString(IDS_PASTERECENT));

    mii.fMask       = MIIM_TYPE;
    mii.dwTypeData  = szMenuText;
    mii.cch         = MAX_PATH;
    hSubMenu        = GetSubMenu(hMenu, IDM_MAIN_HELP);
    GetMenuItemInfo(hSubMenu, IDM_HELP_ABOUT, FALSE, &mii);
    ASSERT((mii.fType & MFT_STRING) == MFT_STRING);
    hMenu = GetSystemMenu(hwnd, FALSE);
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, IDM_HELP_ABOUT, szMenuText);

    SetWindowPos(hwnd, 0, g_WindowSettings.nMainXPos, g_WindowSettings.nMainYPos,
                g_WindowSettings.nMainWidth, g_WindowSettings.nMainHeight, 0);

    if (g_WindowSettings.bMainCentered)
        Window_CenterWindow(hwnd);

    if (g_WindowSettings.bMainSticky)
        SetWindowLong(hwnd, GWL_USERDATA, magicDWord);

    if (g_WindowSettings.bMainOnTop)
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    s_hwndNextViewer = SetClipboardViewer(hwnd);

    return (TRUE);
}

static void Main_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT uNotifyCode)
{
    switch (id)
    {
        case IDM_FILE_NEW:
        {
            TCHAR szFile_[] = _T("Edit");
            File_Open(szFile_, CRLF_STYLE_AUTOMATIC);
        }
        return;
        case IDM_FILE_OPEN:
            File_OpenByDialog(hwnd);
        return;
        case IDM_FILE_ADVANCEDOPEN:
            FastOpen(hwnd); 
        return;
        case IDM_FILE_FAVORITEOPEN:
            Favorite_Create(hwnd);
        return;
        case IDM_FILE_RECENTFILES_CLEARNONEXISTENTFILES:
        {
            int i = 0;
            TCHAR szFile[MAX_PATH] = _T("");
            CREATEMRULIST cml;
            HANDLE hList;

            INITSTRUCT(cml, TRUE);
            cml.nMaxItems   = g_MRUSettings.nFileMax;
            cml.dwFlags     = MRUF_STRING_LIST;
            cml.hKey        = MRU_GetRoot();
            cml.lpszSubKey  = _T("File MRU");
            cml.lpfnCompare = NULL;

            hList = CreateMRUList(&cml);

            while (EnumMRUList(hList, i, szFile, MAX_PATH) != -1)
            {
                if (!Path_FileExists(szFile))
                {
                    DelMRUString(hList, i);

                    continue;
                }

                i++;
            }

            FreeMRUList(hList);

            for (i = IDM_RECENTFILES; DeleteMenu(g_hMenuRecentFiles, i, MF_BYCOMMAND); i++)
                ; /* empty body */
            MRU_SetMenu(g_hMenuRecentFiles, _T("File MRU"), g_MRUSettings.nFileMax, IDM_RECENTFILES);
        }
        return;
        case IDM_FILE_CLOSE:
            Window_Close(hwnd);
        return;
        case IDM_FILE_EXIT:
            Systray_SetInTray(TRUE);
            Window_Close(hwnd);
        return;
        case IDM_VIEW_CHILDREN_CLIPBOOK:
            Main_ToggleChildWindow(IDC_CLIPBOOK, TRUE);
        return;
        case IDM_VIEW_CHILDREN_OUTPUTWINDOW:
            Main_ToggleChildWindow(IDC_OUTPUTWINDOW, TRUE);
        return;
        case IDM_VIEW_CHILDREN_STATUSBAR:
            Main_ToggleChildWindow(IDC_STATUSBAR_MAIN, TRUE);
        return;
        case IDM_VIEW_CHILDREN_WINDOWBAR:
            Main_ToggleChildWindow(IDC_TAB_WINDOWBAR, TRUE);
        return;
        case IDM_VIEW_REBARITEMS_TOOLBAR:
            Rebar_ToggleItem(IDC_TOOLBAR_MAIN);
        return;
        case IDM_VIEW_REBARITEMS_QUICKSEARCH:
            Rebar_ToggleItem(IDC_COMBO_QUICKSEARCH);
        return;
        case IDM_VIEW_REBARITEMS_GOTOLINE:
            Rebar_ToggleItem(IDC_DLG_GOTOLINE);
        return;
        case IDM_VIEW_RUNDOSCOMMAND:
            CmdLine_Initialize(hwnd, FALSE);
        return;
        case IDM_VIEW_RUNWINSLOWSCOMMAND:
            CmdLine_Initialize(hwnd, TRUE);
        return;
        case IDM_OPTIONS_SETFONT:
            if (Font_ChooseFont())
                Font_SetFont();
        return;
        case IDM_OPTIONS_SETTINGS:
            Config_Edit(hwnd);
        return;
        case IDM_HELP_ABOUT:
            About_Initialize(hwnd);
        return;
        case IDM_HELP_TIPOFDADAY:
            g_GlobalSettings.bTipOfDaDay = TipOfDay_CreateDialogBox(hwnd, g_GlobalSettings.bTipOfDaDay);
        return;
        case IDM_TRAY_RESTORE:
            ShowWindow(hwnd, SW_SHOW);
            Systray_SetInTray(!Systray_DeleteIcon(hwnd, IDC_TRAYICON_MAIN));
        return;
        case IDM_TRAY_OPEN:
            if (File_OpenByDialog(hwnd))
                Window_Command(hwnd, IDM_TRAY_RESTORE, 0, 0);
        return;
        case IDM_TRAY_EXIT:
            Systray_SetInTray(TRUE);
            Window_Close(hwnd);
        return;
        //Advanced Accelerators
        case IDA_ACTIVATE_MDICLIENT:
            SetFocus(g_hwndMDIClient);
        return;
        case IDA_ACTIVATE_CLIPBOOK:
            if (!IsWindow(g_hwndClipbook))
                Main_ToggleChildWindow(IDC_CLIPBOOK, TRUE);

            SetFocus(g_hwndClipbook);
        return;
        case IDA_ACTIVATE_OUTPUTWINDOW:
            if (!IsWindow(g_hwndOutputWindow))
                Main_ToggleChildWindow(IDC_OUTPUTWINDOW, TRUE);

            SetFocus(g_hwndOutputWindow);
        return;
        case IDA_ACTIVATE_QUICKSEARCH:
            if (!IsWindow(g_hwndCboQuickSearch))
                Rebar_ToggleItem(IDC_COMBO_QUICKSEARCH);

            SetFocus(g_hwndCboQuickSearch);
        return;
        case IDA_ACTIVATE_NEXT:
            Main_FocusNext(TRUE);
        return;
        case IDA_ACTIVATE_PREV:
            Main_FocusNext(FALSE);
        return;
        default:
        {
            if (id >= IDM_RECENTFILES && id <= (IDM_RECENTFILES + g_MRUSettings.nFileMax))
            {
                int i = 0;
                TCHAR szFile[MAX_PATH] = _T("");
                CREATEMRULIST cml;
                HANDLE hList;

                INITSTRUCT(cml, TRUE);
                cml.nMaxItems   = g_MRUSettings.nFileMax;
                cml.dwFlags     = MRUF_STRING_LIST;
                cml.hKey        = MRU_GetRoot();
                cml.lpszSubKey  = _T("File MRU");
                cml.lpfnCompare = NULL;

                hList = CreateMRUList(&cml);

                while (EnumMRUList(hList, i, szFile, MAX_PATH) != -1)
                {
                    if (i == (id - IDM_RECENTFILES))
                        break;

                    i++;
                }

                FreeMRUList(hList);

                if (!Path_FileExists(szFile))
                {
                    TCHAR szMessage[MAX_PATH + 30];

                    _sntprintf(szMessage, MAX_PATH + 30, String_LoadString(IDS_FORMAT_ERROR_RECENT_FILE_NONEXISTENT), szFile);
                    MessageBox(g_hwndMain, szMessage, _T("Oops"), MB_OK | MB_ICONEXCLAMATION);
                
                    return;
                }
                
                File_Open(szFile, CRLF_STYLE_AUTOMATIC);

                if (Systray_IsInTray())
                {
                    ShowWindow(hwnd, SW_SHOW);
                    Systray_SetInTray(!Systray_DeleteIcon(hwnd, IDC_TRAYICON_MAIN));
                }
            }
            else if (id >= IDM_MDI_FIRSTCHILD && id <= (IDM_MDI_FIRSTCHILD + 8))
            {
            //  DefFrameProc(hwnd, g_hwndMDIClient, WM_COMMAND, MAKEWPARAM(id, uNotifyCode), (LPARAM)hwndCtl);
                MDI_Client_ActivateChild(id);
            }
            else
            {
                if (IsWindow(g_hwndMDIClient))
                    Window_Command(g_hwndMDIClient, id, uNotifyCode, hwndCtl);
            }
        }
    }
}

static LRESULT Main_OnNotify(HWND hwnd, int id, LPNMHDR pnmh)
{
    switch (pnmh->code)
    {
        case CBEN_ENDEDIT:
            switch (pnmh->idFrom)
            {
                case IDC_COMBO_QUICKSEARCH:
                    if (((PNMCBEENDEDIT)pnmh)->iWhy == CBENF_RETURN)
                        QuickSearch_EndEdit((PNMCBEENDEDIT)pnmh);
                return (0);
            }
        break;
        case TCN_SELCHANGE:
        {
            TC_ITEM tci;
            tci.mask    = TCIF_PARAM;
            tci.lParam  = 0;

            TabCtrl_GetItem(g_hwndTabWindowBar, TabCtrl_GetCurSel(g_hwndTabWindowBar), &tci);

            MDI_Activate(g_hwndMDIClient, tci.lParam);
            if (IsIconic((HWND)tci.lParam))
                Main_UpdateWindowBar((HWND)tci.lParam, SIZE_RESTORED, (LPARAM)NULL);
        }
        break;
        case TBN_GETBUTTONINFO:
        return (MyToolbar_GetButtonInfo((LPTBNOTIFY)pnmh));
        case NM_CLICK:
            switch (pnmh->idFrom)
            {
                case IDC_TAB_WINDOWBAR:
                {
                    TC_HITTESTINFO tchti;
                    int iTab;
                    TC_ITEM tci;

                    GetCursorPos(&tchti.pt);
                    ScreenToClient(g_hwndTabWindowBar, &tchti.pt);

                    iTab        = TabCtrl_HitTest(g_hwndTabWindowBar, &tchti);
                    tci.mask    = TCIF_PARAM;
                    tci.lParam  = 0;

                    TabCtrl_GetItem(g_hwndTabWindowBar, iTab, &tci);

                    if (MDI_MyGetActive(FALSE) == (HWND)tci.lParam)
                        ShowWindow((HWND)tci.lParam, SW_MINIMIZE);
                }
                break;
            }
        break;
        case NM_RCLICK:
            switch (pnmh->idFrom)
            {
                case IDC_TAB_WINDOWBAR:
                {
                    TC_HITTESTINFO tchti;
                    int iTab;
                    TC_ITEM tci;
                    HMENU hMenuMDI;

                    GetCursorPos(&tchti.pt);
                    ScreenToClient(g_hwndTabWindowBar, &tchti.pt);

                    iTab        = TabCtrl_HitTest(g_hwndTabWindowBar, &tchti);
                    tci.mask    = TCIF_PARAM;
                    tci.lParam  = 0;

                    TabCtrl_GetItem(g_hwndTabWindowBar, iTab, &tci);

                    ClientToScreen(g_hwndTabWindowBar, &tchti.pt);
                    hMenuMDI = GetSystemMenu((HWND)tci.lParam, FALSE);

                    Menu_TrackPopupMenu(hMenuMDI, tchti.pt.x, tchti.pt.y, (HWND)tci.lParam);
                }
                return (0);
            }
        break;
        case TTN_NEEDTEXT:
            if (!Windowbar_ToolTip((LPTOOLTIPTEXT)pnmh))
                Toolbar_ToolTip((LPTOOLTIPTEXT)pnmh);
        break;
        case BBN_CLOSE:
            switch (pnmh->idFrom)
            {
                case IDC_CLIPBOOK:
                    Main_ToggleChildWindow(IDC_CLIPBOOK, TRUE);
                return (0);
                case IDC_OUTPUTWINDOW:
                    Main_ToggleChildWindow(IDC_OUTPUTWINDOW, TRUE);
                return (0);
            }
        break;
/*      case BCN_GETCLIENTRECT:
        {
            LPBCNM lpbcnm = (LPBCNM)pnmh;

            switch (lpbcnm->hdr.idFrom)
            {
            case IDC_CLIPBOOK:
                GetClientRect(g_hwndClipbook, &lpbcnm->rcClient);
            return (TRUE);
            case IDC_OUTPUTWINDOW:
                GetClientRect(g_hwndOutputWindow, &lpbcnm->rcClient);
            return (TRUE);
            }
        }
        return (TRUE);
        case BCN_STYLECHANGED:
        case BCN_POSCHANGED:*/
        case RBN_HEIGHTCHANGE:
            Window_UpdateLayout(hwnd);
        break;
    }
//  FORWARD_WM_NOTIFY(hwnd, id, pnmh, );
    return (DefFrameProc(hwnd, g_hwndMDIClient, WM_NOTIFY, id, (LPARAM)pnmh));
}

static void Main_OnDropFiles(HWND hwnd, HDROP hDrop)
{
    TCHAR szDraggedFile[MAX_PATH];
    int i;

    for (i = 0; DragQueryFile(hDrop, i, szDraggedFile, MAX_PATH); i++)
        File_Open(szDraggedFile, CRLF_STYLE_AUTOMATIC);

    DragFinish(hDrop);
}

void Main_OnClose(HWND hwnd)
{
    if (g_GlobalSettings.fGotoSystray && !Systray_IsInTray())
    {
        ShowWindow(hwnd, SW_HIDE);
        Systray_SetInTray(Systray_AddIcon(hwnd, IDC_TRAYICON_MAIN,
                    LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN)), (LPTSTR)g_szAppName));
    }
    else    
    {
        if (g_GlobalSettings.bSaveOpenWindows)
            Config_SaveOpenWindows();

        Window_SetRedraw(hwnd, FALSE);
        Window_Command(hwnd, IDM_WINDOW_CLOSEALL, 0, 0);
        Window_SetRedraw(hwnd, TRUE);
        RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN);

        if (GetWindow(g_hwndMDIClient, GW_CHILD) == NULL)
            DestroyWindow(hwnd);
    }
}

static BOOL Main_OnQueryEndSession(HWND hwnd)
{
    Window_Command(hwnd, IDM_WINDOW_CLOSEALL, 0, 0);

    if (GetWindow(g_hwndMDIClient, GW_CHILD) != NULL)
        return (FALSE);
    else
        return (TRUE);
}

static void Main_OnDestroy(HWND hwnd)
{
    QDCM_Remove();
    Menu_Uninstall();
    Font_Deinitialize();

    ChangeClipboardChain(hwnd, s_hwndNextViewer);

    Clip_Save();

    File_Denitialize();

//  Band_UnRegisterControl(g_hInstance);
    UrlCtrl_UnregisterControl(g_hInstance);
    FadeCtrl_UnregisterControl(g_hInstance);
    HexEdit_UnregisterControl(g_hInstance);
    BaseBar_UnregisterControl(g_hInstance);

    Statusbar_Destroy(g_hwndStatusbarMain);

    MyTcl_FreeLibrary();

    if (Systray_IsInTray())
        Systray_SetInTray(!Systray_DeleteIcon(hwnd, IDC_TRAYICON_MAIN));

    g_GlobalSettings.bLastExitSuccessful = TRUE;
    Config_Write();

    PostQuitMessage(0);
}

static void Main_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized)
{
    if (state == WA_CLICKACTIVE)
    {
        POINT p;

        GetCursorPos(&p);
        SendMessage(MDI_MyGetActive(TRUE), WM_LBUTTONDOWN, 0, MAKELPARAM(p.x, p.y));
    }
}

static void Main_OnInitMenuPopup(HWND hwnd, HMENU hMenuPopup, UINT uPos, BOOL fSystemMenu)
{
    UINT uMyPos = uPos;

    if (!fSystemMenu)
    {
        if (MDI_MyGetActive(FALSE) != NULL &&
            IsMaximized(MDI_MyGetActive(FALSE)))
        {
            uMyPos--;
        }

        // only process if the submenu is a submenu of the main menu
        if (Main_GetMainSubMenu(hwnd, uMyPos) != hMenuPopup)
            return;

        switch (uMyPos)
        {
            case IDM_MAIN_FILE:
            break;
            case IDM_MAIN_EDIT:
                TextView_UpdateEditMenuInt(MDI_GetEditView(NULL), hMenuPopup);
            break;
            case IDM_MAIN_SEARCH:
                TextView_UpdateSearchMenuInt(MDI_GetEditView(NULL), hMenuPopup);
            break;
            case IDM_MAIN_VIEW:
            break;
            case IDM_MAIN_OPTIONS:
            break;
            case IDM_MAIN_WINDOW:
                MDI_Client_UpdateWindowMenu(hMenuPopup);
            break;
            case IDM_MAIN_HELP:
            break;
        }
    }
}

static void Main_OnChangeCBChain(HWND hwnd, HWND hwndRemove, HWND hwndNext)
{
    //keep ourselves from crashing =)
    if (hwndRemove == s_hwndNextViewer)
        s_hwndNextViewer = hwndNext;
    else if (s_hwndNextViewer != NULL)
        SendMessage(s_hwndNextViewer, WM_CHANGECBCHAIN, (WPARAM)hwndRemove, (LPARAM)hwndNext);
}

static void Main_OnDrawClipboard(HWND hwnd)
{
    MENUITEMINFO mii;
    HMENU hMenu = Main_GetMainSubMenu(hwnd, IDM_MAIN_EDIT);

    INITSTRUCT(mii, TRUE);
    mii.fMask   = MIIM_SUBMENU;
    GetMenuItemInfo(hMenu, 6, TRUE, &mii);

    Clip_Add(hwnd, &mii.hSubMenu);
    SetMenuItemInfo(hMenu, 6, TRUE, &mii);

    MyTcl_ClipboardMonitorNotify();
}

static void Main_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    DefWindowProc(hwnd, WM_SIZE, (WPARAM)state, (LPARAM)MAKELPARAM(cx, cy));

    if (state != SIZE_MINIMIZED)
    {
        HDWP hdwp;
        RECT rRebar;
        RECT rStatusbar;
        RECT rTabWindowBar;
        RECT rClipbook;
        RECT rcOutput;
        RECT rc;
        RECT rcClient;
        BOOL bRebar = FALSE;
        BOOL bStatusbar = FALSE;
        BOOL bTabWindowBar = FALSE;
        BOOL bClipbook = FALSE;
        BOOL bOutput = FALSE;
        BOOL bMDI = FALSE;
        int cWindows = 0;
        TEXTMETRIC tm;
        HDC hdcStatus = GetDC(g_hwndStatusbarMain);

        GetTextMetrics(hdcStatus, &tm);
        ReleaseDC(g_hwndStatusbarMain, hdcStatus);

        GetClientRect(hwnd, &rcClient);
        CopyRect(&rc, &rcClient);

        ZeroMemory(&rRebar, sizeof(RECT));
        ZeroMemory(&rStatusbar, sizeof(RECT));
        ZeroMemory(&rTabWindowBar, sizeof(RECT));
        ZeroMemory(&rClipbook, sizeof(RECT));
        ZeroMemory(&rcOutput, sizeof(RECT));

        if (g_hwndRebarMain != NULL && IsWindowVisible(g_hwndRebarMain))
        {
            GetWindowRect(g_hwndRebarMain, &rRebar);
            cWindows++;
            bRebar = TRUE;
        }
        if (g_hwndStatusbarMain != NULL && IsWindowVisible(g_hwndStatusbarMain))
        {
            cWindows++;
            bStatusbar = TRUE;
        }
        if (g_hwndTabWindowBar != NULL && IsWindowVisible(g_hwndTabWindowBar))
        {
            GetWindowRect(g_hwndTabWindowBar, &rTabWindowBar);
            cWindows++;
            bTabWindowBar = TRUE;
        }
        if (g_hwndClipbook != NULL && IsWindowVisible(g_hwndClipbook))
        {
            GetWindowRect(g_hwndClipbook, &rClipbook);
            cWindows++;
            bClipbook = TRUE;
        }
        if (IsWindow(g_hwndOutputWindow) && IsWindowVisible(g_hwndOutputWindow))
        {
            GetWindowRect(g_hwndOutputWindow, &rcOutput);
            cWindows++;
            bOutput = TRUE;
        }
        if (g_hwndMDIClient != NULL && IsWindowVisible(g_hwndMDIClient))
        {
            cWindows++;
            bMDI = TRUE;
        }

        hdwp = BeginDeferWindowPos(cWindows);

        if (bRebar)
        {
            hdwp = DeferWindowPos(hdwp, g_hwndRebarMain, NULL,
                        0, 0,
                        Rect_Width(&rcClient), Rect_Height(&rRebar),
                        SWP_NOZORDER);

            rcClient.top += Rect_Height(&rRebar) + 2;
        }

        if (bStatusbar)
        {
            hdwp = DeferWindowPos(hdwp, g_hwndStatusbarMain, NULL,
                        0, (rcClient.bottom - tm.tmHeight),
                        Rect_Width(&rcClient), tm.tmHeight,
                        SWP_NOZORDER);

            rcClient.bottom -= tm.tmHeight;
        }

        if (bTabWindowBar)
        {
            hdwp = DeferWindowPos(hdwp, g_hwndTabWindowBar, NULL,
                        0, rcClient.bottom - Rect_Height(&rTabWindowBar),
                        Rect_Width(&rcClient), tm.tmHeight + 6,
                        SWP_NOZORDER);

            rcClient.bottom -= (tm.tmHeight + 6) + 2;
        }

        if (bOutput)
        {
            DWORD dwAlignment = BandCtrl_GetAlignment(g_hwndOutputWindow);

            if (dwAlignment == CCS_BOTTOM)
            {
                hdwp = DeferWindowPos(hdwp, g_hwndOutputWindow, NULL,
                                0, rcClient.bottom - Rect_Height(&rcOutput),
                                Rect_Width(&rcClient), Rect_Height(&rcOutput),
                                SWP_NOZORDER);

                rcClient.bottom -= Rect_Height(&rcOutput) + 2;
            }
            else if (dwAlignment == CCS_TOP)
            {
                hdwp = DeferWindowPos(hdwp, g_hwndOutputWindow, NULL,
                                    0, rcClient.top,
                                    Rect_Width(&rcClient), Rect_Height(&rcOutput),
                                    SWP_NOZORDER);

                rcClient.top    += Rect_Height(&rcOutput) + 2;
            }
            else
            {
                hdwp = DeferWindowPos(hdwp, g_hwndOutputWindow, HWND_TOP,
                                    g_WindowSettings.nOutputWindowXPos, g_WindowSettings.nOutputWindowYPos,
                                    g_WindowSettings.nOutputWindowWidth, g_WindowSettings.nOutputWindowHeight,
                                    0);
            }
        }

        if (bClipbook)
        {
            DWORD dwAlignment = BandCtrl_GetAlignment(g_hwndClipbook);

            if (dwAlignment == CCS_LEFT)
            {
                hdwp = DeferWindowPos(hdwp, g_hwndClipbook, NULL,
                            0, rcClient.top,
                            Rect_Width(&rClipbook), Rect_Height(&rcClient),
                            SWP_NOZORDER);

                rcClient.left += Rect_Width(&rClipbook) + 2;
            }
            else if (dwAlignment == CCS_RIGHT)
            {
                hdwp = DeferWindowPos(hdwp, g_hwndClipbook, NULL,
                        rcClient.right - Rect_Width(&rClipbook), rcClient.top,
                        Rect_Width(&rClipbook), Rect_Height(&rcClient),
                        SWP_NOZORDER);
        
                rcClient.right -= Rect_Width(&rClipbook);
            }
            else
            {
                hdwp = DeferWindowPos(hdwp, g_hwndClipbook, HWND_TOP,
                                g_WindowSettings.nClipbookXPos, g_WindowSettings.nClipbookYPos,
                                g_WindowSettings.nClipbookWidth, g_WindowSettings.nClipbookHeight,
                                0);
            }

            Window_Size(BandCtrl_GetClientWindow(g_hwndClipbook), -1, -1);
        }

        if (bMDI)
        {
            hdwp = DeferWindowPos(hdwp, g_hwndMDIClient, NULL,
                        rcClient.left, rcClient.top,
                        Rect_Width(&rcClient), Rect_Height(&rcClient),
                        SWP_NOZORDER);
        }

        if (bClipbook || bOutput)
        {
            int cx;
            int cy;

            cx = Rect_Width(&rc);
            cy = Rect_Height(&rc);

            if (bRebar)
            {
                GetWindowRect(g_hwndRebarMain, &rRebar);
                cy -= Rect_Height(&rRebar);
            }
            if (bStatusbar)
            {
                GetWindowRect(g_hwndStatusbarMain, &rStatusbar);
                cy -= Rect_Height(&rStatusbar);
            }
            if (bTabWindowBar)
            {
                GetWindowRect(g_hwndTabWindowBar, &rTabWindowBar);
                cy -= Rect_Height(&rTabWindowBar);
            }

            if (bClipbook)
                BandCtrl_SetMaxSize(g_hwndClipbook, cx, cy);
            if (bOutput)
                BandCtrl_SetMaxSize(g_hwndOutputWindow, cx, cy);
        }

        EndDeferWindowPos(hdwp);


        /* now that the mdi frame has been resized, resize it's children.
         * this could of course have been done inside the defering of
         * the main window. However, the loss is miniscule
         */
        if (bMDI)
            MDI_Client_ResizeChildren(g_hwndMDIClient);

        /* Read above...same for statusbar basically...need to resize
         * panes AFTER the statusbar has been resized =\
         */
        if (bStatusbar)
            Statusbar_OnSize(g_hwndStatusbarMain, state);
    }
}

static void Main_OnWindowPosChanged(HWND hwnd, const LPWINDOWPOS lpwpos)
{
    if (!IsMaximized(hwnd))
    {
        g_WindowSettings.nMainXPos      = lpwpos->x;
        g_WindowSettings.nMainYPos      = lpwpos->y;
        g_WindowSettings.nMainWidth     = lpwpos->cx;
        g_WindowSettings.nMainHeight    = lpwpos->cy;
    }

    DefWindowProc(hwnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)lpwpos);

    MDI_Client_ResizeChildren(g_hwndMDIClient);
}

static void Main_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpDrawItem)
{
    switch (lpDrawItem->CtlID)
    {
        case IDC_STATUSBAR_MAIN:
            Statusbar_Draw(lpDrawItem);
        break;
    }
}

static void Main_OnMenuSelect(HWND hwnd, HMENU hMenu, int item, HMENU hMenuPopup, UINT flags)
{
    TCHAR szText[SB_MAX_TEXT];

    //check this one (flags part)
    if ((item == 0) && (flags == 0xFFFFFFFF) && !(flags & MF_SYSMENU))
    {
        MyStatusbar_Ready(g_hwndStatusbarMain);

        return;
    }
    else if (!(flags & MF_SEPARATOR) /*&& !(flags & MF_SYSMENU)*/ && !(flags & MF_POPUP)
            || (item == IDM_HELP_ABOUT))
    {
        _tcsncpy(szText, String_LoadString(item), SB_MAX_TEXT);
    }
    else
    {
        MyStatusbar_Ready(g_hwndStatusbarMain);

        return;
    }

    Statusbar_SetPaneText(g_hwndStatusbarMain, 0, szText);
}

static void Main_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
    switch (cmd)
    {
        case SC_MAXIMIZE:
            g_WindowSettings.bMainMaximized = TRUE;
        break;
        case SC_RESTORE:
            g_WindowSettings.bMainMaximized = FALSE;
        break;
        case IDM_HELP_ABOUT:
            About_Initialize(hwnd);
        return;
    }

    DefFrameProc(hwnd, g_hwndMDIClient, WM_SYSCOMMAND, cmd, MAKELPARAM(x, y));
}

static BOOL Main_OnCopyData(HWND hwnd, HWND hwndSender, PCOPYDATASTRUCT pcds)
{
    Main_ProcessCommandLine(hwnd, (LPTSTR)pcds->lpData);

    if (Systray_IsInTray())
    {   
        ShowWindow(hwnd, SW_SHOW);
        Systray_SetInTray(!Systray_DeleteIcon(hwnd, IDC_TRAYICON_MAIN));
    }

    return (TRUE);
}


static void Main_OnTrayNotify(HWND hwnd, UINT idCtl, UINT uNotifyCode)
{
    switch (idCtl)
    {
        case IDC_TRAYICON_MAIN:
            switch (uNotifyCode)
            {
                case WM_LBUTTONUP:
                    Window_Command(hwnd, IDM_TRAY_RESTORE, 0, 0);
                break;
                case WM_RBUTTONUP:
                {
                    HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_TRAYMENU));
                    HMENU hMenuTray = GetSubMenu(hMenu, 0);
                    MENUITEMINFO mii;

                    INITSTRUCT(mii, TRUE);
                    mii.fMask       = MIIM_SUBMENU;
                    mii.hSubMenu    = g_hMenuRecentFiles;
                    
                    SetMenuItemInfo(hMenuTray, 3, MF_BYPOSITION, &mii);
                    SetMenuDefaultItem(hMenuTray, IDM_TRAY_RESTORE, FALSE);

                    SetForegroundWindow(hwnd);
                    Menu_TrackPopupMenu(hMenuTray, -1, -1, hwnd);
                    DestroyMenu(hMenu);
                }
                break;
            }
        break;
    }
}

BOOL Main_CreateChildWindows(HWND hwnd)
{
    g_hwndMDIClient         = NULL;
    g_hwndStatusbarMain     = NULL;
    g_hwndTabWindowBar      = NULL;
    g_hwndClipbook          = NULL;
    g_hwndOutputWindow      = NULL;
    g_hwndRebarMain         = NULL;
    g_hwndToolbarMain       = NULL;
    g_hwndCboQuickSearch    = NULL;
    g_hwndDlgGotoline       = NULL;

    // Creation of Rebar takes 3 ms

    // Creation of Toolbar takes 40 ms
    // Always create toolbar...it will also create the rebar...
    // If it isn't active then toggle it again to hide it
    // (rebar will also be hidden)
    // this needs to be fixed in the future (disabled for now)
    if (g_WindowSettings.bToolbarMainActive)
    {
        Rebar_ToggleItem(IDC_TOOLBAR_MAIN);
    }
    else
    {
        Rebar_ToggleItem(IDC_TOOLBAR_MAIN);
        Rebar_ToggleItem(IDC_TOOLBAR_MAIN);
    }

    // Creation of Find Combo takes 30 ms
    if (g_WindowSettings.bQuickSearchActive)
        Rebar_ToggleItem(IDC_COMBO_QUICKSEARCH);
    // Creation of Gotoline Dialog takes XXX ms
    if (g_WindowSettings.bGotoLineActive)
        Rebar_ToggleItem(IDC_DLG_GOTOLINE);

    // Creation of Windowbar (Tab control) (initalized in commctrl.dll) takes 0 ms
    Main_ToggleChildWindow(IDC_TAB_WINDOWBAR, FALSE);
    // If we don't want it visible, then hide it
    if (!g_WindowSettings.bTabWindowBarActive)
        Main_ToggleChildWindow(IDC_TAB_WINDOWBAR, FALSE);

    // Creation of Status Bar (initialized in commctrl.dll) takes 0 ms
    Main_ToggleChildWindow(IDC_STATUSBAR_MAIN, FALSE);
    // If we don't want it visible, then hide it
    if (g_WindowSettings.bStatusbarActive)
        Main_ToggleChildWindow(IDC_STATUSBAR_MAIN, FALSE);

    // Creation of MDI Client takes 0 ms
    g_hwndMDIClient = MDI_Client_Create(hwnd);

    // Creation of Clipbook takes 105 ms
    if (g_WindowSettings.bClipbookActive)
        Main_ToggleChildWindow(IDC_CLIPBOOK, FALSE);

    // Total 213 ms

    if (g_WindowSettings.bOutputWindowActive)
        Main_ToggleChildWindow(IDC_OUTPUTWINDOW, FALSE);

    if (g_hwndMDIClient == NULL)
        return (FALSE);
    else
        return (TRUE);
}

void Main_ToggleChildWindow(UINT uID, BOOL bResize)
{
    BOOL bIsWindow = FALSE;
    BOOL bVisible = TRUE;
    UINT uMenuID = 0;
    HMENU hMenu;

    switch (uID)
    {
    case IDC_STATUSBAR_MAIN:
        if (IsWindow(g_hwndStatusbarMain))
        {
            if (IsWindowVisible(g_hwndStatusbarMain))
            {
                ShowWindow(g_hwndStatusbarMain, SW_HIDE);
                bVisible = FALSE;
            }
            else
            {
                ShowWindow(g_hwndStatusbarMain, SW_SHOW);
            }
        }
        else
        {
            g_hwndStatusbarMain = MyStatusbar_Create(g_hwndMain);
        }

        bIsWindow   = (IsWindow(g_hwndStatusbarMain) && bVisible);
        uMenuID     = IDM_VIEW_CHILDREN_STATUSBAR;
        g_WindowSettings.bStatusbarActive = bIsWindow;
    break;
    case IDC_TAB_WINDOWBAR:
        if (IsWindow(g_hwndTabWindowBar))
        {
            if (IsWindowVisible(g_hwndTabWindowBar))
            {
                ShowWindow(g_hwndTabWindowBar, SW_HIDE);
                bVisible = FALSE;
            }
            else
            {
                ShowWindow(g_hwndTabWindowBar, SW_SHOW);
            }
        }
        else
        {
            g_hwndTabWindowBar  = Windowbar_Create(g_hwndMain);
        }

        bIsWindow   = (IsWindow(g_hwndTabWindowBar) && bVisible);
        uMenuID     = IDM_VIEW_CHILDREN_WINDOWBAR;
        g_WindowSettings.bTabWindowBarActive = bIsWindow;
    break;
    case IDC_CLIPBOOK:
        if (IsWindow(g_hwndClipbook))
            g_hwndClipbook  = Clipbook_Destroy(g_hwndClipbook);
        else
            g_hwndClipbook  = Clipbook_Create(g_hwndMain);

        bIsWindow   = IsWindow(g_hwndClipbook);
        uMenuID     = IDM_VIEW_CHILDREN_CLIPBOOK;
        g_WindowSettings.bClipbookActive = bIsWindow;
    break;
    case IDC_OUTPUTWINDOW:
        if (IsWindow(g_hwndOutputWindow))
            g_hwndOutputWindow  = OutputWindow_Destroy(g_hwndOutputWindow);
        else
            g_hwndOutputWindow  = OutputWindow_Create(g_hwndMain);

        bIsWindow   = IsWindow(g_hwndOutputWindow);
        uMenuID     = IDM_VIEW_CHILDREN_WINDOWBAR;
        g_WindowSettings.bOutputWindowActive = bIsWindow;
    break;
    default:
        ASSERT(FALSE);
    break;
    }

    hMenu = Main_GetMainSubMenu(g_hwndMain, IDM_MAIN_VIEW);
    Menu_CheckMenuItem(hMenu, uMenuID, bIsWindow);
//  DestroyMenu(hMenu);

    if (IsWindow(g_hwndToolbarMain))
        Toolbar_CheckButton(g_hwndToolbarMain, uMenuID, bIsWindow);

    if (bResize)
        Window_UpdateLayout(g_hwndMain);
}

static BOOL Main_ProcessCommandLine(HWND hwndMain, LPTSTR pszCommandLine)
{
    TCHAR szFile[MAX_PATH], szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szName[_MAX_FNAME], szExt[_MAX_EXT];

    if (_tcslen(pszCommandLine) == 0)
    {
        Window_Command(hwndMain, IDM_FILE_NEW, 0, 0);

        return (FALSE);
    }

    if (pszCommandLine[0] == _T('\"'))
        pszCommandLine++;

    if (pszCommandLine[_tcslen(pszCommandLine) - 1] == _T('\"'))
        pszCommandLine[_tcslen(pszCommandLine) - 1] = _T('\0');

    _splitpath(pszCommandLine, szDrive, szDir, szName, szExt);

    if ((_tcslen(szDrive)) == 0)
    {
        GetCurrentDirectory(MAX_PATH, szFile);
        _tcscat(szFile, _T("\\"));
        _tcscat(szFile, pszCommandLine);

    }
    else
    {
        _tcscpy(szFile, pszCommandLine);
    }

    File_Open(szFile, CRLF_STYLE_AUTOMATIC);

    return (TRUE);
}

void Main_WriteLogEntry(LPTSTR pszLogEntry)
{
    LPTSTR szLogEntryTemplate =
_T("\
*************************************************\n\
* Debug Log Entry:\n\
* Time-Date: %s-%s\n\
* %s\n\
*************************************************\n\n");
//* GetLastError() returned %s\r\n\r\n");
    FILE *f;
    TCHAR szLogFile[MAX_PATH];
    TCHAR szTime[128], szDate[128], szFileName[MAX_PATH];
    LPTSTR pszActualLogEntry = (LPTSTR)Mem_Alloc(
                                        _tcslen(pszLogEntry) +
                                        _tcslen(szLogEntryTemplate) +
                                        256 + /* Time var */
                                        SZ);

    Path_GetModuleFileName(g_hInstance, szLogFile, MAX_PATH);
    Path_GetModuleFileName(g_hInstance, szFileName, MAX_PATH);

    Path_RemoveFileName(szLogFile);
    _tcscat(szLogFile, _T("User\\"));
    _tcscat(szLogFile, Path_GetFileName(szFileName));
    Path_SwapExt(szLogFile, _T("log"));

    _tzset();
    _strtime(szTime);
    _strdate(szDate);
    _stprintf(pszActualLogEntry, szLogEntryTemplate, szTime, szDate, pszLogEntry);

    if ((f = fopen(szLogFile, _T("a"))) == NULL)
        return;

    fwrite(pszActualLogEntry, sizeof(TCHAR), _tcslen(pszActualLogEntry), f);

    fclose(f);

    Mem_Free(pszActualLogEntry);
}

void Main_FocusNext(BOOL bNext)
{
#define WINDOW_COUNT (5)
    HWND hwndArray[WINDOW_COUNT] = {
            MDI_MyGetActive(TRUE),
            ComboBoxEx_GetEditControl(GetDlgItem(MDI_MyGetActive(FALSE), IDC_CBO_TCL)),
            ComboBoxEx_GetEditControl(g_hwndCboQuickSearch),
//          ComboBoxEx_GetComboControl(GetDlgItem(g_hwndClipbook, IDC_CBOCLIPS)),
//          GetDlgItem(g_hwndClipbook, IDC_LISTCLIPS),
            g_hwndClipbook,
            GetDlgItem(g_hwndOutputWindow, IDC_OUTPUTWINDOW_EDIT) };
    HWND hwndFocus    = GetFocus();
    int i;

    for (i = 0; i < WINDOW_COUNT; i++)
    {
        if (hwndArray[i] == hwndFocus)
        {
            if (bNext)
            {
                int j = ((i == WINDOW_COUNT - 1) ? 0 : (i + 1));
                BOOL bLooped = FALSE;

                while (!IsWindow(hwndArray[j]))
                {
                    j++;

                    if (j == WINDOW_COUNT - 1)
                    {
                        if (bLooped)
                        {
                            break;
                        }
                        else
                        {
                            j = 0;
                            bLooped = TRUE;
                        }
                    }
                }

                SetFocus(hwndArray[j]);
            }
            else
            {
                int j = ((i == 0) ? (WINDOW_COUNT - 1) : (i - 1));
                BOOL bLooped = FALSE;

                while (!IsWindow(hwndArray[j]))
                {
                    j--;

                    if (j == 0)
                    {
                        if (bLooped)
                        {
                            break;
                        }
                        else
                        {
                            j = (WINDOW_COUNT - 1);
                            bLooped = TRUE;
                        }
                    }
                }

                SetFocus(hwndArray[j]);
            }
        }
    }
}

unsigned long Main_HandleException(HWND hwnd, LPMSG lpmsg, EXCEPTION_POINTERS *pExceptionInfo)
{
    EXCEPTION_RECORD *pException = pExceptionInfo->ExceptionRecord;
    CONTEXT *pContext = pExceptionInfo->ContextRecord;
    TCHAR szClassName[256];
    TCHAR szTitle[256];
    TCHAR szQuestion[2048];
    UINT i;
    int nUserResponse;

    // break program on a 'non-continuable' exception
    if (pException->ExceptionFlags)
        return (EXCEPTION_CONTINUE_SEARCH);

    // let debugger handle debugging exceptions
    if (pException->ExceptionCode == EXCEPTION_BREAKPOINT ||
        pException->ExceptionCode == EXCEPTION_SINGLE_STEP)
    {
        return (EXCEPTION_CONTINUE_SEARCH);
    }
    
    GetClassName(lpmsg->hwnd, szClassName, 256);
    GetWindowText(lpmsg->hwnd, szTitle, 256);

    _sntprintf(szQuestion, 2048, String_LoadString(IDS_FORMAT_EXCEPTION),
    lpmsg->hwnd, lpmsg->hwnd,
    lpmsg->message, lpmsg->message,
    lpmsg->wParam, lpmsg->wParam,
    lpmsg->lParam, lpmsg->lParam,
    szClassName, szTitle,
    pException->ExceptionCode, pException->ExceptionCode,
    pException->ExceptionFlags, pException->ExceptionFlags,
    (pException->ExceptionRecord) ? _T("yes") : _T("no"),
    pException->ExceptionAddress, pException->ExceptionAddress,
    pException->NumberParameters);
    
    for (i = 0; i < pException->NumberParameters; i++)
    {
        TCHAR szParam[256];
        
        _stprintf(szParam, _T("\nParameter[%d] = 0x%X (%d)"),
            i,
            pException->ExceptionInformation[i],
            pException->ExceptionInformation[i]);

        _tcscat(szQuestion, szParam);
    }
    
    _tcscat(szQuestion, _T("\n\nDo you wish to attempt to\n")
                        _T("continue executing the program?"));
    
    nUserResponse = MessageBox(hwnd, szQuestion, _T("Exception Handler"),
                                MB_ICONEXCLAMATION|MB_YESNO);

    if (nUserResponse == IDYES)
        return (EXCEPTION_EXECUTE_HANDLER);
    else
        return (EXCEPTION_CONTINUE_SEARCH);
}

#ifdef _DEBUG
int _cdecl Main_CrtAllocHook(int nAllocType, void *pvData, size_t nSize, int nBlockUse, long lRequest, const unsigned char *szFileName, int  nLine)
{
    LPTSTR pszOperation[] = { _T(""), _T("allocating"), _T("re-allocating"), _T("freeing") };
    LPTSTR pszBlockType[] = { _T("Free"), _T("Normal"), _T("CRT"), _T("Ignore"), _T("Client") };
    
    if (nBlockUse == _CRT_BLOCK)   // Ignore internal C runtime library allocations
        return(TRUE);
    
    ASSERT((nAllocType > 0) && (nAllocType < 4));
    ASSERT((nBlockUse >= 0) && (nBlockUse < 5));
    
    TRACE(String_LoadString(IDS_FORMAT_TRACE_MEMORYLEAK),
            szFileName, nLine, pszOperation[nAllocType], nSize, 
            pszBlockType[nBlockUse], lRequest);

    if (pvData != NULL)
        TRACE(_T(" at %X"), pvData);
    
    return (TRUE);      // Allow the memory operation to proceed
}
#endif /* _DEBUG */

static DWORD WINAPI Main_TclLoadThread(LPVOID lpParameter)
{
    if (!MyTcl_Init())
        MessageBox(NULL, String_LoadString(IDS_ERROR_TCL_FAILEDTOLOAD), NULL, MB_OK);

    _endthreadex(0);
    return (0);
}

static void Main_CreateDirectories(void)
{
    int i;
    TCHAR szFileName[MAX_PATH];
    TCHAR szPath[MAX_PATH];
    TCHAR aszPathNames[2][16] =
    {
        _T("System"),
        _T("User")
    };

    Path_GetModuleFileName(g_hInstance, szFileName, sizeof(szFileName));

    Path_RemoveFileName(szFileName);

    for (i = 0; i < DIMOF(aszPathNames); i++)
    {
        _sntprintf(szPath, MAX_PATH, _T("%s%s"), szFileName, aszPathNames[i]);

        if (GetFileAttributes(szPath) != FILE_ATTRIBUTE_DIRECTORY)
            CreateDirectory(szPath, NULL);
    }
}

// HWND hwnd could be removed since we're always gonna be doing GetMenu(g_hwndMain) anyhow

HMENU Main_GetMainSubMenu(HWND hwnd, int nPos)
{
    return (GetSubMenu(GetMenu(hwnd), (IsMaximized(MDI_MyGetActive(FALSE))) ? nPos + 1 : nPos));
}


/* the new main stuff starts here */

__inline HWND Main_GetWindow(void)
{
    return (s_hwndMain);
}

__inline HWND Main_GetInstance(void)
{
    return (s_hInstance);
}
