/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_combobox.h
 * Created    : 10/04/99
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:21:15
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __COMBOBOXEX_H
#define __COMBOBOXEX_H

#define ComboBoxEx_GetItem(hwndCtl, lpcbxi)		((DWORD)SNDMSG((hwndCtl), CBEM_GETITEM, 0, (LPARAM)(PCOMBOBOXEXITEM)(lpcbxi)))
#define ComboBoxEx_InsertItem(hwndCtl, lpcbxi)	((DWORD)SNDMSG((hwndCtl), CBEM_INSERTITEM, 0, (LPARAM)(const COMBOBOXEXITEM FAR *)(lpcbxi)))
#define ComboBoxEx_GetComboControl(hwndCtl)		((HWND)SNDMSG((hwndCtl), CBEM_GETCOMBOCONTROL, 0, 0L))
#define ComboBoxEx_GetEditControl(hwndCtl)		((HWND)SNDMSG((hwndCtl), CBEM_GETEDITCONTROL, 0, 0L))

#endif /* __COMBOBOXEX_H */
