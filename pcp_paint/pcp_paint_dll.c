/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_paint
 *
 * File       : pcp_paint_dll.c
 * Created    : 06/27/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 13:29:04
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

HINSTANCE PCPPaint_GetHandle(void)
{
    return (g_hDllInstance);
}
