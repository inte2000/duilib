#ifndef __UIRENDER_H__
#define __UIRENDER_H__

#pragma once

namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

class DUILIB_API CRenderClip
{
public:
    ~CRenderClip();
    RECT rcItem;
    HDC hDC;
    HRGN hRgn;
    HRGN hOldRgn;

    static void GenerateClip(HDC hDC, RECT rc, CRenderClip& clip);
	//width：一个角的宽度，height：一个角的高度
    static void GenerateRoundClip(HDC hDC, RECT rc, RECT rcItem, int width, int height, CRenderClip& clip);
    static void UseOldClipBegin(HDC hDC, CRenderClip& clip);
    static void UseOldClipEnd(HDC hDC, CRenderClip& clip);
};

/////////////////////////////////////////////////////////////////////////////////////
//

class DUILIB_API CRenderEngine
{
public:
    static DWORD AdjustColor(DWORD dwColor, short H, short S, short L);
	static HBITMAP CreateARGB32Bitmap(HDC hDC, int cx, int cy, COLORREF** pBits);
    static HBITMAP CreateBitmap(HDC hDC, int cx, int cy, BYTE** pBits);

	static void AdjustImage(bool bUseHSL, TImageInfo* imageInfo, short H, short S, short L);
    static TImageInfo* LoadImage(STRINGorID bitmap, LPCTSTR type = NULL, DWORD mask = 0);
    static void FreeImage(TImageInfo* bitmap, bool bDelete = true);
    static Gdiplus::Image* GdiplusLoadImage(LPCTSTR pstrPath, int nScale);
    static Gdiplus::Image* GdiplusLoadImage(LPVOID pBuf, size_t dwSize);

    static void DrawIcon(HDC hDC, HICON hIcon, const RECT& rc, const RECT& rcPaint);
    static void DrawImage(HDC hDC, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, \
        const RECT& rcBmpPart, const RECT& rcScale9, bool alphaChannel, BYTE uFade = 255, 
        bool hole = false, bool xtiled = false, bool ytiled = false);
	static bool DrawImage(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint, 
		TDrawInfo& drawInfo);
	static bool DrawImage(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint, 
		TDrawInfo& drawInfo, bool bExtDest);
    static void DrawColor(HDC hDC, const RECT& rc, DWORD color);
    static void DrawEllipseColor(HDC hDC, int x, int y, int x_radius, int y_radius, DWORD color);
    static void DrawGradient(HDC hDC, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps);

    // 以下函数默认使用Gdi+渲染，支持alpha通道，如果禁用 _USE_GDIPLUS，则颜色参数中的alpha值无效
    static void DrawLine(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID);
    static void DrawRect(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID);
    static void DrawRoundRect(HDC hDC, const RECT& rc, int width, int height, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID);
    static void DrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, \
        DWORD dwTextColor, int iFont, UINT uStyle);
    static void DrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, \
        DWORD dwTextColor, int iFont, UINT uStyle, DWORD dwTextBKColor);
    static void DrawHtmlText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, 
        DWORD dwTextColor, RECT* pLinks, CDuiString* sLinks, int& nLinkRects, int iDefaultFont, UINT uStyle);
	static HBITMAP GenerateBitmap(CPaintManagerUI* pManager, RECT rc, CControlUI* pStopControl = NULL, DWORD dwFilterColor = 0);
    static HBITMAP GenerateBitmap(CPaintManagerUI* pManager, CControlUI* pControl, RECT rc, DWORD dwFilterColor = 0);
	static SIZE GetTextSize(HDC hDC, CPaintManagerUI* pManager , LPCTSTR pstrText, int iFont, UINT uStyle);
	static void RestoreAlphaColor(LPBYTE pBits, int bitsWidth, PRECT rc);
};

DUILIB_API int GetScaleInt(int iValue, int nScale);
DUILIB_API SIZE GetScaleSize(const SIZE& sz, int nScale);
DUILIB_API RECT GetScaleRect(const RECT& rc, int nScale);

bool ParseDrawInfo(CPaintManagerUI* pManager, TDrawInfo& drawInfo, int nScale);

bool ParseDrawImageList(CPaintManagerUI* pManager, TDrawImgList& drawList, int nScale);

} // namespace DuiLib

#endif // __UIRENDER_H__
