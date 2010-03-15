/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : font.h
 * Created    : not known
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:05:23
 * Comments   : 
 *              
 *              
 *****************************************************************/


#ifndef __FONT_H
#define __FONT_H

BOOL Font_ChooseFont(void);
void Font_InitializeFirst(void);
void Font_Initialize(HWND hwndEdit);
void Font_SetFont(void);
void Font_Deinitialize(void);
HFONT Font_GetFont(void);

#endif /* FONT_H */
