/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_dll.c
 * Created    : 07/23/00
 * Owner      : pcppopper
 * Revised on : 07/23/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include <pcp_includes.h>

/* pcp_textedit */
#include "pcp_textedit_internal.h"

/****************************************************************
 * Type Definitions
 ****************************************************************/

/****************************************************************
 * Function Definitions
 ****************************************************************/

/****************************************************************
 * Global Variables
 ****************************************************************/

static HINSTANCE g_hDllInstance = NULL;

/****************************************************************
 * Function Implementations
 ****************************************************************/

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hDllInstance = hinstDLL;

            TextEdit_Internal_RegisterControl(GetModuleHandle(NULL));
        break;
        case DLL_PROCESS_DETACH:
            TextEdit_Internal_UnregisterControl(GetModuleHandle(NULL));
        break;
    }
    return (TRUE);
}

HINSTANCE PCP_Edit_Dll_GetHandle(void)
{
    return (g_hDllInstance);
}
