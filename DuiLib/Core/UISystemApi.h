#ifndef __UISYSTEMAPI_H__
#define __UISYSTEMAPI_H__

#pragma once

namespace DuiLib {

    BOOL Dui_UpdateLayeredWindow(HWND hWnd, HDC hdcDst, POINT* pptDst, SIZE* psize,
        HDC hdcSrc, POINT* pptSrc, COLORREF crKey, BLENDFUNCTION* pblend, DWORD dwFlags);

    BOOL Dui_SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

    BOOL Dui_AlphaBlend(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest,
                        HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn);

    typedef BOOL(WINAPI* PFUNCGRADIENTFILL)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);
    extern PFUNCGRADIENTFILL g_fGradientFill;
    BOOL Dui_GradientFill(HDC hdc, PTRIVERTEX pVertex, ULONG nVertex, PVOID pMesh, ULONG nMesh, ULONG ulMode);

} // namespace DuiLib

#endif // __UISYSTEMAPI_H__
