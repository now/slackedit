/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : output.h
 * Created    : 01/24/00
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:13:34
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __OUTPUT_H
#define __OUTPUT_H

#define IDC_OUTPUTWINDOW_EDIT 2001 /* A Space Odyssey */

HWND OutputWindow_Create(HWND hwndParent);
HWND OutputWindow_Destroy(HWND hwndOutputWindow);

#endif /* __OUTPUT_H */
