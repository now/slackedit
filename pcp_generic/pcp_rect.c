/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_rect.c
 * Created    : not known (before 01/03/00)
 * Owner      : pcppopper
 * Revised on : 06/27/00 18:23:39
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"
#include "pcp_rect.h"

__inline int Rect_Height(LPRECT lprc)
{
    return (lprc->bottom - lprc->top);
}

__inline int Rect_Width(LPRECT lprc)
{
    return (lprc->right - lprc->left);
}
