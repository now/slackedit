/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_controls
 *
 * File       : pcp_paint.h
 * Created    : 06/27/00
 * Owner      : pcppopper
 * Revised on : 06/28/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#ifndef __PCP_PAINT_H
#define __PCP_PAINT_H

#include "pcp_paint_dll.h"

#define HDIB HANDLE
#define PALVERSION  0x300
#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER)) 
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4) 

FOREXPORT void Paint_Draw3DRect(HDC hdc, LPRECT rc, COLORREF crHilite, COLORREF crShadow, int iPenWidth);
FOREXPORT void Paint_DrawCheckMark(HDC hdc, LPRECT lprc, COLORREF cr);
FOREXPORT void Paint_DrawRadioDot(HDC hdc, LPRECT lprc, COLORREF cr);

FOREXPORT HBITMAP Paint_Bitmap_GrayScale(HBITMAP hbmSrc, double luminace, COLORREF *lpcr);
FOREXPORT HBITMAP Paint_Bitmap_CreateEmpty(HDC hdc, int cx, int cy, int nColors);
FOREXPORT HBITMAP Paint_Bitmap_LoadSysColorBitmap(HINSTANCE hInst, LPCTSTR pszResource, BOOL bMono);
FOREXPORT HBITMAP Paint_Bitmap_CopyBitmap(HBITMAP hbm);
FOREXPORT HBITMAP Paint_Bitmap_CropBitmap(HDC hdc, HBITMAP hbm, PRECT prc);
FOREXPORT HBITMAP Paint_Bitmap_FlipBitmap(HDC hdcSrc, HBITMAP hbmSrc);

FOREXPORT BOOL Paint_DitherBlt(HDC hdc, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc);

FOREXPORT void Paint_FillRect(HDC hdc, const LPRECT lprc, COLORREF cr);

FOREXPORT HBRUSH Paint_Brush_GetHalftoneBrush(void);


#endif /* __PCP_PAINT_H */
