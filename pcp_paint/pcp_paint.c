/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_paint
 *
 * File       : pcp_paint.c
 * Created    : 06/27/00
 * Owner      : pcppopper
 * Revised on : 07/01/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* standard */
#include <commctrl.h>
#include <math.h>

/* pcp_paint */
#include "pcp_paint.h"

/* pcp_generic */
#include <pcp_rect.h>
#include <pcp_mem.h>

#define RGB_TO_RGBQUAD(r,g,b)   (RGB(b,g,r))
#define CLR_TO_RGBQUAD(clr)     (RGB(GetBValue(clr),GetGValue(clr),GetRValue(clr)))

// From Petzold's Book
#define BB_MAGIC 0xB8074A

// Used in Paint_DitherBlt
typedef struct tagBWBITMAPINFO
{
    BITMAPINFOHEADER bmiHeader; 
    RGBQUAD          bmiColors[2]; 
} BWBITMAPINFO, *LPBWBITMAPINFO;

typedef struct tagSysColorMap
{
  DWORD rgbqFrom;
  int   iSysColorTo;
} SYSCOLORMAP;

static const SYSCOLORMAP sysColorMap[] =
{
  {RGB_TO_RGBQUAD(0x00, 0x00, 0x00), COLOR_BTNTEXT},    // black
  {RGB_TO_RGBQUAD(0x80, 0x80, 0x80), COLOR_BTNSHADOW},  // dark gray
  {RGB_TO_RGBQUAD(0xC0, 0xC0, 0xC0), COLOR_BTNFACE},    // bright gray
  {RGB_TO_RGBQUAD(0xFF, 0xFF, 0xFF), COLOR_BTNHIGHLIGHT}// white
};

__inline int Paint_RgbToGray3( int r, int g, int b, double luminance);
WORD Paint_PaletteSize(LPTSTR lpDIB);
WORD Paint_DIBNumColors(LPTSTR lpDIB);
HDIB Paint_DDBToDIB(HBITMAP hBitmap, HPALETTE hPal);
HPALETTE Paint_GetSystemPalette(void);
int Paint_PalEntriesOnDevice(HDC hDC);
void Paint_DrawResizeLine(HWND hwnd, int pos, int prevpos, LPRECT rcDivider, int mode);

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

void Paint_Draw3DRect(HDC hdc, LPRECT rc, COLORREF crHilite, COLORREF crShadow, int iPenWidth)
{
    HPEN hPenHilite, hPenShadow, hPenOld;

    hPenHilite  = CreatePen(PS_SOLID, iPenWidth, crHilite);

    hPenShadow  = CreatePen(PS_SOLID, iPenWidth, crShadow);
    hPenOld     = SelectObject(hdc, hPenHilite);

    MoveToEx(hdc, rc->left, (rc->bottom - 1), NULL);
    LineTo(hdc, rc->left, rc->top);
    LineTo(hdc, (rc->right - 1), rc->top);

    SelectObject(hdc, hPenShadow);

    LineTo(hdc, (rc->right - 1), (rc->bottom - 1));
    LineTo(hdc, rc->left - 1, (rc->bottom - 1));

    SelectObject(hdc, hPenOld);

    DeleteObject(hPenHilite);
    DeleteObject(hPenShadow);
}

void Paint_DrawCheckMark(HDC hdc, LPRECT lprc, COLORREF cr)
{
    int x, y;

    x = lprc->left + (Rect_Width(lprc) / 2 - 4);
    y = lprc->top + (Rect_Height(lprc) / 2 - 4);

    SetPixel(hdc, x, y + 2, cr);
    SetPixel(hdc, x, y + 3, cr);
    SetPixel(hdc, x, y + 4, cr);

    SetPixel(hdc, x + 1, y + 3, cr);
    SetPixel(hdc, x + 1, y + 4, cr);
    SetPixel(hdc, x + 1, y + 5, cr);

    SetPixel(hdc, x + 2, y + 4, cr);
    SetPixel(hdc, x + 2, y + 5, cr);
    SetPixel(hdc, x + 2, y + 6, cr);

    SetPixel(hdc, x + 3, y + 3, cr);
    SetPixel(hdc, x + 3, y + 4, cr);
    SetPixel(hdc, x + 3, y + 5, cr);

    SetPixel(hdc, x + 4, y + 2, cr);
    SetPixel(hdc, x + 4, y + 3, cr);
    SetPixel(hdc, x + 4, y + 4, cr);

    SetPixel(hdc, x + 5, y + 1, cr);
    SetPixel(hdc, x + 5, y + 2, cr);
    SetPixel(hdc, x + 5, y + 3, cr);

    SetPixel(hdc, x + 6, y, cr);
    SetPixel(hdc, x + 6, y + 1, cr);
    SetPixel(hdc, x + 6, y + 2, cr);
}

void Paint_DrawRadioDot(HDC hdc, LPRECT lprc, COLORREF cr)
{
    RECT rcDot;
    HBRUSH hBrush, hBrushOld;
    HPEN hPen, hPenOld;

    CopyRect(&rcDot, lprc);
    rcDot.left      = rcDot.left + (Rect_Width(lprc) / 2 - 3);
    rcDot.top       = rcDot.top + (Rect_Width(lprc) / 2 - 3);
    rcDot.right     = rcDot.left + 6;
    rcDot.bottom    = rcDot.top + 6;

    hBrush  = CreateSolidBrush(cr);
    hPen    = CreatePen(PS_SOLID, 1, cr);

    hBrushOld   = SelectObject(hdc, hBrush);
    hPenOld     = SelectObject(hdc, hPen);

    Ellipse(hdc, rcDot.left, rcDot.top, rcDot.right, rcDot.bottom);

    DeleteObject(SelectObject(hdc, hBrushOld));
    DeleteObject(SelectObject(hdc, hPenOld));
}

HDIB Paint_DDBToDIB(HBITMAP hBitmap, HPALETTE hPal)
{
    BITMAP              bm;
    BITMAPINFOHEADER    bi;
    LPBITMAPINFOHEADER  lpbi;
    DWORD               dwLen;
    HDIB                hDIB;
    HDC                 hdc;
    WORD                biBits;
 
    if (hBitmap == NULL) 
        return (NULL); 
 
    if (!GetObject(hBitmap, sizeof(bm), &bm)) 
        return (NULL); 
 
    // if no palette is specified, use default palette 
    if (hPal == NULL) 
        hPal = GetStockObject(DEFAULT_PALETTE); 
 
    // calculate bits per pixel 
    biBits = (unsigned short)(bm.bmPlanes * bm.bmBitsPixel);
 
    // make sure bits per pixel is valid 
 
    if (biBits <= 1) 
        biBits = 1; 
    else if (biBits <= 4) 
        biBits = 4; 
    else if (biBits <= 8) 
        biBits = 8; 
    else // if greater than 8-bit, force to 24-bit 
        biBits = 24; 
 
    // initialize BITMAPINFOHEADER 
 
    bi.biSize = sizeof(BITMAPINFOHEADER); 
    bi.biWidth = bm.bmWidth; 
    bi.biHeight = bm.bmHeight; 
    bi.biPlanes = 1; 
    bi.biBitCount = biBits; 
    bi.biCompression = BI_RGB; 
    bi.biSizeImage = 0; 
    bi.biXPelsPerMeter = 0; 
    bi.biYPelsPerMeter = 0; 
    bi.biClrUsed = 0; 
    bi.biClrImportant = 0; 
 
    // calculate size of memory block required to store BITMAPINFO 
 
    dwLen = bi.biSize + Paint_PaletteSize((LPTSTR)&bi); 
 
    // get a DC 
 
    hdc = GetDC(NULL); 
 
    // select and realize our palette 
 
    hPal = SelectPalette(hdc, hPal, FALSE); 
    RealizePalette(hdc); 
 
    // alloc memory block to store our bitmap 
 
    hDIB = Mem_Alloc(dwLen); 
 
    // if we couldn't get memory block 
 
    if (hDIB == NULL)
    { 
      // clean up and return NULL 
 
      SelectPalette(hdc, hPal, TRUE); 
      RealizePalette(hdc); 
      ReleaseDC(NULL, hdc); 

      return (NULL); 
    } 
 
    // lock memory and get pointer to it 
 
    lpbi = (LPBITMAPINFOHEADER)hDIB; 

    /// use our bitmap info. to fill BITMAPINFOHEADER 
 
    *lpbi = bi; 
 
    // call GetDIBits with a NULL lpBits param, so it will calculate the 
    // biSizeImage field for us     
 
    GetDIBits(hdc, hBitmap, 0, (UINT)bi.biHeight, NULL, (LPBITMAPINFO)lpbi, 
        DIB_RGB_COLORS); 
 
    // get the info. returned by GetDIBits and unlock memory block 
 
    bi = *lpbi; 
 
    // if the driver did not fill in the biSizeImage field, make one up  
    if (bi.biSizeImage == 0) 
        bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight; 
 
    // realloc the buffer big enough to hold all the bits 
 
    dwLen = bi.biSize + Paint_PaletteSize((LPTSTR)&bi) + bi.biSizeImage; 
 
    if ((hDIB = Mem_ReAlloc(hDIB, dwLen)) == NULL)
    { 
        // clean up and return NULL 
 
        Mem_Free(hDIB); 
        hDIB = NULL; 
        SelectPalette(hdc, hPal, TRUE); 
        RealizePalette(hdc); 
        ReleaseDC(NULL, hdc);
        
        return (NULL); 
    } 
 
    // lock memory block and get pointer to it */ 
 
    lpbi = (LPBITMAPINFOHEADER)hDIB; 
 
    // call GetDIBits with a NON-NULL lpBits param, and actualy get the 
    // bits this time 
 
    if (GetDIBits(hdc, hBitmap, 0, (UINT)bi.biHeight, (LPTSTR)lpbi + 
            (WORD)lpbi->biSize + Paint_PaletteSize((LPTSTR)lpbi), (LPBITMAPINFO)lpbi, 
            DIB_RGB_COLORS) == 0) 
    { 
        // clean up and return NULL 
 
        Mem_Free(hDIB); 
        hDIB = NULL; 
        SelectPalette(hdc, hPal, TRUE); 
        RealizePalette(hdc); 
        ReleaseDC(NULL, hdc);
        
        return (NULL); 
    } 
 
    bi = *lpbi; 
 
    // clean up  
    SelectPalette(hdc, hPal, TRUE); 
    RealizePalette(hdc); 
    ReleaseDC(NULL, hdc); 

    // return handle to the DIB 
    return (hDIB); 
}

WORD Paint_PaletteSize(LPTSTR lpDIB) 
{
    // calculate the size required by the palette 
    if (IS_WIN30_DIB (lpDIB)) 
        return (unsigned short)(Paint_DIBNumColors(lpDIB) * sizeof(RGBQUAD)); 
    else
        return (unsigned short)(Paint_DIBNumColors(lpDIB) * sizeof(RGBTRIPLE)); 
}

HPALETTE Paint_GetSystemPalette(void) 
{ 
    HDC hdc;
    static HPALETTE hPalette = NULL;
    LPLOGPALETTE lpLogPalette;
    int iColors;

    // Find out how many palette entries we want.
    hdc = GetDC(NULL);

    if (hdc == NULL)
        return (NULL);

    // Number of palette entries
    iColors = Paint_PalEntriesOnDevice(hdc);

    // Allocate room for the palette and lock it. 
    lpLogPalette = Mem_Alloc(sizeof(LOGPALETTE) + (iColors * sizeof(PALETTEENTRY)));
 
    // if we didn't get a logical palette, return NULL
    if (lpLogPalette == NULL)
        return (NULL);
 
    // set some important fields
    lpLogPalette->palVersion    = (unsigned short)PALVERSION;
    lpLogPalette->palNumEntries = (unsigned short)iColors;

    // Copy the current system palette into our logical palette 
    GetSystemPaletteEntries(hdc, 0, iColors, (LPPALETTEENTRY)(lpLogPalette->palPalEntry));
 
    // Go ahead and create the palette.  Once it's created,
    // we no longer need the LOGPALETTE, so free it.
    hPalette = CreatePalette(lpLogPalette);
 
    // clean up
    Mem_Free(lpLogPalette);
    ReleaseDC(NULL, hdc);

    return (hPalette);
}

WORD Paint_DIBNumColors(LPTSTR lpDIB) 
{
    WORD wBitCount;
  
    if (IS_WIN30_DIB(lpDIB)) 
    { 
        DWORD dwClrUsed; 
 
        dwClrUsed = ((LPBITMAPINFOHEADER)lpDIB)->biClrUsed; 
        if (dwClrUsed) 
 
        return ((WORD)dwClrUsed); 
    } 
 
    // Calculate the number of colors in the color table based on 
    // the number of bits per pixel for the DIB. 
     
    if (IS_WIN30_DIB(lpDIB)) 
        wBitCount = ((LPBITMAPINFOHEADER)lpDIB)->biBitCount; 
    else 
        wBitCount = ((LPBITMAPCOREHEADER)lpDIB)->bcBitCount; 
 
    // return number of colors based on bits per pixel 
 
    switch (wBitCount) 
    { 
        case 1: 
            return (2); 
 
        case 4: 
            return (16); 
 
        case 8: 
            return (256); 
 
        default: 
            return (0); 
    } 
} 

HBITMAP Paint_Bitmap_GrayScale(HBITMAP hbmSrc, double luminace, COLORREF *lpcr)
{
    HBITMAP         hbmNew;
    HDIB            hDib;
    LPBITMAPINFO    lpBitmapInfo;
    LPVOID          lpDIBBits;
    int             nColors, nWidth, nHeight;
    int             i;
    BOOL            bNewTransparentSet = FALSE;

    if (luminace == -1)
        luminace = 2.5;

    hDib = Paint_DDBToDIB(hbmSrc, Paint_GetSystemPalette());

    if (!hDib)
        return (NULL);

    lpBitmapInfo    = (LPBITMAPINFO)hDib;
    nColors         = lpBitmapInfo->bmiHeader.biClrUsed ? lpBitmapInfo->bmiHeader.biClrUsed : 1 << lpBitmapInfo->bmiHeader.biBitCount;

    if (lpBitmapInfo->bmiHeader.biBitCount > 8)
        lpDIBBits = (LPVOID)((LPDWORD)(lpBitmapInfo->bmiColors + lpBitmapInfo->bmiHeader.biClrUsed) + 
                                    ((lpBitmapInfo->bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0));
    else
        lpDIBBits = (LPVOID)(lpBitmapInfo->bmiColors + nColors);

    nWidth  = lpBitmapInfo->bmiHeader.biWidth;
    nHeight = lpBitmapInfo->bmiHeader.biHeight;

    if (nColors <= 256)
    {
        for (i = 0; i < nColors; i++)
        {
/*          long lSquareSum = lpBitmapInfo->bmiColors[i].rgbRed
                            * lpBitmapInfo->bmiColors[i].rgbRed + lpBitmapInfo->bmiColors[i].rgbGreen
                            * lpBitmapInfo->bmiColors[i].rgbGreen + lpBitmapInfo->bmiColors[i].rgbBlue
                            * lpBitmapInfo->bmiColors[i].rgbBlue;
*/
            if (lpBitmapInfo->bmiColors[i].rgbRed   != lpBitmapInfo->bmiColors[i].rgbGreen ||
                lpBitmapInfo->bmiColors[i].rgbGreen != lpBitmapInfo->bmiColors[i].rgbBlue  ||
                lpBitmapInfo->bmiColors[i].rgbRed   != lpBitmapInfo->bmiColors[i].rgbBlue)
            {
                int nGray = Paint_RgbToGray3(lpBitmapInfo->bmiColors[i].rgbRed,lpBitmapInfo->bmiColors[i].rgbGreen, lpBitmapInfo->bmiColors[i].rgbBlue, luminace);

                if (lpcr != NULL && !bNewTransparentSet)
                {
                    if (RGB(lpBitmapInfo->bmiColors[i].rgbRed,
                            lpBitmapInfo->bmiColors[i].rgbGreen,
                            lpBitmapInfo->bmiColors[i].rgbBlue) == *lpcr)
                    {
                        *lpcr = RGB(nGray, nGray, nGray);
                        bNewTransparentSet = TRUE;
                    }
                }

                lpBitmapInfo->bmiColors[i].rgbRed   = (unsigned char)nGray;
                lpBitmapInfo->bmiColors[i].rgbGreen = (unsigned char)nGray;
                lpBitmapInfo->bmiColors[i].rgbBlue  = (unsigned char)nGray;
            }
        }
    }
    else
    {
        if (lpBitmapInfo->bmiHeader.biBitCount == 24)
        {
            BYTE* dst  = (BYTE*)lpDIBBits;
            int Size = nWidth * nHeight;

            while (Size--)
            {
                BYTE r   = dst[2];
                BYTE g   = dst[1];
                BYTE b   = dst[0];
                int  nGray = Paint_RgbToGray3(r, g, b, luminace);

                if (lpcr != NULL && !bNewTransparentSet)
                {
                    if (RGB(r, g, b) == *lpcr)
                    {
                        *lpcr = RGB(nGray, nGray, nGray);
                        bNewTransparentSet = TRUE;
                    }
                }

                dst[0]  = (BYTE)nGray;
                dst[1]  = (BYTE)nGray;
                dst[2]  = (BYTE)nGray;
                dst     += 3;
            }
        }
        else if (lpBitmapInfo->bmiHeader.biBitCount == 16 )
        {
            WORD *dst = (WORD*)lpDIBBits;
            int  Size = nWidth * nHeight;

            while (Size--)
            {
                BYTE b   = (BYTE)((*dst)        & (0x1F));
                BYTE g   = (BYTE)(((*dst) >> 5) & (0x1F));
                BYTE r   = (BYTE)(((*dst) >>10) & (0x1F));
                int  nGray = Paint_RgbToGray3(r,g,b,luminace);

                if (lpcr != NULL && !bNewTransparentSet)
                {
                    if (RGB(r, g, b) == *lpcr)
                    {
                        *lpcr = RGB(nGray, nGray, nGray);
                        bNewTransparentSet = TRUE;
                    }
                }

                *dst++ = ((WORD)(((BYTE)(nGray)|((WORD)((BYTE)(nGray))<<5))|(((DWORD)(BYTE)(nGray))<<10)));
            }
        }
    }

    hbmNew  = Paint_Bitmap_CreateEmpty(NULL, nWidth, nHeight, lpBitmapInfo->bmiHeader.biBitCount);

    if (hbmNew != NULL)
    {
        HDC hdc = GetDC(NULL);

        SetDIBits(hdc, hbmNew, (DWORD) 0, (DWORD) nHeight, lpDIBBits, (LPBITMAPINFO) lpBitmapInfo, (DWORD) DIB_RGB_COLORS);

        ReleaseDC(NULL, hdc);
    }

    Mem_Free(hDib);

    return (hbmNew);
}

__inline int Paint_RgbToGray3( int r, int g, int b, double luminance)
{
    long lSquareSum = r * r + g * g + b * b;
    return ((int)sqrt(((double)lSquareSum) / (double)luminance));
}

HBITMAP Paint_Bitmap_CreateEmpty(HDC hdc, int cx, int cy, int nColors)
{
    if (nColors == 2)
    {
        return (CreateBitmap(cx, cy, 1, 1, NULL));
    }
    else
    {
        BITMAPINFOHEADER bmih;
        HBITMAP hEmptyBitmap;

        bmih.biSize          = sizeof(BITMAPINFOHEADER);
        bmih.biWidth         = cx;
        bmih.biHeight        = cy;
        bmih.biPlanes        = 1;
        bmih.biBitCount      = (unsigned short)nColors;
        bmih.biCompression   = 0;   //BI_RGB;
        bmih.biSizeImage     = 0;
        bmih.biXPelsPerMeter = 0;
        bmih.biYPelsPerMeter = 0;
        bmih.biClrUsed       = 0;
        bmih.biClrImportant  = 0;

        if (hdc == NULL)
            hdc = GetDC(NULL);
        
        hEmptyBitmap = CreateDIBitmap(hdc, &bmih, 0L, NULL, NULL, DIB_RGB_COLORS);

        ReleaseDC(NULL, hdc);

        return (hEmptyBitmap);
    }
}

int Paint_PalEntriesOnDevice(HDC hDC) 
{ 
    int nColors;  // number of colors 
 
    // Find out the number of colors on this device. 
     
    nColors = (1 << (GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES))); 
 
    return (nColors); 
}

void Paint_FillRect(HDC hdc, const LPRECT lprc, COLORREF cr)
{
    HBRUSH hBrush = CreateSolidBrush(cr);
    HBRUSH hbOld  = (HBRUSH)SelectObject(hdc, hBrush);
    PatBlt(hdc, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top, PATCOPY);
    SelectObject(hdc, hbOld);
    DeleteObject(hBrush);
    return;
}

HBITMAP Paint_Bitmap_LoadSysColorBitmap(HINSTANCE hInst, LPCTSTR pszResource, BOOL bMono)
{
    HGLOBAL             hglb;
    DWORD               *pColorTable;
    const int           nColorTableSize = 16;
    UINT                nSize;
    int                 nWidth, nHeight, i, iColor;
    HDC                 hDCScreen;
    LPBITMAPINFOHEADER  lpBitmap;
    LPBITMAPINFOHEADER  lpBitmapInfo;
    HBITMAP             hbm;
    HRSRC               hRsrc;


    if ((hRsrc = FindResource(hInst, pszResource, RT_BITMAP)) == NULL)
        return (NULL);

    if ((hglb = LoadResource(hInst, hRsrc)) == NULL)
        return (NULL);

    lpBitmap = (LPBITMAPINFOHEADER)LockResource(hglb);

    if (lpBitmap == NULL)
        return (NULL);

    nSize = lpBitmap->biSize + nColorTableSize * sizeof(RGBQUAD);
    lpBitmapInfo = (LPBITMAPINFOHEADER)Mem_Alloc(nSize);

    memcpy(lpBitmapInfo, lpBitmap, nSize);

    pColorTable = (DWORD *)(((LPBYTE)lpBitmapInfo) + (UINT)lpBitmapInfo->biSize);

    for (iColor = 0; iColor < nColorTableSize; iColor++)
    {
        for (i = 0; i < sizeof(sysColorMap) / sizeof(SYSCOLORMAP); i++)
        {
            if (pColorTable[iColor] == sysColorMap[i].rgbqFrom)
            {
                if (bMono)
                {
                    if (sysColorMap[i].iSysColorTo != COLOR_BTNTEXT)
                        pColorTable[iColor] = RGB_TO_RGBQUAD(255, 255, 255);
                }
                else
                    pColorTable[iColor] = CLR_TO_RGBQUAD(GetSysColor(sysColorMap[i].iSysColorTo));
                break;
            }
        }
    }

    nWidth = (int)lpBitmapInfo->biWidth;
    nHeight = (int)lpBitmapInfo->biHeight;
    hDCScreen = GetDC(NULL);
    hbm = CreateCompatibleBitmap(hDCScreen, nWidth, nHeight);

    if (hbm != NULL)
    {
        HDC hDCGlyfs = CreateCompatibleDC(hDCScreen);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hDCGlyfs, hbm);
        LPBYTE lpBits;

        lpBits = (LPBYTE) (lpBitmap + 1);
        lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

        StretchDIBits(hDCGlyfs, 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight,
                lpBits, (LPBITMAPINFO) lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);

        SelectObject(hDCGlyfs, hbmOld);
        DeleteDC(hDCGlyfs);
    }

    ReleaseDC(NULL, hDCScreen);
    Mem_Free(lpBitmapInfo);
    FreeResource(hglb);

    return (hbm);
}

void Paint_DrawResizeLine(HWND hwnd, int pos, int prevpos, LPRECT rcDivider, int mode)
{
    HPEN hPen, hPenOld;     // pen for line and previous pen
    HDC  hdc;           // window's HDC
    RECT rc;            // window client rectangle

    UNREFERENCED_PARAMETER(prevpos);

    hPen = CreatePen(PS_DASH, 3, RGB(50, 50, 50));
    GetClientRect(hwnd, &rc);

    hdc     = GetDC(hwnd);             // get the window's DC
    hPenOld = (HPEN)SelectObject(hdc, hPen); // use the pen we just created
    SetROP2(hdc, R2_NOT);                  // just invert the pixels

    if (mode == 1)
    {
        MoveToEx(hdc, pos, rcDivider->top + 3, NULL);
        LineTo(hdc, pos, rcDivider->bottom - 3);
    }
    else
    {
        MoveToEx(hdc, rcDivider->left + 3, pos, NULL);
        LineTo(hdc, rcDivider->right - 3, pos);
    }

    SelectObject(hdc, hPenOld); // put the old pen back
    SetROP2(hdc, R2_NOP);       // reset the ROP2 code
    ReleaseDC(hwnd, hdc);
    DeleteObject(hPen);     // delete the pen
    return;
}

/***************************************************************
 *                                                             *
 *  FUNCTION   : CopyBitmap (HBITMAP hbm)                      *
 *                                                             *
 *  PURPOSE    : Copies the given bitmap to another.           *
 *                                                             *
 *  RETURNS    : A handle to the new bitmap.                   *
 *                                                             *
 ***************************************************************/

HBITMAP Paint_Bitmap_CopyBitmap(HBITMAP hbm)
{
  BITMAP bm;
  RECT rc;

  if (hbm == NULL)
    return (NULL);

  GetObject(hbm, sizeof(BITMAP), &bm);
  rc.left = 0;
  rc.top = 0;
  rc.right = bm.bmWidth;
  rc.bottom = bm.bmHeight;
  return (Paint_Bitmap_CropBitmap(NULL, hbm, &rc));
}

HBITMAP Paint_Bitmap_FlipBitmap(HDC hdc, HBITMAP hbmSrc)
{
    LONG iSrcX, iSrcY;
    LONG iDstX, iDstY;
    BITMAP bm;
    HDC hdcSrc, hdcDst, hdcTemp = NULL;
    HBITMAP hbmDst;

    if (hbmSrc == NULL)
        return (NULL);

    if (hdc == NULL)
        hdcTemp = GetDC(NULL);
    else
        hdcTemp = hdc;

    hdcDst = CreateCompatibleDC(hdcTemp);
    hdcSrc = CreateCompatibleDC(hdcTemp);

    GetObject(hbmSrc, sizeof(BITMAP), &bm);

    hbmDst = Paint_Bitmap_CreateEmpty(hdc/*Temp*/, bm.bmWidth, bm.bmHeight, bm.bmBitsPixel);

    if (hbmDst != NULL)
    {
        SelectObject(hdcSrc, hbmSrc);
        SelectObject(hdcDst, hbmDst);


        for (iSrcY = bm.bmHeight, iDstY = 0; iSrcY >= 0; iSrcY--, iDstY++)
        {
            for (iSrcX = bm.bmWidth, iDstX = 0; iSrcX >= 0; iSrcX--, iDstX++)
            {
                COLORREF clPixel = GetPixel(hdcSrc, iSrcX, iSrcY);

                SetPixel(hdcDst, iDstX, iDstY, clPixel);
            }
        }
    }

    if (hdcTemp != NULL)
        ReleaseDC(NULL, hdcTemp);

    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);

    return (hbmDst);
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   :  CropBitmap (hbm,lprect)                                   *
 *                                                                          *
 *  PURPOSE    :  Crops a bitmap to a new size specified by the lprect      *
 *                parameter.                                                *
 *                                                                          *
 *  RETURNS    :  A handle to the new bitmap.                               *
 *                                                                          *
 ****************************************************************************/

HBITMAP Paint_Bitmap_CropBitmap(HDC hdc, HBITMAP hbm, PRECT prc)
{
    HDC hMemDCsrc;
    HDC hMemDCdst;
    HDC hdcTmp;
    HBITMAP hNewBm;
    BITMAP bm;
    INT dx, dy;
    
    if (!hbm)
        return (NULL);
    
    if (hdc == NULL)
        hdcTmp = GetDC(NULL);
    else
        hdcTmp = hdc;
    
    hMemDCsrc = CreateCompatibleDC(hdcTmp);
    hMemDCdst = CreateCompatibleDC(hdcTmp);
    
    GetObject(hbm, sizeof(BITMAP), &bm);
    dx = prc->right - prc->left;
    dy = prc->bottom - prc->top;
    hNewBm = Paint_Bitmap_CreateEmpty(hdc, dx, dy, bm.bmBitsPixel);
    GetObject(hNewBm, sizeof(BITMAP), (LPTSTR) & bm);
    
    if (hNewBm)
    {
        SelectObject(hMemDCsrc, hbm);
        SelectObject(hMemDCdst, hNewBm);
        BitBlt(hMemDCdst, 0, 0, dx, dy, hMemDCsrc, prc->left, prc->top, SRCCOPY);
    }
    
    if (hdc == NULL)
        ReleaseDC(NULL, hdcTmp);
    
    DeleteDC(hMemDCsrc);
    DeleteDC(hMemDCdst);
    
    return (hNewBm);
}

BOOL Paint_DitherBlt(HDC hdc, int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc)
{
    HDC hdcDIB;
    BWBITMAPINFO bwBitmapInfo =
    {
        { sizeof(BITMAPINFOHEADER), nWidth, nHeight, 1, 1, BI_RGB, 0, 0, 0, 0, 0 },
        { { 0x00, 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0xFF, 0x00 } }
    };
    HBITMAP hbmBWDIB;
    HBITMAP hOldBWBitmap;
    HBITMAP hOldBitmap = NULL;
    LPVOID lpBWBits;
    RECT rc = { nXDest, nYDest, nXDest + nWidth, nYDest + nHeight };
    HBRUSH hHilightBrush;
    HBRUSH hOldHilightBrush;
    HBRUSH hShadowBrush;

    ASSERT(hdc != NULL && hdcSrc != NULL);
    ASSERT(nWidth > 0 && nHeight > 0);

    hdcDIB = CreateCompatibleDC(hdcSrc);

    if (hdcDIB == NULL)
        return (FALSE);

    hbmBWDIB = CreateDIBSection(hdcDIB, (LPBITMAPINFO)&bwBitmapInfo, DIB_RGB_COLORS, &lpBWBits, NULL, 0);

    ASSERT(hbmBWDIB != NULL);

    if (hbmBWDIB == NULL)
    {
        if (hdcDIB != NULL)
            DeleteDC(hdcDIB);

        return (FALSE);
    }

    hOldBWBitmap = SelectObject(hdcDIB, hbmBWDIB);

    // From WTL (by microsoft)
    // Block: Dark gray removal: we want (128, 128, 128) pixels to become black and not white
    {
        HDC hdcTemp1 = CreateCompatibleDC(hdcSrc);
        HDC hdcTemp2 = CreateCompatibleDC(hdcSrc);
        HBITMAP hbmTemp1 = CreateCompatibleBitmap(hdcSrc, nWidth, nHeight);
        HBITMAP hbmTemp2 = CreateBitmap(nWidth, nHeight, 1, 1, NULL);
        HBITMAP hOldBitmap1 = SelectObject(hdcTemp1, hbmTemp1);
        HBITMAP hOldBitmap2 = SelectObject(hdcTemp2, hbmTemp2);

        // Let's copy our image, it will be altered
        BitBlt(hdcTemp1, 0, 0, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, SRCCOPY);

        // All dark gray pixels will become white, the others black
        SetBkColor(hdcTemp1, RGB(128, 128, 128));
        BitBlt(hdcTemp2, 0, 0, nWidth, nHeight, hdcTemp1, 0, 0, SRCCOPY);
        // Do an XOR to set to black these white pixels
        BitBlt(hdcTemp1, 0, 0, nWidth, nHeight, hdcTemp2, 0, 0, SRCINVERT);

        // BitBlt the bitmap into the monochrome DIB section
        // The DIB section will do a true monochrome conversion
        // The magenta background being closer to white will become white
        BitBlt(hdcDIB, 0, 0, nWidth, nHeight, hdcTemp1, 0, 0, SRCCOPY);

        // Cleanup
        DeleteObject(SelectObject(hdcTemp1, hOldBitmap1));
        DeleteObject(SelectObject(hdcTemp2, hOldBitmap2));
        DeleteDC(hdcTemp1);
        DeleteDC(hdcTemp2);
    }
    
    // Paint the destination rectangle in gray
    FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));

    // BitBlt the black bits in the monochrome bitmap into COLOR_3DHILIGHT bits in the destination DC
    // The magic ROP comes from the Charles Petzold's book
    hHilightBrush = CreateSolidBrush(GetSysColor(COLOR_3DHILIGHT));
    hOldHilightBrush = SelectObject(hdc, hHilightBrush);
    BitBlt(hdc, nXDest + 1, nYDest + 1, nWidth, nHeight, hdcDIB, 0, 0, BB_MAGIC);

    // BitBlt the black bits in the monochrome bitmap into COLOR_3DSHADOW bits in the destination DC
    hShadowBrush = CreateSolidBrush(GetSysColor(COLOR_3DSHADOW));
    DeleteObject(SelectObject(hdc, hShadowBrush));
    BitBlt(hdc, nXDest, nYDest, nWidth, nHeight, hdcDIB, 0, 0, BB_MAGIC);

    DeleteObject(SelectObject(hdc, hOldHilightBrush));
    DeleteObject(SelectObject(hdcDIB, hOldBWBitmap));
    DeleteDC(hdcDIB);

    return (TRUE);
}

// more or less ripped from microsoft WTL source
HBRUSH Paint_Brush_GetHalftoneBrush(void)
{
    HBRUSH hHalftoneBrush = NULL;
    WORD wGrayPattern[8];
    int i;
    HBITMAP hGrayBitmap;

    for (i = 0; i < 8; i++)
        wGrayPattern[i] = (WORD)(0x5555 << (i & 1));

    hGrayBitmap = CreateBitmap(8, 8, 1, 1, &wGrayPattern);

    if (hGrayBitmap != NULL)
    {
        hHalftoneBrush = CreatePatternBrush(hGrayBitmap);
        DeleteObject(hGrayBitmap);
    }

    return (hHalftoneBrush);
}
