/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : se_mdi_client.h
 * Created    : not known (before 12/06/99)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:16:58
 * Comments   : 
 *              
 *              
 *****************************************************************/


#ifndef __MDICLIENT_H
#define __MDICLIENT_H

HWND MDI_Client_Create(HWND hwndParent);

BOOL MDI_Client_DestroyChildren(void);

void MDI_Client_ResizeChildren(HWND hwnd);

void MDI_Client_UpdateWindowMenu(HMENU hMenu);
void MDI_Client_ActivateChild(UINT uID);


#endif /* __MDICLIENT_H */
