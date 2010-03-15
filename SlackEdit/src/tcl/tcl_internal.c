/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : tcl_int.c
 * Created    : not known   (before 09/05/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:06:40
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* standard */
#include <limits.h>
#include <tcl.h>

/* SlackEdit */
#include "tcl_internal.h"

/* pcp_generic */
#include <pcp_mem.h>

#define ERANGE                  34

LPTSTR Tcl_Int_DoFileName(LPTSTR pszOldFile)
{
    LPTSTR pszNewFile, pszEnv, pszEnvSub;
    int i = 0, j, iFilePos = 0, iStartPos = 0, iEnvSubLen, iEnvLen = 0;
    BOOL fChkEnv = FALSE;
    
    if ((pszNewFile = (LPTSTR)Mem_AllocStr(SZ)) == NULL)
        return (NULL);

    for (i = 0, iFilePos = 0; pszOldFile[i] != _T('\0'); i++)
    {
        if (!fChkEnv)
        {
            if (pszOldFile[i] == '%')
            {
                fChkEnv = TRUE;
                iStartPos = i;
                continue;
            }

            if ((pszNewFile = (LPTSTR)Mem_ReAllocStr(pszNewFile,
                    Mem_SizeStr(pszNewFile) + 1)) == NULL)
                return (NULL);
            
            if (pszOldFile[i] == _T('/'))
                *(pszNewFile + (iFilePos++)) = _T('\\');
            else
                *(pszNewFile + (iFilePos++)) = pszOldFile[i];
        }
        else
        {
            if (pszOldFile[i] != '%')
            {
                iEnvLen++;
            }
            else
            {
                fChkEnv = FALSE;        /* reset flag */
                
                if ((pszEnv = (LPTSTR)Mem_AllocStr(iEnvLen + SZ)) == NULL)
                    return (NULL);
                
                for (j = 0; j < iEnvLen; j++)
                    *(pszEnv + j) = pszOldFile[iStartPos + 1 + j];
                
                *(pszEnv + j) = _T('\0');

                if ((pszEnvSub = getenv(pszEnv)) == NULL)
                {
                    Mem_Free(pszEnv);
                    continue;
                }

                iEnvSubLen = _tcslen(pszEnvSub);

                if ((pszNewFile = (LPTSTR)Mem_ReAlloc(pszNewFile,
                            (iFilePos + iEnvSubLen + 1))) == NULL)
                {
                    Mem_Free(pszEnv);
                    return (NULL);
                }

                for (j = 0; j < iEnvSubLen; j++)
                    *(pszNewFile + (iFilePos++)) = *(pszEnvSub + j);
                
                Mem_Free(pszEnv);
            }
        }
    }

    /* Add the terminating NULL to the string */
    if ((pszNewFile = (LPTSTR)Mem_ReAllocStr(pszNewFile,
                Mem_SizeStr(pszNewFile) + 1)) == NULL)
                return (NULL);

    *(pszNewFile + (i) + 1) = _T('\0');
    
    return (pszNewFile);
}
