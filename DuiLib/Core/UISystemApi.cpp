#include "StdAfx.h"
#include "UISystemApi.h"


namespace DuiLib {

    typedef BOOL(__stdcall* PFUNCUPDATELAYEREDWINDOW)(HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD);
    PFUNCUPDATELAYEREDWINDOW g_fUpdateLayeredWindow1 = NULL;

    BOOL Dui_UpdateLayeredWindow(HWND hWnd, HDC hdcDst, POINT* pptDst, SIZE* psize,
        HDC hdcSrc, POINT* pptSrc, COLORREF crKey, BLENDFUNCTION* pblend, DWORD dwFlags)
    {
        if (g_fUpdateLayeredWindow1 == NULL)
        {
            HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
            if (hUser32)
            {
                g_fUpdateLayeredWindow1 = (PFUNCUPDATELAYEREDWINDOW)::GetProcAddress(hUser32, "UpdateLayeredWindow");
                if (g_fUpdateLayeredWindow1 == NULL)
                    return FALSE;
            }
        }

        return g_fUpdateLayeredWindow1(hWnd, hdcDst, pptDst, psize, hdcSrc, pptSrc, crKey, pblend, dwFlags);
    }

    typedef BOOL(__stdcall* PFUNCSETLAYEREDWINDOWATTR)(HWND, COLORREF, BYTE, DWORD);
    PFUNCSETLAYEREDWINDOWATTR g_fSetLayeredWindowAttributes = NULL;

    BOOL Dui_SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
    {
        if (g_fSetLayeredWindowAttributes == NULL)
        {
            HMODULE hUser32 = ::GetModuleHandle(_T("User32.dll"));
            if (hUser32)
            {
                g_fSetLayeredWindowAttributes = (PFUNCSETLAYEREDWINDOWATTR)::GetProcAddress(hUser32, "SetLayeredWindowAttributes");
                if (g_fSetLayeredWindowAttributes == NULL)
                    return FALSE;
            }
        }

        return g_fSetLayeredWindowAttributes(hwnd, crKey, bAlpha, dwFlags);
    }

    static COLORREF PixelAlpha(COLORREF clrSrc, double src_darken, COLORREF clrDest, double dest_darken)
    {
        return RGB(GetRValue(clrSrc) * src_darken + GetRValue(clrDest) * dest_darken,
            GetGValue(clrSrc) * src_darken + GetGValue(clrDest) * dest_darken,
            GetBValue(clrSrc) * src_darken + GetBValue(clrDest) * dest_darken);
    }

    static BOOL WINAPI AlphaBitBlt(HDC hDC, int nDestX, int nDestY, int dwWidth, int dwHeight, HDC hSrcDC, \
        int nSrcX, int nSrcY, int wSrc, int hSrc, BLENDFUNCTION ftn)
    {
        HDC hTempDC = ::CreateCompatibleDC(hDC);
        if (NULL == hTempDC)
            return FALSE;

        //Creates Source DIB
        LPBITMAPINFO lpbiSrc = NULL;
        // Fill in the BITMAPINFOHEADER
        lpbiSrc = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
        if (lpbiSrc == NULL)
        {
            ::DeleteDC(hTempDC);
            return FALSE;
        }
        lpbiSrc->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        lpbiSrc->bmiHeader.biWidth = dwWidth;
        lpbiSrc->bmiHeader.biHeight = dwHeight;
        lpbiSrc->bmiHeader.biPlanes = 1;
        lpbiSrc->bmiHeader.biBitCount = 32;
        lpbiSrc->bmiHeader.biCompression = BI_RGB;
        lpbiSrc->bmiHeader.biSizeImage = dwWidth * dwHeight;
        lpbiSrc->bmiHeader.biXPelsPerMeter = 0;
        lpbiSrc->bmiHeader.biYPelsPerMeter = 0;
        lpbiSrc->bmiHeader.biClrUsed = 0;
        lpbiSrc->bmiHeader.biClrImportant = 0;

        COLORREF* pSrcBits = NULL;
        HBITMAP hSrcDib = CreateDIBSection(
            hSrcDC, lpbiSrc, DIB_RGB_COLORS, (void**)&pSrcBits,
            NULL, NULL);

        if ((NULL == hSrcDib) || (NULL == pSrcBits))
        {
            delete[] lpbiSrc;
            ::DeleteDC(hTempDC);
            return FALSE;
        }

        HBITMAP hOldTempBmp = (HBITMAP)::SelectObject(hTempDC, hSrcDib);
        ::StretchBlt(hTempDC, 0, 0, dwWidth, dwHeight, hSrcDC, nSrcX, nSrcY, wSrc, hSrc, SRCCOPY);
        ::SelectObject(hTempDC, hOldTempBmp);

        //Creates Destination DIB
        LPBITMAPINFO lpbiDest = NULL;
        // Fill in the BITMAPINFOHEADER
        lpbiDest = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
        if (lpbiDest == NULL)
        {
            delete[] lpbiSrc;
            ::DeleteObject(hSrcDib);
            ::DeleteDC(hTempDC);
            return FALSE;
        }

        lpbiDest->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        lpbiDest->bmiHeader.biWidth = dwWidth;
        lpbiDest->bmiHeader.biHeight = dwHeight;
        lpbiDest->bmiHeader.biPlanes = 1;
        lpbiDest->bmiHeader.biBitCount = 32;
        lpbiDest->bmiHeader.biCompression = BI_RGB;
        lpbiDest->bmiHeader.biSizeImage = dwWidth * dwHeight;
        lpbiDest->bmiHeader.biXPelsPerMeter = 0;
        lpbiDest->bmiHeader.biYPelsPerMeter = 0;
        lpbiDest->bmiHeader.biClrUsed = 0;
        lpbiDest->bmiHeader.biClrImportant = 0;

        COLORREF* pDestBits = NULL;
        HBITMAP hDestDib = CreateDIBSection(
            hDC, lpbiDest, DIB_RGB_COLORS, (void**)&pDestBits,
            NULL, NULL);

        if ((NULL == hDestDib) || (NULL == pDestBits))
        {
            delete[] lpbiSrc;
            ::DeleteObject(hSrcDib);
            ::DeleteDC(hTempDC);
            return FALSE;
        }

        ::SelectObject(hTempDC, hDestDib);
        ::BitBlt(hTempDC, 0, 0, dwWidth, dwHeight, hDC, nDestX, nDestY, SRCCOPY);
        ::SelectObject(hTempDC, hOldTempBmp);

        double src_darken;
        BYTE nAlpha;

        for (int pixel = 0; pixel < dwWidth * dwHeight; pixel++, pSrcBits++, pDestBits++)
        {
            nAlpha = LOBYTE(*pSrcBits >> 24);
            src_darken = (double)(nAlpha * ftn.SourceConstantAlpha) / 255.0 / 255.0;
            if (src_darken < 0.0) src_darken = 0.0;
            *pDestBits = PixelAlpha(*pSrcBits, src_darken, *pDestBits, 1.0 - src_darken);
        } //for

        ::SelectObject(hTempDC, hDestDib);
        ::BitBlt(hDC, nDestX, nDestY, dwWidth, dwHeight, hTempDC, 0, 0, SRCCOPY);
        ::SelectObject(hTempDC, hOldTempBmp);

        delete[] lpbiDest;
        ::DeleteObject(hDestDib);

        delete[] lpbiSrc;
        ::DeleteObject(hSrcDib);

        ::DeleteDC(hTempDC);
        return TRUE;
    }

    typedef BOOL(WINAPI* PFUNCALPHABLEND)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
    PFUNCALPHABLEND g_fAlphaBlend = NULL;

    BOOL Dui_AlphaBlend(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest,
        HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn)
    {
        if (g_fAlphaBlend == NULL)
        {
            HMODULE hMsing32 = ::GetModuleHandle(_T("msimg32.dll"));
            if (hMsing32)
            {
                g_fAlphaBlend = (PFUNCALPHABLEND)::GetProcAddress(hMsing32, "AlphaBlend");
                if (g_fAlphaBlend == NULL)
                    g_fAlphaBlend = AlphaBitBlt; //使用本地 GDI 版本
            }
        }

        return g_fAlphaBlend(hdcDest, xoriginDest, yoriginDest, wDest, hDest,
            hdcSrc, xoriginSrc, yoriginSrc, wSrc, hSrc, ftn);
    }

    PFUNCGRADIENTFILL g_fGradientFill = NULL;

    BOOL Dui_GradientFill(HDC hdc, PTRIVERTEX pVertex, ULONG nVertex, PVOID pMesh, ULONG nMesh, ULONG ulMode)
    {
        if (g_fGradientFill == NULL)
        {
            HMODULE hMsing32 = ::GetModuleHandle(_T("msimg32.dll"));
            if (hMsing32)
            {
                g_fGradientFill = (PFUNCGRADIENTFILL)::GetProcAddress(hMsing32, "GradientFill");
                if (g_fGradientFill == NULL)
                    return FALSE; 
            }
        }

        return g_fGradientFill(hdc, pVertex, nVertex, pMesh, nMesh, ulMode);
    }

} // namespace DuiLib
