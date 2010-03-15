/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : config.h
 * Created    : not known
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:03:46
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

#define MAX_FILTER			4096
#define REG_SLACKEDIT		_T("Software\\da.box Software Division\\SlackEdit")
#define REG_ROOT			HKEY_CURRENT_USER
#define REG_SLACKEDIT_TYPE	_T("SlackEditFile")

typedef struct tagFILETYPEDATA
{
	TCHAR szExt[MAX_PATH];
	TCHAR szDefaultIcon[MAX_PATH + 2];
	TCHAR szCommand[20];
	TCHAR szDescription[MAX_PATH];
} FILETYPEDATA, *LPFILETYPEDATA;

typedef struct tagFILTERDATA
{
	LPTSTR pszDisplay;
	LPTSTR pszFileTypes;
} FILTERDATA, *LPFILTERDATA;

void Config_Read(void);
void Config_Write(void);
void Config_Edit(HWND hwndOwner);

void MRU_SetListView(HWND hwndList, LPTSTR pszSubKey, int nMaxItems);

void Config_SaveOpenWindows(void);
void Config_LoadLastOpenWindows(void);

void Config_LoadToolbar(void);
void Config_SaveToolbar(void);

struct GlobalSettings
{
	BOOL	fGotoSystray;
	BOOL	bMultipleInstances;
	BOOL	bSaveOpenWindows;
	BOOL	bOnlyReloadIfCommandLineEmpty;
	BOOL	bTipOfDaDay;
	BOOL	bLastExitSuccessful;
	BOOL	bCrashProtection;
} g_GlobalSettings;

struct EditSettings
{
	BOOL	bRecentToClipboard;
} g_EditSettings;

struct BackupSettings
{
	TCHAR	szBackupExt[_MAX_EXT];
	BOOL	bBackup;
	TCHAR	szBackupDirectory[MAX_PATH];
	BOOL	bUseBackupDirectory;
	TCHAR	szAutoSaveExt[_MAX_EXT];
	BOOL	bAutoSave;
	UINT	uAutoSaveInterval;
	TCHAR	szAutoSaveDirectory[MAX_PATH];
	BOOL	bUseAutoSaveDirectory;
} g_BackupSettings;

struct CBSettings
{
	int		nLastIndex;
} g_CBSettings;

struct FindSettings
{
	BOOL	bWholeWords;
	BOOL	bCase;
	BOOL	bRegExp;
	BOOL	bWrapSearch;
	int		nDirection;
	int		nContext;
} g_FindSettings;

struct WindowSettings
{
	BOOL	bMainCentered;
	BOOL	bMainSticky;
	BOOL	bMainOnTop;
	BOOL	bMainMaximized;
	BOOL	bFindCentered;
	BOOL	bAdvancedOpenCentered;
	BOOL	bReplaceCentered;
	BOOL	bFavoritesCentered;
	BOOL	bPrintCentered;
	BOOL	bConfigCentered;
	BOOL	bAboutCentered;
	BOOL	bWindowListCentered;
	BOOL	bClipbookActive;
	BOOL	bOutputWindowActive;
	BOOL	bToolbarMainActive;
	BOOL	bQuickSearchActive;
	BOOL	bGotoLineActive;
	BOOL	bTabWindowBarActive;
	BOOL	bStatusbarActive;
	int		nMainXPos;
	int		nMainYPos;
	int		nMainWidth;
	int		nMainHeight;
	int		nAdvancedOpenXPos;
	int		nAdvancedOpenYPos;
	int		nFindXPos;
	int		nFindYPos;
	int		nReplaceXPos;
	int		nReplaceYPos;
	int		nFavoritesXPos;
	int		nFavoritesYPos;
	int		nConfigXPos;
	int		nConfigYPos;
	int		nAboutXPos;
	int		nAboutYPos;
	int		nPrintXPos;
	int		nPrintYPos;
	int		nClipbookXPos;
	int		nClipbookYPos;
	int		nClipbookWidth;
	int		nClipbookHeight;
	int		nClipbookAlignment;
	int		nWindowListX;
	int		nWindowListY;
	int		nOutputWindowXPos;
	int		nOutputWindowYPos;
	int		nOutputWindowWidth;
	int		nOutputWindowHeight;
	int		nOutputWindowAlignment;
	int		nMDIWinState; 
} g_WindowSettings;

struct FilterSettings
{
	TCHAR szOpenFilter[MAX_FILTER];
	TCHAR szSaveFilter[MAX_FILTER];
} g_FilterSettings;

struct MRUSettings
{
	int	nAdvancedOpenDirMax;
	int	nAdvancedOpenFileMax;
	int	nFileMax;
	int	nFindMax;
	int	nReplaceMax;
	int	nFavoritesMax;
	int	nLinesMax;
	int	nTclCmdMax;
} g_MRUSettings;

struct ImageSettings
{
	BOOL	bMDIHatch;
	BOOL	bMDIImage;
	BOOL	bMDITransparent;
	BOOL	bMDITile;
	BOOL	bCoolbarImage;
	TCHAR	szCoolbarImage[MAX_PATH];
	TCHAR	szMDIImage[MAX_PATH];
	TCHAR	szMarginCursorFile[MAX_PATH];
} g_ImageSettings;

struct ScriptSettings
{
	TCHAR	szScriptDir[MAX_PATH];
	TCHAR	szTclInstallDir[MAX_PATH];
} g_ScriptSettings;

struct PrintSettings
{
	UINT	uMarginLeft;
	UINT	uMarginRight;
	UINT	uMarginTop;
	UINT	uMarginBottom;
} g_PrintSettings;

#endif /* __CONFIG_H */
