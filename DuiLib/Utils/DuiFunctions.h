#ifndef __DUI_FUNCTIONS_H__
#define __DUI_FUNCTIONS_H__

#pragma once

namespace DuiLib
{
    BOOL SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR lpFileName);
    BOOL ScreenToClientEx(HWND hWnd, LPRECT lpRect);
    BOOL ClientToScreenEx(HWND hWnd, LPRECT lpRect);

    HBITMAP GetWindowSnapBitmap(HWND hWnd);
    HBITMAP GetDCSnapBitmap(HDC hDC, const RECT rc);
}// namespace DuiLib

#endif // __DUI_FUNCTIONS_H__