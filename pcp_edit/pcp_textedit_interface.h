/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_edit_interface.h
 * Created    : 07/22/00
 * Owner      : pcppopper
 * Revised on : 07/22/00
 * Comments   : 
 *              
 *              
 ****************************************************************/

#ifndef __PCP_EDIT_INTERFACE_H
#define __PCP_EDIT_INTERFACE_H

/* pcp_edit */
#include "pcp_textedit_view.h" /* For: LPTEXTEDITVIEW */

typedef struct tagTEXTEDITINTERFACE
{
	LPEDITINTERFACE		lpInterface;
	LPTEXTEDITVIEW		lpte;
	LPEDITWINDOW		lpEditWindow;
} TEXTEDITINTERFACE, *LPTEXTEDITINTERFACE;

void TextEdit_Internal_SetupInterface(void);
void TextEdit_Internal_DeleteInterface(void);

BOOL TextEdit_Internal_CreateInterface(HWND hwnd);
LPTEXTEDITINTERFACE TextEdit_Internal_GetInterface(HWND hwnd);
BOOL TextEdit_Internal_DestroyInterface(HWND hwnd);

#endif /* __PCP_EDIT_INTERFACE_H */
