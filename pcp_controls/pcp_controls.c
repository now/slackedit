/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_controls.c
 * Created    : not known (before 12/01/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:44:54
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

int iRefCount = 0;
HINSTANCE g_hDllInstance = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            iRefCount++;

            if (iRefCount == 1)
                g_hDllInstance = hinstDLL;
        break;
        case DLL_PROCESS_DETACH:
            iRefCount--;
        break;
    }
    return (TRUE);
}

HINSTANCE PCPControls_GetHandle(void)
{
    return (g_hDllInstance);
}
