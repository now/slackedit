/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_edit
 *
 * File       : pcp_editview_internal.h
 * Created    : not known (before 01/03/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:41:42
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __EDITVIEW_INTERNAL_H
#define __EDITVIEW_INTERNAL_H

typedef struct tagINSERTCONTEXT
{
	BOOL bInsert;
	POINT ptStart;
	POINT ptEnd;
} INSERTCONTEXT, FAR *LPINSERTCONTEXT;

typedef struct tagTEXTBLOCK
{
	int	iCharPos;
	int iColorIndex;
} TEXTBLOCK, FAR *LPTEXTBLOCK;

/* pcp_edit_interface */
LRESULT CALLBACK PCP_Edit_Interface_HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

#endif /* __EDITVIEW_INTERNAL_H */
