/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_textedit.c
 * Created    : 07/23/00
 * Owner      : pcppopper
 * Revised on : 07/23/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include <pcp_includes.h>

/* pcp_edit */
#include "pcp_textedit.h"
#include "pcp_textedit_internal.h"
#include "pcp_textedit_interface.h"

/****************************************************************
 * Type Definitions
 ****************************************************************/

/****************************************************************
 * Function Definitions
 ****************************************************************/

/****************************************************************
 * Global Variables
 ****************************************************************/

/****************************************************************
 * Function Implementations
 ****************************************************************/

void PCP_Edit_TextEdit_AddSyntaxDefinition(LPSYNTAXDEFINITION lptd)
{
    TextEdit_Internal_AddSyntaxDefinition(lptd);
}

LPEDITINTERFACE PCP_Edit_TextEdit_GetInterface(HWND hwnd)
{
    LPTEXTEDITINTERFACE lpteInterface;
    
    lpteInterface = TextEdit_Internal_GetInterface(hwnd);

    return ((LPEDITINTERFACE)lpteInterface);
}
