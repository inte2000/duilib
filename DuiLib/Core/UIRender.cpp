#include "StdAfx.h"
#include "UiSystemApi.h"
#include "..\Utils\XUnzip.h"

#define STB_IMAGE_IMPLEMENTATION
#include "..\Utils\stb_image.h"

///////////////////////////////////////////////////////////////////////////////////////
#define OpenZip OpenZipU
#define CloseZip(hz) CloseZipU(hz)
extern HZIP OpenZipU(void *z,unsigned int len,DWORD flags, const char* password);
extern ZRESULT CloseZipU(HZIP hz);
#ifdef _UNICODE
#define ZIPENTRY ZIPENTRYW
#define GetZipItem GetZipItemW
#define FindZipItem FindZipItemW
#else
#define GetZipItem GetZipItemA
#define FindZipItem FindZipItemA
#endif
///////////////////////////////////////////////////////////////////////////////////////

#define RES_TYPE_COLOR _T("*COLOR*")

extern "C"
{
    extern unsigned char *stbi_load_from_memory(unsigned char const *buffer, int len, int *x, int *y, int *comp, int req_comp);
    extern unsigned char* stbi_load_gif_from_memory(unsigned char const* buffer, int len, int** delays, int* x, int* y, int* z, int* comp, int req_comp);
	extern void     stbi_image_free(void *retval_from_stbi_load);
};

namespace DuiLib {

static int g_iFontID = MAX_FONT_ID;

/////////////////////////////////////////////////////////////////////////////////////
//
//

CRenderClip::~CRenderClip()
{
    ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
    ASSERT(::GetObjectType(hRgn)==OBJ_REGION);
    ASSERT(::GetObjectType(hOldRgn)==OBJ_REGION);
    ::SelectClipRgn(hDC, hOldRgn);
    ::DeleteObject(hOldRgn);
    ::DeleteObject(hRgn);
}

void CRenderClip::GenerateClip(HDC hDC, RECT rc, CRenderClip& clip)
{
    RECT rcClip = { 0 };
    ::GetClipBox(hDC, &rcClip);
    clip.hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    clip.hRgn = ::CreateRectRgnIndirect(&rc);
    ::CombineRgn(clip.hRgn, clip.hRgn, clip.hOldRgn, RGN_AND);
    ::SelectClipRgn(hDC, clip.hRgn);
    clip.hDC = hDC;
    clip.rcItem = rc;
}

void CRenderClip::GenerateRoundClip(HDC hDC, RECT rc, RECT rcItem, int width, int height, CRenderClip& clip)
{
    RECT rcClip = { 0 };
    ::GetClipBox(hDC, &rcClip);
    clip.hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    clip.hRgn = ::CreateRectRgnIndirect(&rc);
    HRGN hRgnItem = ::CreateRoundRectRgn(rcItem.left, rcItem.top, rcItem.right + 1, rcItem.bottom + 1, width, height);
    ::CombineRgn(clip.hRgn, clip.hRgn, hRgnItem, RGN_AND);
    ::CombineRgn(clip.hRgn, clip.hRgn, clip.hOldRgn, RGN_AND);
    ::SelectClipRgn(hDC, clip.hRgn);
    clip.hDC = hDC;
    clip.rcItem = rc;
    ::DeleteObject(hRgnItem);
}

void CRenderClip::UseOldClipBegin(HDC hDC, CRenderClip& clip)
{
    ::SelectClipRgn(hDC, clip.hOldRgn);
}

void CRenderClip::UseOldClipEnd(HDC hDC, CRenderClip& clip)
{
    ::SelectClipRgn(hDC, clip.hRgn);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

static const float OneThird = 1.0f / 3;

static void RGBtoHSL(DWORD ARGB, float* H, float* S, float* L) {
    const float
        R = (float)GetRValue(ARGB),
        G = (float)GetGValue(ARGB),
        B = (float)GetBValue(ARGB),
        nR = (R<0?0:(R>255?255:R))/255,
        nG = (G<0?0:(G>255?255:G))/255,
        nB = (B<0?0:(B>255?255:B))/255,
        m = min(min(nR,nG),nB),
        M = max(max(nR,nG),nB);
    *L = (m + M)/2;
    if (M==m) *H = *S = 0;
    else {
        const float
            f = (nR==m)?(nG-nB):((nG==m)?(nB-nR):(nR-nG)),
            i = (nR==m)?3.0f:((nG==m)?5.0f:1.0f);
        *H = (i-f/(M-m));
        if (*H>=6) *H-=6;
        *H*=60;
        *S = (2*(*L)<=1)?((M-m)/(M+m)):((M-m)/(2-M-m));
    }
}

static void HSLtoRGB(DWORD* ARGB, float H, float S, float L) {
    const float
        q = 2*L<1?L*(1+S):(L+S-L*S),
        p = 2*L-q,
        h = H/360,
        tr = h + OneThird,
        tg = h,
        tb = h - OneThird,
        ntr = tr<0?tr+1:(tr>1?tr-1:tr),
        ntg = tg<0?tg+1:(tg>1?tg-1:tg),
        ntb = tb<0?tb+1:(tb>1?tb-1:tb),
        B = 255*(6*ntr<1?p+(q-p)*6*ntr:(2*ntr<1?q:(3*ntr<2?p+(q-p)*6*(2.0f*OneThird-ntr):p))),
        G = 255*(6*ntg<1?p+(q-p)*6*ntg:(2*ntg<1?q:(3*ntg<2?p+(q-p)*6*(2.0f*OneThird-ntg):p))),
        R = 255*(6*ntb<1?p+(q-p)*6*ntb:(2*ntb<1?q:(3*ntb<2?p+(q-p)*6*(2.0f*OneThird-ntb):p)));
    *ARGB &= 0xFF000000;
    *ARGB |= RGB( (BYTE)(R<0?0:(R>255?255:R)), (BYTE)(G<0?0:(G>255?255:G)), (BYTE)(B<0?0:(B>255?255:B)) );
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

DWORD CRenderEngine::AdjustColor(DWORD dwColor, short H, short S, short L)
{
    if( H == 180 && S == 100 && L == 100 ) return dwColor;
    float fH, fS, fL;
    float S1 = S / 100.0f;
    float L1 = L / 100.0f;
    RGBtoHSL(dwColor, &fH, &fS, &fL);
    fH += (H - 180);
    fH = fH > 0 ? fH : fH + 360; 
    fS *= S1;
    fL *= L1;
    HSLtoRGB(&dwColor, fH, fS, fL);
    return dwColor;
}

HBITMAP CRenderEngine::CreateARGB32Bitmap(HDC hDC, int cx, int cy, COLORREF** pBits)
{
	LPBITMAPINFO lpbiSrc = NULL;
	lpbiSrc = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
	if (lpbiSrc == NULL) return NULL;

	lpbiSrc->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbiSrc->bmiHeader.biWidth = cx;
	lpbiSrc->bmiHeader.biHeight = cy;
	lpbiSrc->bmiHeader.biPlanes = 1;
	lpbiSrc->bmiHeader.biBitCount = 32;
	lpbiSrc->bmiHeader.biCompression = BI_RGB;
	lpbiSrc->bmiHeader.biSizeImage = cx * cy;
	lpbiSrc->bmiHeader.biXPelsPerMeter = 0;
	lpbiSrc->bmiHeader.biYPelsPerMeter = 0;
	lpbiSrc->bmiHeader.biClrUsed = 0;
	lpbiSrc->bmiHeader.biClrImportant = 0;

	HBITMAP hBitmap = CreateDIBSection (hDC, lpbiSrc, DIB_RGB_COLORS, (void **)pBits, NULL, NULL);
	delete [] lpbiSrc;
	return hBitmap;
}

HBITMAP CRenderEngine::CreateBitmap(HDC hDC, int cx, int cy, BYTE** pBits)
{
    //建立up-left Bitmap
    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));
    LPBITMAPINFO lpbiSrc = NULL;

    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = -cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = cx * cy * 4;
    bmi.bmiHeader.biClrUsed = 0;
    HBITMAP hBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void**)pBits, NULL, NULL);

    return hBitmap;
}

void CRenderEngine::AdjustImage(bool bUseHSL, TImageInfo* imageInfo, short H, short S, short L)
{
	if( imageInfo == NULL || imageInfo->bUseHSL == false || imageInfo->hBitmap == NULL || 
		imageInfo->pBits == NULL || imageInfo->pSrcBits == NULL ) 
		return;
	if( bUseHSL == false || (H == 180 && S == 100 && L == 100)) {
		::CopyMemory(imageInfo->pBits, imageInfo->pSrcBits, imageInfo->nX * imageInfo->nY * 4);
		return;
	}

	float fH, fS, fL;
	float S1 = S / 100.0f;
	float L1 = L / 100.0f;
	for( int i = 0; i < imageInfo->nX * imageInfo->nY; i++ ) {
		RGBtoHSL(*(DWORD*)(imageInfo->pSrcBits + i*4), &fH, &fS, &fL);
		fH += (H - 180);
		fH = fH > 0 ? fH : fH + 360; 
		fS *= S1;
		fL *= L1;
		HSLtoRGB((DWORD*)(imageInfo->pBits + i*4), fH, fS, fL);
	}
}

TImageInfo* CRenderEngine::LoadImage(STRINGorID bitmap, LPCTSTR type, DWORD mask)
{
    LPBYTE pData = NULL;
    DWORD dwSize = 0;

    if(_tcsnicmp(bitmap.m_lpstr, _T("exeicon:"), 8) == 0)
    {
        /*调用SHGetFileInfo之前要先调用CoInitialize*/
        SHFILEINFO fi = { 0 };
        ::SHGetFileInfo(bitmap.m_lpstr + 8, 0, &fi, sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_ICON);

        if (fi.hIcon != NULL)
        {
            TImageInfo* data = new TImageInfo;
            data->isIcon = true;
            data->hIcon = fi.hIcon;
            data->hBitmap = NULL;
            data->pBits = NULL;
            data->nX = 32;
            data->nY = 32;
            data->bAlpha = false;
            data->bUseHSL = false;
            data->pSrcBits = NULL;
            //data->sResType = _T("");
            data->dwMask = 0;

            return data;
        }
        else
        {
            bitmap.m_lpstr = _T("default_app_icon.png");
        }
    }

	do 
	{
		if( type == NULL ) {
			CDuiString sFile = CPaintManagerUI::GetResourcePath();
			if( CPaintManagerUI::GetResourceZip().IsEmpty() ) {
				sFile += bitmap.m_lpstr;
				HANDLE hFile = ::CreateFile(sFile.GetData(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
					FILE_ATTRIBUTE_NORMAL, NULL);
				if( hFile == INVALID_HANDLE_VALUE ) break;
				dwSize = ::GetFileSize(hFile, NULL);
                if (dwSize == 0)
                {
                    ::CloseHandle(hFile);
                    break;
                }

				DWORD dwRead = 0;
				pData = new BYTE[ dwSize ];
				::ReadFile( hFile, pData, dwSize, &dwRead, NULL );
				::CloseHandle( hFile );

				if( dwRead != dwSize ) {
					delete[] pData;
					pData = NULL;
					break;
				}
			}
			else {
				sFile += CPaintManagerUI::GetResourceZip();
				HZIP hz = NULL;
				if( CPaintManagerUI::IsCachedResourceZip() ) hz = (HZIP)CPaintManagerUI::GetResourceZipHandle();
				else hz = OpenZip((void*)sFile.GetData(), 0, 2, CPaintManagerUI::GetResourceZipPassword());
				if( hz == NULL ) break;
				ZIPENTRY ze; 
				int i; 
				if( FindZipItem(hz, bitmap.m_lpstr, true, &i, &ze) != 0 ) break;
				dwSize = ze.unc_size;
				if( dwSize == 0 ) break;
				pData = new BYTE[ dwSize ];
				int res = UnzipItem(hz, i, pData, dwSize, 3);
				if( res != 0x00000000 && res != 0x00000600) {
					delete[] pData;
					pData = NULL;
					if( !CPaintManagerUI::IsCachedResourceZip() ) CloseZip(hz);
					break;
				}
				if( !CPaintManagerUI::IsCachedResourceZip() ) CloseZip(hz);
			}
		}
		else if (_tcscmp(type, RES_TYPE_COLOR) == 0) {
			pData = (PBYTE)0x1;  /* dummy pointer */
		}
		else {
			HRSRC hResource = ::FindResource(CPaintManagerUI::GetResourceDll(), bitmap.m_lpstr, type);
			if( hResource == NULL ) break;
			HGLOBAL hGlobal = ::LoadResource(CPaintManagerUI::GetResourceDll(), hResource);
			if( hGlobal == NULL ) {
				FreeResource(hResource);
				break;
			}

			dwSize = ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource);
			if( dwSize == 0 ) break;
			pData = new BYTE[ dwSize ];
			::CopyMemory(pData, (LPBYTE)::LockResource(hGlobal), dwSize);
			::FreeResource(hResource);
		}
	} while (0);

	while (!pData)
	{
		//读不到图片, 则直接去读取bitmap.m_lpstr指向的路径
		HANDLE hFile = ::CreateFile(bitmap.m_lpstr, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
			FILE_ATTRIBUTE_NORMAL, NULL);
		if( hFile == INVALID_HANDLE_VALUE ) break;
		dwSize = ::GetFileSize(hFile, NULL);
        if (dwSize == 0)
        {
            ::CloseHandle(hFile);
            break;
        }

		DWORD dwRead = 0;
		pData = new BYTE[ dwSize ];
		::ReadFile( hFile, pData, dwSize, &dwRead, NULL );
		::CloseHandle( hFile );

		if( dwRead != dwSize ) {
			delete[] pData;
			pData = NULL;
		}
		break;
	}
	if (!pData)
	{
		//::MessageBox(0, _T("读取图片数据失败！"), _T("抓BUG"), MB_OK);
		return NULL;
	}

    LPBYTE pImage = NULL;
    int x = 1, y = 1, n;
    if (!type || _tcscmp(type, RES_TYPE_COLOR) != 0) {
        pImage = stbi_load_from_memory(pData, dwSize, &x, &y, &n, 4);
        delete[] pData;
        if( !pImage ) {
            //::MessageBox(0, _T("解析图片失败"), _T("抓BUG"), MB_OK);
            return NULL;
        }
    }
    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = x;
    bmi.bmiHeader.biHeight = -y;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = x * y * 4;

    bool bAlphaChannel = false;
    LPBYTE pDest = NULL;
    HBITMAP hBitmap = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
	if( !hBitmap ) {
		//::MessageBox(0, _T("CreateDIBSection失败"), _T("抓BUG"), MB_OK);
		return NULL;
	}

    BYTE bColorBits[4] = { 0 };
    if (type && _tcscmp(type, RES_TYPE_COLOR) == 0) {
        LPTSTR pstr = NULL;
        LPCTSTR pstrValue = bitmap.m_lpstr;
        if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);

        pImage = (LPBYTE)&clrColor;
        /* BGRA -> RGBA */
        bColorBits[3] = pImage[3];
        bColorBits[2] = pImage[0];
        bColorBits[1] = pImage[1];
        bColorBits[0] = pImage[2];
        pImage = bColorBits;
    }

    for( int i = 0; i < x * y; i++ ) 
    {
        pDest[i*4 + 3] = pImage[i*4 + 3];
        if( pDest[i*4 + 3] < 255 )
        {
            pDest[i*4] = (BYTE)(DWORD(pImage[i*4 + 2])*pImage[i*4 + 3]/255);
            pDest[i*4 + 1] = (BYTE)(DWORD(pImage[i*4 + 1])*pImage[i*4 + 3]/255);
            pDest[i*4 + 2] = (BYTE)(DWORD(pImage[i*4])*pImage[i*4 + 3]/255); 
            bAlphaChannel = true;
        }
        else
        {
            pDest[i*4] = pImage[i*4 + 2];
            pDest[i*4 + 1] = pImage[i*4 + 1];
            pDest[i*4 + 2] = pImage[i*4]; 
        }

        if( *(DWORD*)(&pDest[i*4]) == mask ) {
            pDest[i*4] = (BYTE)0;
            pDest[i*4 + 1] = (BYTE)0;
            pDest[i*4 + 2] = (BYTE)0; 
            pDest[i*4 + 3] = (BYTE)0;
            bAlphaChannel = true;
        }
    }

    if (!type || _tcscmp(type, RES_TYPE_COLOR) != 0) {
        stbi_image_free(pImage);
    }

	TImageInfo* data = new TImageInfo;
    data->isIcon = false;
    data->hIcon = NULL;
	data->hBitmap = hBitmap;
	data->pBits = pDest;
	data->nX = x;
	data->nY = y;
	data->bAlpha = bAlphaChannel;
	data->bUseHSL = false;
	data->pSrcBits = NULL;
	return data;
}

void CRenderEngine::FreeImage(TImageInfo* bitmap, bool bDelete)
{
	if (bitmap == NULL) return;
	if (bitmap->hBitmap) {
		::DeleteObject(bitmap->hBitmap);
		bitmap->hBitmap = NULL;
	}
	if (bitmap->hIcon) {
		::DestroyIcon(bitmap->hIcon);
		bitmap->hIcon = NULL; 
	}
	if (bitmap->pSrcBits) {
		delete[] bitmap->pSrcBits;
		bitmap->pSrcBits = NULL;
	}
	if (bDelete) delete bitmap ;
}

Gdiplus::Image* CRenderEngine::GdiplusLoadImage(LPCTSTR pstrPath1, int nScale)
{
    CDuiString sImageName = pstrPath1;
    LPBYTE pData = NULL;
    DWORD dwSize = 0;

    do
    {
        CDuiString sFile = CPaintManagerUI::GetResourcePath();
        if (CPaintManagerUI::GetResourceZip().IsEmpty()) {
            sFile += sImageName;
            HANDLE hFile = ::CreateFile(sFile.GetData(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
                FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) break;
            dwSize = ::GetFileSize(hFile, NULL);
            if (dwSize == 0) //20240325 合并来自官方库的修改，官方库的这个修改位于 CGifAnimUI::LoadGifFromFile
            {
                ::CloseHandle(hFile);
                break;
            }

            DWORD dwRead = 0;
            pData = new BYTE[dwSize];
            ::ReadFile(hFile, pData, dwSize, &dwRead, NULL);
            ::CloseHandle(hFile);

            if (dwRead != dwSize) {
                delete[] pData;
                pData = NULL;
                break;
            }
        }
        else {
            sFile += CPaintManagerUI::GetResourceZip();
            HZIP hz = NULL;
            if (CPaintManagerUI::IsCachedResourceZip()) hz = (HZIP)CPaintManagerUI::GetResourceZipHandle();
            else hz = OpenZip((void*)sFile.GetData(), 0, 2, CPaintManagerUI::GetResourceZipPassword());

            if (hz == NULL) break;
            ZIPENTRY ze;
            int i = 0;
            CDuiString key = sImageName;
            key.Replace(_T("\\"), _T("/"));
            if (FindZipItem(hz, key, true, &i, &ze) != 0) break;
            dwSize = ze.unc_size;
            if (dwSize == 0) break;
            pData = new BYTE[dwSize];
            int res = UnzipItem(hz, i, pData, dwSize, 3);
            if (res != 0x00000000 && res != 0x00000600) {
                delete[] pData;
                pData = NULL;
                if (!CPaintManagerUI::IsCachedResourceZip()) CloseZip(hz);
                break;
            }
            if (!CPaintManagerUI::IsCachedResourceZip()) CloseZip(hz);
        }

    } while (0);

    while (!pData)
    {
        //读不到图片, 则直接去读取bitmap.m_lpstr指向的路径
        HANDLE hFile = ::CreateFile(sImageName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) break;
        dwSize = ::GetFileSize(hFile, NULL);
        if (dwSize == 0) //20240325 合并来自官方库的修改，官方库的这个修改位于 CGifAnimUI::LoadGifFromFile
        {
            ::CloseHandle(hFile);
            break;
        }

        DWORD dwRead = 0;
        pData = new BYTE[dwSize];
        ::ReadFile(hFile, pData, dwSize, &dwRead, NULL);
        ::CloseHandle(hFile);

        if (dwRead != dwSize) {
            delete[] pData;
            pData = NULL;
        }
        break;
    }

    Gdiplus::Image* pImage = NULL;
    if (pData != NULL) {
        pImage = GdiplusLoadImage(pData, dwSize);
        delete[] pData;
        pData = NULL;
    }
    return pImage;
}

Gdiplus::Image* CRenderEngine::GdiplusLoadImage(LPVOID pBuf, size_t dwSize)
{
    Gdiplus::Image* pImg = NULL;

    HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwSize);
    if (hMem)
    {
        BYTE* pMem = (BYTE*)::GlobalLock(hMem);
        if (pMem != NULL)
        {
            memcpy(pMem, pBuf, dwSize);
            IStream* pStm = NULL;
            if (::CreateStreamOnHGlobal(hMem, TRUE, &pStm) == S_OK) //释放 pStm 的时候自动释放 hMem
            {
                pImg = Gdiplus::Image::FromStream(pStm);
                pStm->Release();
                if (!pImg && pImg->GetLastStatus() != Gdiplus::Ok)
                {
                    delete pImg;
                    pImg = NULL;
                }
            }
            //::GlobalUnlock(hMem);
        }
        //::GlobalFree(hMem);
    }

    return pImg;
}

void CRenderEngine::DrawIcon(HDC hDC, HICON hIcon, const RECT& rc, const RECT& rcPaint)
{
    ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
    
    ::DrawIcon(hDC, rc.left, rc.top, hIcon);
}

void CRenderEngine::DrawImage(HDC hDC, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint,
							  const RECT& rcBmpPart, const RECT& rcScale9, bool bAlpha, 
							  BYTE uFade, bool bHole, bool bTiledX, bool bTiledY)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);

	if( hBitmap == NULL ) return;

	HDC hCloneDC = ::CreateCompatibleDC(hDC);
	HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
	::SetStretchBltMode(hDC, COLORONCOLOR);

	RECT rcTemp = {0};
	RECT rcDest = {0};
	if( bAlpha || uFade < 255 ) {
		BLENDFUNCTION bf = { AC_SRC_OVER, 0, uFade, AC_SRC_ALPHA };
		// middle
		if( !bHole ) {
			rcDest.left = rc.left + rcScale9.left;
			rcDest.top = rc.top + rcScale9.top;
			rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
			rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
				if( !bTiledX && !bTiledY ) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
                    Dui_AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
						rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, \
						rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, bf);
				}
				else if( bTiledX && bTiledY ) {
					LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right;
					LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom;
					int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
					int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
					for( int j = 0; j < iTimesY; ++j ) {
						LONG lDestTop = rcDest.top + lHeight * j;
						LONG lDestBottom = rcDest.top + lHeight * (j + 1);
						LONG lDrawHeight = lHeight;
						if( lDestBottom > rcDest.bottom ) {
							lDrawHeight -= lDestBottom - rcDest.bottom;
							lDestBottom = rcDest.bottom;
						}
						for( int i = 0; i < iTimesX; ++i ) {
							LONG lDestLeft = rcDest.left + lWidth * i;
							LONG lDestRight = rcDest.left + lWidth * (i + 1);
							LONG lDrawWidth = lWidth;
							if( lDestRight > rcDest.right ) {
								lDrawWidth -= lDestRight - rcDest.right;
								lDestRight = rcDest.right;
							}
                            Dui_AlphaBlend(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j,
								lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC, 
								rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, lDrawWidth, lDrawHeight, bf);
						}
					}
				}
				else if( bTiledX ) {
					LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right;
					int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
					for( int i = 0; i < iTimes; ++i ) {
						LONG lDestLeft = rcDest.left + lWidth * i;
						LONG lDestRight = rcDest.left + lWidth * (i + 1);
						LONG lDrawWidth = lWidth;
						if( lDestRight > rcDest.right ) {
							lDrawWidth -= lDestRight - rcDest.right;
							lDestRight = rcDest.right;
						}
						rcDest.bottom -= rcDest.top;
                        Dui_AlphaBlend(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom,
							hCloneDC, rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
							lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, bf);
					}
				}
				else { // bTiledY
					LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom;
					int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
					for( int i = 0; i < iTimes; ++i ) {
						LONG lDestTop = rcDest.top + lHeight * i;
						LONG lDestBottom = rcDest.top + lHeight * (i + 1);
						LONG lDrawHeight = lHeight;
						if( lDestBottom > rcDest.bottom ) {
							lDrawHeight -= lDestBottom - rcDest.bottom;
							lDestBottom = rcDest.bottom;
						}
						rcDest.right -= rcDest.left;
                        Dui_AlphaBlend(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop,
							hCloneDC, rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
							rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, lDrawHeight, bf);                    
					}
				}
			}
		}

		// left-top
		if( rcScale9.left > 0 && rcScale9.top > 0 ) {
			rcDest.left = rc.left;
			rcDest.top = rc.top;
			rcDest.right = rcScale9.left;
			rcDest.bottom = rcScale9.top;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
                Dui_AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left, rcBmpPart.top, rcScale9.left, rcScale9.top, bf);
			}
		}
		// top
		if( rcScale9.top > 0 ) {
			rcDest.left = rc.left + rcScale9.left;
			rcDest.top = rc.top;
			rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
			rcDest.bottom = rcScale9.top;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
                Dui_AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left + rcScale9.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - \
					rcScale9.left - rcScale9.right, rcScale9.top, bf);
			}
		}
		// right-top
		if( rcScale9.right > 0 && rcScale9.top > 0 ) {
			rcDest.left = rc.right - rcScale9.right;
			rcDest.top = rc.top;
			rcDest.right = rcScale9.right;
			rcDest.bottom = rcScale9.top;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
                Dui_AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.right - rcScale9.right, rcBmpPart.top, rcScale9.right, rcScale9.top, bf);
			}
		}
		// left
		if( rcScale9.left > 0 ) {
			rcDest.left = rc.left;
			rcDest.top = rc.top + rcScale9.top;
			rcDest.right = rcScale9.left;
			rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
                Dui_AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left, rcBmpPart.top + rcScale9.top, rcScale9.left, rcBmpPart.bottom - \
					rcBmpPart.top - rcScale9.top - rcScale9.bottom, bf);
			}
		}
		// right
		if( rcScale9.right > 0 ) {
			rcDest.left = rc.right - rcScale9.right;
			rcDest.top = rc.top + rcScale9.top;
			rcDest.right = rcScale9.right;
			rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
                Dui_AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.right - rcScale9.right, rcBmpPart.top + rcScale9.top, rcScale9.right, \
					rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, bf);
			}
		}
		// left-bottom
		if( rcScale9.left > 0 && rcScale9.bottom > 0 ) {
			rcDest.left = rc.left;
			rcDest.top = rc.bottom - rcScale9.bottom;
			rcDest.right = rcScale9.left;
			rcDest.bottom = rcScale9.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
                Dui_AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left, rcBmpPart.bottom - rcScale9.bottom, rcScale9.left, rcScale9.bottom, bf);
			}
		}
		// bottom
		if( rcScale9.bottom > 0 ) {
			rcDest.left = rc.left + rcScale9.left;
			rcDest.top = rc.bottom - rcScale9.bottom;
			rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
			rcDest.bottom = rcScale9.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
                Dui_AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.left + rcScale9.left, rcBmpPart.bottom - rcScale9.bottom, \
					rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, rcScale9.bottom, bf);
			}
		}
		// right-bottom
		if( rcScale9.right > 0 && rcScale9.bottom > 0 ) {
			rcDest.left = rc.right - rcScale9.right;
			rcDest.top = rc.bottom - rcScale9.bottom;
			rcDest.right = rcScale9.right;
			rcDest.bottom = rcScale9.bottom;
			rcDest.right += rcDest.left;
			rcDest.bottom += rcDest.top;
			if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
				rcDest.right -= rcDest.left;
				rcDest.bottom -= rcDest.top;
                Dui_AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
					rcBmpPart.right - rcScale9.right, rcBmpPart.bottom - rcScale9.bottom, rcScale9.right, \
					rcScale9.bottom, bf);
			}
		}
	}
	else 
	{
		if (rc.right - rc.left == rcBmpPart.right - rcBmpPart.left \
			&& rc.bottom - rc.top == rcBmpPart.bottom - rcBmpPart.top \
			&& rcScale9.left == 0 && rcScale9.right == 0 && rcScale9.top == 0 && rcScale9.bottom == 0)
		{
			if( ::IntersectRect(&rcTemp, &rcPaint, &rc) ) {
				::BitBlt(hDC, rcTemp.left, rcTemp.top, rcTemp.right - rcTemp.left, rcTemp.bottom - rcTemp.top, \
					hCloneDC, rcBmpPart.left + rcTemp.left - rc.left, rcBmpPart.top + rcTemp.top - rc.top, SRCCOPY);
			}
		}
		else
		{
			// middle
			if( !bHole ) {
				rcDest.left = rc.left + rcScale9.left;
				rcDest.top = rc.top + rcScale9.top;
				rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
				rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
					if( !bTiledX && !bTiledY ) {
						rcDest.right -= rcDest.left;
						rcDest.bottom -= rcDest.top;
						::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
							rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
							rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, \
							rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, SRCCOPY);
					}
					else if( bTiledX && bTiledY ) {
						LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right;
						LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom;
						int iTimesX = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
						int iTimesY = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
						for( int j = 0; j < iTimesY; ++j ) {
							LONG lDestTop = rcDest.top + lHeight * j;
							LONG lDestBottom = rcDest.top + lHeight * (j + 1);
							LONG lDrawHeight = lHeight;
							if( lDestBottom > rcDest.bottom ) {
								lDrawHeight -= lDestBottom - rcDest.bottom;
								lDestBottom = rcDest.bottom;
							}
							for( int i = 0; i < iTimesX; ++i ) {
								LONG lDestLeft = rcDest.left + lWidth * i;
								LONG lDestRight = rcDest.left + lWidth * (i + 1);
								LONG lDrawWidth = lWidth;
								if( lDestRight > rcDest.right ) {
									lDrawWidth -= lDestRight - rcDest.right;
									lDestRight = rcDest.right;
								}
								::BitBlt(hDC, rcDest.left + lWidth * i, rcDest.top + lHeight * j, \
									lDestRight - lDestLeft, lDestBottom - lDestTop, hCloneDC, \
									rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, SRCCOPY);
							}
						}
					}
					else if( bTiledX ) {
						LONG lWidth = rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right;
						int iTimes = (rcDest.right - rcDest.left + lWidth - 1) / lWidth;
						for( int i = 0; i < iTimes; ++i ) {
							LONG lDestLeft = rcDest.left + lWidth * i;
							LONG lDestRight = rcDest.left + lWidth * (i + 1);
							LONG lDrawWidth = lWidth;
							if( lDestRight > rcDest.right ) {
								lDrawWidth -= lDestRight - rcDest.right;
								lDestRight = rcDest.right;
							}
							rcDest.bottom -= rcDest.top;
							::StretchBlt(hDC, lDestLeft, rcDest.top, lDestRight - lDestLeft, rcDest.bottom, 
								hCloneDC, rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
								lDrawWidth, rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, SRCCOPY);
						}
					}
					else { // bTiledY
						LONG lHeight = rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom;
						int iTimes = (rcDest.bottom - rcDest.top + lHeight - 1) / lHeight;
						for( int i = 0; i < iTimes; ++i ) {
							LONG lDestTop = rcDest.top + lHeight * i;
							LONG lDestBottom = rcDest.top + lHeight * (i + 1);
							LONG lDrawHeight = lHeight;
							if( lDestBottom > rcDest.bottom ) {
								lDrawHeight -= lDestBottom - rcDest.bottom;
								lDestBottom = rcDest.bottom;
							}
							rcDest.right -= rcDest.left;
							::StretchBlt(hDC, rcDest.left, rcDest.top + lHeight * i, rcDest.right, lDestBottom - lDestTop, 
								hCloneDC, rcBmpPart.left + rcScale9.left, rcBmpPart.top + rcScale9.top, \
								rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, lDrawHeight, SRCCOPY);                    
						}
					}
				}
			}

			// left-top
			if( rcScale9.left > 0 && rcScale9.top > 0 ) {
				rcDest.left = rc.left;
				rcDest.top = rc.top;
				rcDest.right = rcScale9.left;
				rcDest.bottom = rcScale9.top;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left, rcBmpPart.top, rcScale9.left, rcScale9.top, SRCCOPY);
				}
			}
			// top
			if( rcScale9.top > 0 ) {
				rcDest.left = rc.left + rcScale9.left;
				rcDest.top = rc.top;
				rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
				rcDest.bottom = rcScale9.top;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left + rcScale9.left, rcBmpPart.top, rcBmpPart.right - rcBmpPart.left - \
						rcScale9.left - rcScale9.right, rcScale9.top, SRCCOPY);
				}
			}
			// right-top
			if( rcScale9.right > 0 && rcScale9.top > 0 ) {
				rcDest.left = rc.right - rcScale9.right;
				rcDest.top = rc.top;
				rcDest.right = rcScale9.right;
				rcDest.bottom = rcScale9.top;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.right - rcScale9.right, rcBmpPart.top, rcScale9.right, rcScale9.top, SRCCOPY);
				}
			}
			// left
			if( rcScale9.left > 0 ) {
				rcDest.left = rc.left;
				rcDest.top = rc.top + rcScale9.top;
				rcDest.right = rcScale9.left;
				rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left, rcBmpPart.top + rcScale9.top, rcScale9.left, rcBmpPart.bottom - \
						rcBmpPart.top - rcScale9.top - rcScale9.bottom, SRCCOPY);
				}
			}
			// right
			if( rcScale9.right > 0 ) {
				rcDest.left = rc.right - rcScale9.right;
				rcDest.top = rc.top + rcScale9.top;
				rcDest.right = rcScale9.right;
				rcDest.bottom = rc.bottom - rc.top - rcScale9.top - rcScale9.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.right - rcScale9.right, rcBmpPart.top + rcScale9.top, rcScale9.right, \
						rcBmpPart.bottom - rcBmpPart.top - rcScale9.top - rcScale9.bottom, SRCCOPY);
				}
			}
			// left-bottom
			if( rcScale9.left > 0 && rcScale9.bottom > 0 ) {
				rcDest.left = rc.left;
				rcDest.top = rc.bottom - rcScale9.bottom;
				rcDest.right = rcScale9.left;
				rcDest.bottom = rcScale9.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left, rcBmpPart.bottom - rcScale9.bottom, rcScale9.left, rcScale9.bottom, SRCCOPY);
				}
			}
			// bottom
			if( rcScale9.bottom > 0 ) {
				rcDest.left = rc.left + rcScale9.left;
				rcDest.top = rc.bottom - rcScale9.bottom;
				rcDest.right = rc.right - rc.left - rcScale9.left - rcScale9.right;
				rcDest.bottom = rcScale9.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.left + rcScale9.left, rcBmpPart.bottom - rcScale9.bottom, \
						rcBmpPart.right - rcBmpPart.left - rcScale9.left - rcScale9.right, rcScale9.bottom, SRCCOPY);
				}
			}
			// right-bottom
			if( rcScale9.right > 0 && rcScale9.bottom > 0 ) {
				rcDest.left = rc.right - rcScale9.right;
				rcDest.top = rc.bottom - rcScale9.bottom;
				rcDest.right = rcScale9.right;
				rcDest.bottom = rcScale9.bottom;
				rcDest.right += rcDest.left;
				rcDest.bottom += rcDest.top;
				if( ::IntersectRect(&rcTemp, &rcPaint, &rcDest) ) {
					rcDest.right -= rcDest.left;
					rcDest.bottom -= rcDest.top;
					::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.right, rcDest.bottom, hCloneDC, \
						rcBmpPart.right - rcScale9.right, rcBmpPart.bottom - rcScale9.bottom, rcScale9.right, \
						rcScale9.bottom, SRCCOPY);
				}
			}
		}
	}

	::SelectObject(hCloneDC, hOldBitmap);
	::DeleteDC(hCloneDC);
}

static bool ParseDrawInfo(CPaintManagerUI* pManager, TDrawInfo& drawInfo, bool bExtDest)
{
    bool bUseRes = false;
    CDuiString sImageName = drawInfo.sDrawString;
    CDuiString sImageResType;
    DWORD dwMask = 0;
    bool bUseHSL = false;

    CDuiString sItem;
    CDuiString sValue;
    LPTSTR pstr = NULL;
    LPCTSTR pstrImage = drawInfo.sDrawString.GetData();
    while (*pstrImage != _T('\0')) {
        sItem.Empty();
        sValue.Empty();
        while (*pstrImage > _T('\0') && *pstrImage <= _T(' ')) pstrImage = ::CharNext(pstrImage);
        while (*pstrImage != _T('\0') && *pstrImage != _T('=') && *pstrImage > _T(' ')) {
            LPTSTR pstrTemp = ::CharNext(pstrImage);
            while (pstrImage < pstrTemp) {
                sItem += *pstrImage++;
            }
        }
        while (*pstrImage > _T('\0') && *pstrImage <= _T(' ')) pstrImage = ::CharNext(pstrImage);
        if (*pstrImage++ != _T('=')) break;
        while (*pstrImage > _T('\0') && *pstrImage <= _T(' ')) pstrImage = ::CharNext(pstrImage);
        if (*pstrImage++ != _T('\'')) break;
        while (*pstrImage != _T('\0') && *pstrImage != _T('\'')) {
            LPTSTR pstrTemp = ::CharNext(pstrImage);
            while (pstrImage < pstrTemp) {
                sValue += *pstrImage++;
            }
        }
        if (*pstrImage++ != _T('\'')) break;
        if (!sValue.IsEmpty()) {
            if (sItem == _T("file")) {
                sImageName = sValue;
            }
            else if (sItem == _T("res")) {
                bUseRes = true;
                sImageName = sValue;
            }
            else if (sItem == _T("restype")) {
                sImageResType = sValue;
            }
            else if (sItem == _T("color")) {
                bUseRes = true;
                sImageResType = RES_TYPE_COLOR;
                sImageName = sValue;
            }
            else if (sItem == _T("dest")) {
                drawInfo.rcDestOffset.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);
                drawInfo.rcDestOffset.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
                drawInfo.rcDestOffset.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
                drawInfo.rcDestOffset.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
            }
            else if (sItem == _T("source")) {
                drawInfo.rcBmpPart.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);
                drawInfo.rcBmpPart.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
                drawInfo.rcBmpPart.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
                drawInfo.rcBmpPart.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
            }
            else if (sItem == _T("corner") || sItem == _T("scale9")) {
                drawInfo.rcScale9.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);
                drawInfo.rcScale9.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
                drawInfo.rcScale9.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
                drawInfo.rcScale9.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
            }
            else if (sItem == _T("mask")) {
                if (sValue[0] == _T('#')) dwMask = _tcstoul(sValue.GetData() + 1, &pstr, 16);
                else dwMask = _tcstoul(sValue.GetData(), &pstr, 16);
            }
            else if (sItem == _T("fade")) {
                drawInfo.uFade = (BYTE)_tcstoul(sValue.GetData(), &pstr, 10);
            }
            else if (sItem == _T("hole")) {
                drawInfo.bHole = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
            else if (sItem == _T("xtiled")) {
                drawInfo.bTiledX = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
            else if (sItem == _T("ytiled")) {
                drawInfo.bTiledY = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
            else if (sItem == _T("hsl")) {
                bUseHSL = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
            else if (sItem == _T("scaled")) {
                drawInfo.bSupportScale = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
        }
        if (*pstrImage++ != _T(' ')) break;
    }

    // 调整DPI资源
    drawInfo.sImageName = sImageName;
    CDuiString scaleImgName = sImageName;
    if (pManager->GetDPIObj()->GetScale() != 100) {
        CDuiString sScale;
        sScale.Format(_T("@%d."), pManager->GetDPIObj()->GetScale());
        scaleImgName.Replace(_T("."), sScale);
    }
    //if (CPaintManagerUI::IsResourceFileExisted(scaleImgName))
    if(drawInfo.bSupportScale) {
        sImageName = scaleImgName;
        drawInfo.rcBmpPart = GetScaleRect(drawInfo.rcBmpPart, pManager->GetDPIObj()->GetScale());
        drawInfo.rcScale9 = GetScaleRect(drawInfo.rcScale9, pManager->GetDPIObj()->GetScale());
    }
    if(!bExtDest)
        drawInfo.rcDestOffset = GetScaleRect(drawInfo.rcDestOffset, pManager->GetDPIObj()->GetScale());

    const TImageInfo* data = NULL;
    if (bUseRes == false) {
        data = pManager->GetImageEx((LPCTSTR)sImageName, NULL, dwMask, bUseHSL);
    }
    else {
        data = pManager->GetImageEx((LPCTSTR)sImageName, (LPCTSTR)sImageResType, dwMask, bUseHSL);
    }
    if (!data) return false;

    drawInfo.pImageInfo = data;
    if (drawInfo.rcBmpPart.left == 0 && drawInfo.rcBmpPart.right == 0 &&
        drawInfo.rcBmpPart.top == 0 && drawInfo.rcBmpPart.bottom == 0) {
        drawInfo.rcBmpPart.right = data->nX;
        drawInfo.rcBmpPart.bottom = data->nY;
    }

    return true;
}

bool CRenderEngine::DrawImage(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint, 
					  TDrawInfo& drawInfo)
{
	// 1、aaa.jpg
	// 2、file='aaa.jpg' res='' restype='0' dest='0,0,0,0' source='0,0,0,0' scale9='0,0,0,0' 
	// mask='#FF0000' fade='255' hole='false' xtiled='false' ytiled='false' hsl='false'
	if( pManager == NULL ) return true;
	if( drawInfo.pImageInfo == NULL ) {
		if( drawInfo.bLoaded ) return false;
		drawInfo.bLoaded = true;
		if( drawInfo.sDrawString.IsEmpty() ) return false;

        if (!ParseDrawInfo(pManager, drawInfo, false))
            return false;
	}
	if( drawInfo.rcBmpPart.right > drawInfo.pImageInfo->nX ) drawInfo.rcBmpPart.right = drawInfo.pImageInfo->nX;
	if( drawInfo.rcBmpPart.bottom > drawInfo.pImageInfo->nY ) drawInfo.rcBmpPart.bottom = drawInfo.pImageInfo->nY;

	if( hDC == NULL ) return true;

	RECT rcDest = rcItem;
	if( drawInfo.rcDestOffset.left != 0 || drawInfo.rcDestOffset.top != 0 ||
		drawInfo.rcDestOffset.right != 0 || drawInfo.rcDestOffset.bottom != 0 ) {
			rcDest.left = rcItem.left + drawInfo.rcDestOffset.left;
			rcDest.top = rcItem.top + drawInfo.rcDestOffset.top;
			rcDest.right = rcItem.left + drawInfo.rcDestOffset.right;
			if( rcDest.right > rcItem.right ) rcDest.right = rcItem.right;
			rcDest.bottom = rcItem.top + drawInfo.rcDestOffset.bottom;
			if( rcDest.bottom > rcItem.bottom ) rcDest.bottom = rcItem.bottom;
	}

	RECT rcTemp;
	if( !::IntersectRect(&rcTemp, &rcDest, &rcItem) ) return true;
	if( !::IntersectRect(&rcTemp, &rcDest, &rcPaint) ) return true;
	
    if(drawInfo.pImageInfo->isIcon)
        DrawIcon(hDC, drawInfo.pImageInfo->hIcon, rcDest, rcPaint);
    else
	    DrawImage(hDC, drawInfo.pImageInfo->hBitmap, rcDest, rcPaint, drawInfo.rcBmpPart, drawInfo.rcScale9,
	        drawInfo.pImageInfo->bAlpha, drawInfo.uFade, drawInfo.bHole, drawInfo.bTiledX, drawInfo.bTiledY);
	return true;
}

bool CRenderEngine::DrawImage(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint,
    TDrawInfo& drawInfo, bool bExtDest)
{
    // 1、aaa.jpg
    // 2、file='aaa.jpg' res='' restype='0' dest='0,0,0,0' source='0,0,0,0' scale9='0,0,0,0' 
    // mask='#FF0000' fade='255' hole='false' xtiled='false' ytiled='false' hsl='false'
    if (pManager == NULL) return true;
    if (drawInfo.pImageInfo == NULL) {
        if (drawInfo.bLoaded) return false;
        drawInfo.bLoaded = true;
        if (drawInfo.sDrawString.IsEmpty()) return false;

        if (!ParseDrawInfo(pManager, drawInfo, bExtDest))
            return false;
    }
    if (drawInfo.rcBmpPart.right > drawInfo.pImageInfo->nX) drawInfo.rcBmpPart.right = drawInfo.pImageInfo->nX;
    if (drawInfo.rcBmpPart.bottom > drawInfo.pImageInfo->nY) drawInfo.rcBmpPart.bottom = drawInfo.pImageInfo->nY;

    if (hDC == NULL) return true;

    RECT rcDest;
    if (!bExtDest) {
        rcDest = rcItem;
        if (drawInfo.rcDestOffset.left != 0 || drawInfo.rcDestOffset.top != 0 ||
            drawInfo.rcDestOffset.right != 0 || drawInfo.rcDestOffset.bottom != 0) {
            rcDest.left = rcItem.left + drawInfo.rcDestOffset.left;
            rcDest.top = rcItem.top + drawInfo.rcDestOffset.top;
            rcDest.right = rcItem.left + drawInfo.rcDestOffset.right;
            if (rcDest.right > rcItem.right) rcDest.right = rcItem.right;
            rcDest.bottom = rcItem.top + drawInfo.rcDestOffset.bottom;
            if (rcDest.bottom > rcItem.bottom) rcDest.bottom = rcItem.bottom;
        }
    }
    else {
        rcDest = drawInfo.rcDestOffset;
    }

    RECT rcTemp;
    if (!::IntersectRect(&rcTemp, &rcDest, &rcItem)) return true;
    if (!::IntersectRect(&rcTemp, &rcDest, &rcPaint)) return true;

    if (drawInfo.pImageInfo->isIcon)
        DrawIcon(hDC, drawInfo.pImageInfo->hIcon, rcDest, rcPaint);
    else
	    DrawImage(hDC, drawInfo.pImageInfo->hBitmap, rcDest, rcPaint, drawInfo.rcBmpPart, drawInfo.rcScale9,
	        drawInfo.pImageInfo->bAlpha, drawInfo.uFade, drawInfo.bHole, drawInfo.bTiledX, drawInfo.bTiledY);
	return true;
}

void CRenderEngine::DrawColor(HDC hDC, const RECT& rc, DWORD color)
{
    if( color <= 0x00FFFFFF ) return;
#ifdef _USE_GDIPLUS
    Gdiplus::Graphics graphics(hDC);
    Gdiplus::SolidBrush brush(Gdiplus::Color((LOBYTE((color) >> 24)), GetBValue(color), GetGValue(color), GetRValue(color)));

    graphics.FillRectangle(&brush, (int)rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
#else
    if( color >= 0xFF000000 )
    {
        ::SetBkColor(hDC, RGB(GetBValue(color), GetGValue(color), GetRValue(color)));
        ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    }
    else
    {
        // Create a new 32bpp bitmap with room for an alpha channel
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = 1;
        bmi.bmiHeader.biHeight = 1;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 1 * 1 * sizeof(DWORD);
        LPDWORD pDest = NULL;
        HBITMAP hBitmap = ::CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (LPVOID*) &pDest, NULL, 0);
        if( !hBitmap ) return;

        *pDest = color;

        RECT rcBmpPart = {0, 0, 1, 1};
        RECT rcCorners = {0};
        DrawImage(hDC, hBitmap, rc, rc, rcBmpPart, rcCorners, true, 255);
        ::DeleteObject(hBitmap);
    }
#endif
}

void CRenderEngine::DrawEllipseColor(HDC hDC, int x, int y, int x_radius, int y_radius, DWORD color)
{
    if (color <= 0x00FFFFFF) return;
#ifdef _USE_GDIPLUS
    Gdiplus::Graphics graphics(hDC);
    Gdiplus::SolidBrush brush(Gdiplus::Color((LOBYTE((color) >> 24)), GetBValue(color), GetGValue(color), GetRValue(color)));

    graphics.FillEllipse(&brush, x, y, x_radius, y_radius);
#else
    HBRUSH hFillBrush = ::CreateSolidBrush(RGB(GetBValue(color), GetGValue(color), GetRValue(color)));
    if (hFillBrush != NULL)
    {
        RECT rc = {x - x_radius, y - y_radius, x + x_radius, y + y_radius };
        HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDC, hFillBrush);
        ::Ellipse(hDC, rc.left, rc.top, rc.right, rc.bottom);
        ::SelectObject(hDC, hOldBrush);
        ::DeleteObject(hFillBrush);
    }
#endif
}

void CRenderEngine::DrawGradient(HDC hDC, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps)
{
    typedef BOOL (WINAPI *PGradientFill)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);
    static PGradientFill lpGradientFill = (PGradientFill) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "GradientFill");

    BYTE bAlpha = (BYTE)(((dwFirst >> 24) + (dwSecond >> 24)) >> 1);
    if( bAlpha == 0 ) return;
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;
    RECT rcPaint = rc;
    HDC hPaintDC = hDC;
    HBITMAP hPaintBitmap = NULL;
    HBITMAP hOldPaintBitmap = NULL;
    if( bAlpha < 255 ) {
        rcPaint.left = rcPaint.top = 0;
        rcPaint.right = cx;
        rcPaint.bottom = cy;
        hPaintDC = ::CreateCompatibleDC(hDC);
        hPaintBitmap = ::CreateCompatibleBitmap(hDC, cx, cy);
        ASSERT(hPaintDC);
        ASSERT(hPaintBitmap);
        hOldPaintBitmap = (HBITMAP) ::SelectObject(hPaintDC, hPaintBitmap);
    }
    if( lpGradientFill != NULL ) 
    {
        TRIVERTEX triv[2] = 
        {
            { rcPaint.left, rcPaint.top, GetBValue(dwFirst) << 8, GetGValue(dwFirst) << 8, GetRValue(dwFirst) << 8, 0xFF00 },
            { rcPaint.right, rcPaint.bottom, GetBValue(dwSecond) << 8, GetGValue(dwSecond) << 8, GetRValue(dwSecond) << 8, 0xFF00 }
        };
        GRADIENT_RECT grc = { 0, 1 };
        lpGradientFill(hPaintDC, triv, 2, &grc, 1, bVertical ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
    }
    else 
    {
        // Determine how many shades
        int nShift = 1;
        if( nSteps >= 64 ) nShift = 6;
        else if( nSteps >= 32 ) nShift = 5;
        else if( nSteps >= 16 ) nShift = 4;
        else if( nSteps >= 8 ) nShift = 3;
        else if( nSteps >= 4 ) nShift = 2;
        int nLines = 1 << nShift;
        for( int i = 0; i < nLines; i++ ) {
            // Do a little alpha blending
            BYTE bR = (BYTE) ((GetBValue(dwSecond) * (nLines - i) + GetBValue(dwFirst) * i) >> nShift);
            BYTE bG = (BYTE) ((GetGValue(dwSecond) * (nLines - i) + GetGValue(dwFirst) * i) >> nShift);
            BYTE bB = (BYTE) ((GetRValue(dwSecond) * (nLines - i) + GetRValue(dwFirst) * i) >> nShift);
            // ... then paint with the resulting color
            HBRUSH hBrush = ::CreateSolidBrush(RGB(bR,bG,bB));
            RECT r2 = rcPaint;
            if( bVertical ) {
                r2.bottom = rc.bottom - ((i * (rc.bottom - rc.top)) >> nShift);
                r2.top = rc.bottom - (((i + 1) * (rc.bottom - rc.top)) >> nShift);
                if( (r2.bottom - r2.top) > 0 ) ::FillRect(hDC, &r2, hBrush);
            }
            else {
                r2.left = rc.right - (((i + 1) * (rc.right - rc.left)) >> nShift);
                r2.right = rc.right - ((i * (rc.right - rc.left)) >> nShift);
                if( (r2.right - r2.left) > 0 ) ::FillRect(hPaintDC, &r2, hBrush);
            }
            ::DeleteObject(hBrush);
        }
    }
    if( bAlpha < 255 ) {
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, bAlpha, AC_SRC_ALPHA };
        Dui_AlphaBlend(hDC, rc.left, rc.top, cx, cy, hPaintDC, 0, 0, cx, cy, bf);
        ::SelectObject(hPaintDC, hOldPaintBitmap);
        ::DeleteObject(hPaintBitmap);
        ::DeleteDC(hPaintDC);
    }
}

void CRenderEngine::DrawLine( HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);

#ifdef _USE_GDIPLUS
    Gdiplus::Graphics graphics(hDC);
    Gdiplus::Pen pen(Gdiplus::Color(dwPenColor), (Gdiplus::REAL)nSize);
    pen.SetDashStyle((Gdiplus::DashStyle)nStyle);
    graphics.DrawLine(&pen, Gdiplus::Point(rc.left, rc.top), Gdiplus::Point(rc.right, rc.bottom));
#else
	LOGPEN lg;
	lg.lopnColor = RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor));
	lg.lopnStyle = nStyle;
	lg.lopnWidth.x = nSize;
	HPEN hPen = CreatePenIndirect(&lg);
	HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
	POINT ptTemp = { 0 };
	::MoveToEx(hDC, rc.left, rc.top, &ptTemp);
	::LineTo(hDC, rc.right, rc.bottom);
	::SelectObject(hDC, hOldPen);
	::DeleteObject(hPen);
#endif
}

void CRenderEngine::DrawRect(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle)
{
    ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);

#ifdef _USE_GDIPLUS
    Gdiplus::Graphics graphics(hDC);
    Gdiplus::Pen pen(Gdiplus::Color(dwPenColor), (Gdiplus::REAL)nSize);
    pen.SetAlignment(Gdiplus::PenAlignmentInset);
    /*
    * orbit 20240603
    * 宽度是 1 的时候，矩形框的右边和下边会画到边界外边，这与 GDI 的 PS_INSIDEFRAME 行为不一致，
    * 可能的原因是 GDI+内部浮点数计算的错误，下面的链接是关于这个问题的讨论
    * https://stackoverflow.com/questions/70980397/gdi-penalignmentinset-seems-to-be-different-than-gdi-ps-insideframe
    */
    if(nSize == 1)
        graphics.DrawRectangle(&pen, (int)rc.left, rc.top, rc.right - rc.left - 1, rc.bottom - rc.top - 1);
    else
        graphics.DrawRectangle(&pen, (int)rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
#else
    HPEN hPen = ::CreatePen(nStyle | PS_INSIDEFRAME, nSize, RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor)));
    HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
    ::SelectObject(hDC, ::GetStockObject(HOLLOW_BRUSH));
    ::Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);
#endif
}

void CRenderEngine::DrawRoundRect(HDC hDC, const RECT& rc, int nSize, int width, int height, DWORD dwPenColor, int nStyle)
{
    ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);

#ifdef _USE_GDIPLUS
    Gdiplus::Graphics graphics(hDC);
    Gdiplus::Pen pen(Gdiplus::Color(dwPenColor), (Gdiplus::REAL)nSize);
    pen.SetAlignment(Gdiplus::PenAlignmentInset);

    Gdiplus::GraphicsPath path;
    // 把圆角矩形分成直线、弧的组合，依次加到路径中
    //左
    path.AddLine(Gdiplus::Point(rc.left, rc.bottom - height), Gdiplus::Point(rc.left, rc.top + height));
    //左上
    path.AddArc(rc.left, rc.top, width * 2, height * 2, (Gdiplus::REAL)180, (Gdiplus::REAL)90);

    //上
    path.AddLine(Gdiplus::Point(rc.left + width, rc.top), Gdiplus::Point(rc.right - width, rc.top));
    //右上
    path.AddArc(rc.right - width * 2 - 1, rc.top, width * 2, height * 2, (Gdiplus::REAL)270, (Gdiplus::REAL)90);

    //右
    path.AddLine(Gdiplus::Point(rc.right - 1, rc.top + height), Gdiplus::Point(rc.right - 1, rc.bottom - height));
    //右下
    path.AddArc(rc.right - width * 2 - 1, rc.bottom - height * 2 - 1, width * 2, height * 2, (Gdiplus::REAL)0, (Gdiplus::REAL)90);

    //下
    path.AddLine(Gdiplus::Point(rc.right - width, rc.bottom - 1), Gdiplus::Point(rc.left + width, rc.bottom - 1));
    //左下
    path.AddArc(rc.left, rc.bottom - height * 2 - 1, width * 2, height * 2, (Gdiplus::REAL)90, (Gdiplus::REAL)90);

    graphics.DrawPath(&pen, &path);
#else
    HPEN hPen = ::CreatePen(nStyle | PS_INSIDEFRAME, nSize, RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor)));
    HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
    ::SelectObject(hDC, ::GetStockObject(HOLLOW_BRUSH));
    ::RoundRect(hDC, rc.left, rc.top, rc.right, rc.bottom, width, height);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);
#endif
}

void CRenderEngine::DrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwTextColor, int iFont, UINT uStyle)
{
    ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
    if( pstrText == NULL || pManager == NULL ) return;

    if (pManager->IsLayered() || pManager->IsUseGdiplusText())
    {
        Gdiplus::Graphics graphics(hDC);

        Gdiplus::Font font(hDC, pManager->GetFont(iFont));
        Gdiplus::RectF rectF((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)(rc.right - rc.left), (Gdiplus::REAL)(rc.bottom - rc.top));
        Gdiplus::SolidBrush brush(Gdiplus::Color(254, GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));

        graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias); //默认抗锯齿级别
        Gdiplus::StringFormat stringFormat = Gdiplus::StringFormat::GenericTypographic();

        if ((uStyle & DT_END_ELLIPSIS) != 0) {
            stringFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
        }

        int formatFlags = 0;
        if ((uStyle & DT_NOCLIP) != 0) {
            formatFlags |= Gdiplus::StringFormatFlagsNoClip;
        }
        if ((uStyle & DT_SINGLELINE) != 0) {
            formatFlags |= Gdiplus::StringFormatFlagsNoWrap;
        }

        stringFormat.SetFormatFlags(formatFlags);

        if ((uStyle & DT_LEFT) != 0) {
            stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
        }
        else if ((uStyle & DT_CENTER) != 0) {
            stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
        }
        else if ((uStyle & DT_RIGHT) != 0) {
            stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
        }
        else {
            stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
        }
        stringFormat.GenericTypographic();
        if ((uStyle & DT_TOP) != 0) {
            stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
        }
        else if ((uStyle & DT_VCENTER) != 0) {
            stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        }
        else if ((uStyle & DT_BOTTOM) != 0) {
            stringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
        }
        else {
            stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
        }

        if ((uStyle & DT_CALCRECT) != 0)
        {
            Gdiplus::RectF bounds;
            graphics.MeasureString(pstrText, -1, &font, rectF, &stringFormat, &bounds);

            // MeasureString存在计算误差，这里加一像素
            rc.bottom = rc.top + (long)bounds.Height + 1;
            rc.right = rc.left + (long)bounds.Width + 1;
        }
        else
        {
            graphics.DrawString(pstrText, -1, &font, rectF, &stringFormat, &brush);
        }
    }
    else
    {
        ::SetBkMode(hDC, TRANSPARENT);
        ::SetTextColor(hDC, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));
        HFONT hOldFont = (HFONT)::SelectObject(hDC, pManager->GetFont(iFont));
        ::DrawText(hDC, pstrText, -1, &rc, uStyle | DT_NOPREFIX);
        ::SelectObject(hDC, hOldFont);
    }
}

void CRenderEngine::DrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwTextColor, int iFont, UINT uStyle, DWORD dwTextBKColor)
{
    ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
    if (pstrText == NULL || pManager == NULL) return;

    DrawColor(hDC, rc, dwTextBKColor);
    DrawText(hDC, pManager, rc, pstrText, dwTextColor, iFont, uStyle);
}

void CRenderEngine::DrawHtmlText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, DWORD dwTextColor, RECT* prcLinks, CDuiString* sLinks, int& nLinkRects, int iDefaultFont, UINT uStyle)
{
    // 考虑到在xml编辑器中使用<>符号不方便，可以使用{}符号代替
    // 支持标签嵌套（如<l><b>text</b></l>），但是交叉嵌套是应该避免的（如<l><b>text</l></b>）
    // The string formatter supports a kind of "mini-html" that consists of various short tags:
    //
    //   Bold:             <b>text</b>
    //   Color:            <c #xxxxxx>text</c>  where x = RGB in hex
    //   Font:             <f x>text</f>        where x = font id
    //   Italic:           <i>text</i>
    //   Image:            <i x y z>            where x = image name and y = imagelist num and z(optional) = imagelist id
    //   Link:             <a x>text</a>        where x(optional) = link content, normal like app:notepad or http:www.xxx.com
    //   NewLine           <n>                  
    //   Paragraph:        <p x>text</p>        where x = extra pixels indent in p
    //   Raw Text:         <r>text</r>
    //   Selected:         <s>text</s>
    //   Underline:        <u>text</u>
    //   X Indent:         <x i>                where i = hor indent in pixels
    //   Y Indent:         <y i>                where i = ver indent in pixels 
	//   Vertical align    <v x>				where x = top or x = center or x = bottom

    ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
    if( pstrText == NULL || pManager == NULL ) return;
    if( ::IsRectEmpty(&rc) ) return;

    bool bDraw = (uStyle & DT_CALCRECT) == 0;

    CDuiPtrArray aFontArray(10);
    CDuiPtrArray aColorArray(10);
    CDuiPtrArray aPIndentArray(10);
	CDuiPtrArray aVAlignArray(10);

    RECT rcClip = { 0 };
    ::GetClipBox(hDC, &rcClip);
    HRGN hOldRgn = ::CreateRectRgnIndirect(&rcClip);
    HRGN hRgn = ::CreateRectRgnIndirect(&rc);
    if( bDraw ) ::ExtSelectClipRgn(hDC, hRgn, RGN_AND);


    TEXTMETRIC* pTm = &pManager->GetFontInfo(iDefaultFont)->tm;
    HFONT hOldFont = (HFONT) ::SelectObject(hDC, pManager->GetFontInfo(iDefaultFont)->hFont);
    ::SetBkMode(hDC, TRANSPARENT);
    ::SetTextColor(hDC, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));
    DWORD dwBkColor = pManager->GetDefaultSelectedBkColor();
    ::SetBkColor(hDC, RGB(GetBValue(dwBkColor), GetGValue(dwBkColor), GetRValue(dwBkColor)));

    // If the drawstyle include a alignment, we'll need to first determine the text-size so
    // we can draw it at the correct position...
	if( ((uStyle & DT_CENTER) != 0 || (uStyle & DT_RIGHT) != 0 || (uStyle & DT_VCENTER) != 0 || (uStyle & DT_BOTTOM) != 0) && (uStyle & DT_CALCRECT) == 0 ) {
		RECT rcText = { 0, 0, 9999, 100 };
		if ((uStyle & DT_SINGLELINE) == 0) {
			rcText.right = rc.right - rc.left;
			rcText.bottom = rc.bottom - rc.top;
		}
		int nLinks = 0;
		DrawHtmlText(hDC, pManager, rcText, pstrText, dwTextColor, NULL, NULL, nLinks, iDefaultFont, uStyle | DT_CALCRECT & ~DT_CENTER & ~DT_RIGHT & ~DT_VCENTER & ~DT_BOTTOM);
		if( (uStyle & DT_SINGLELINE) != 0 ){
			if( (uStyle & DT_CENTER) != 0 ) {
				rc.left = rc.left + ((rc.right - rc.left) / 2) - ((rcText.right - rcText.left) / 2);
				rc.right = rc.left + (rcText.right - rcText.left);
			}
			if( (uStyle & DT_RIGHT) != 0 ) {
				rc.left = rc.right - (rcText.right - rcText.left);
			}
		}
		if( (uStyle & DT_VCENTER) != 0 ) {
			rc.top = rc.top + ((rc.bottom - rc.top) / 2) - ((rcText.bottom - rcText.top) / 2);
			rc.bottom = rc.top + (rcText.bottom - rcText.top);
		}
		if( (uStyle & DT_BOTTOM) != 0 ) {
			rc.top = rc.bottom - (rcText.bottom - rcText.top);
		}
	}

    bool bHoverLink = false;
    CDuiString sHoverLink;
    POINT ptMouse = pManager->GetMousePos();
    for( int i = 0; !bHoverLink && i < nLinkRects; i++ ) {
        if( ::PtInRect(prcLinks + i, ptMouse) ) {
            sHoverLink = *(CDuiString*)(sLinks + i);
            bHoverLink = true;
        }
    }

    POINT pt = { rc.left, rc.top };
    int iLinkIndex = 0;
	int cxLine = 0;
    int cyLine = pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
    int cyMinHeight = 0;
    int cxMaxWidth = 0;
    POINT ptLinkStart = { 0 };
    bool bLineEnd = false;
    bool bInRaw = false;
    bool bInLink = false;
    bool bInSelected = false;
    int iLineLinkIndex = 0;

    // 排版习惯是图文底部对齐，所以每行绘制都要分两步，先计算高度，再绘制
    CDuiPtrArray aLineFontArray;
    CDuiPtrArray aLineColorArray;
    CDuiPtrArray aLinePIndentArray;
	CDuiPtrArray aLineVAlignArray;
    LPCTSTR pstrLineBegin = pstrText;
    bool bLineInRaw = false;
    bool bLineInLink = false;
    bool bLineInSelected = false;
	UINT iVAlign = DT_BOTTOM;
	int cxLineWidth = 0;
    int cyLineHeight = 0;
	int cxOffset = 0;
    bool bLineDraw = false; // 行的第二阶段：绘制
    while( *pstrText != _T('\0') ) {
        if( pt.x >= rc.right || *pstrText == _T('\n') || bLineEnd ) {
            if( *pstrText == _T('\n') ) pstrText++;
            if( bLineEnd ) bLineEnd = false;
            if( !bLineDraw ) {
                if( bInLink && iLinkIndex < nLinkRects ) {
                    ::SetRect(&prcLinks[iLinkIndex++], ptLinkStart.x, ptLinkStart.y, MIN(pt.x, rc.right), pt.y + cyLine);
                    CDuiString *pStr1 = (CDuiString*)(sLinks + iLinkIndex - 1);
                    CDuiString *pStr2 = (CDuiString*)(sLinks + iLinkIndex);
                    *pStr2 = *pStr1;
                }
                for( int i = iLineLinkIndex; i < iLinkIndex; i++ ) {
                    prcLinks[i].bottom = pt.y + cyLine;
                }
                if( bDraw ) {
                    bInLink = bLineInLink;
                    iLinkIndex = iLineLinkIndex;
                }
            }
            else {
                if( bInLink && iLinkIndex < nLinkRects ) iLinkIndex++;
                bLineInLink = bInLink;
                iLineLinkIndex = iLinkIndex;
            }
            if( (uStyle & DT_SINGLELINE) != 0 && (!bDraw || bLineDraw) ) 
				break;
            if( bDraw ) bLineDraw = !bLineDraw; // !
            pt.x = rc.left;
			cxOffset = 0;
			if (bLineDraw) {
				if( (uStyle & DT_SINGLELINE) == 0 && (uStyle & DT_CENTER) != 0 ) {
					cxOffset = (rc.right - rc.left - cxLineWidth)/2;
				}
				else if( (uStyle & DT_SINGLELINE) == 0 && (uStyle & DT_RIGHT) != 0) {
					cxOffset = rc.right - rc.left - cxLineWidth;
				}
			}
            else {
				pt.y += cyLine;
			}
            if( pt.y > rc.bottom && bDraw ) 
				break;
            ptLinkStart = pt;
            cyLine = pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1);
            if( pt.x >= rc.right )
				break;
        }
        else if( !bInRaw && ( *pstrText == _T('<') || *pstrText == _T('{') )
            && ( pstrText[1] >= _T('a') && pstrText[1] <= _T('z') )
            && ( pstrText[2] == _T(' ') || pstrText[2] == _T('>') || pstrText[2] == _T('}') ) ) {
                pstrText++;
                LPCTSTR pstrNextStart = NULL;
                switch( *pstrText ) {
            case _T('a'):  // Link
                {
                    pstrText++;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    if( iLinkIndex < nLinkRects && !bLineDraw ) {
                        CDuiString *pStr = (CDuiString*)(sLinks + iLinkIndex);
                        pStr->Empty();
                        while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') ) {
                            LPCTSTR pstrTemp = ::CharNext(pstrText);
                            while( pstrText < pstrTemp) {
                                *pStr += *pstrText++;
                            }
                        }
                    }

                    DWORD clrColor = pManager->GetDefaultLinkFontColor();
                    if( bHoverLink && iLinkIndex < nLinkRects ) {
                        CDuiString *pStr = (CDuiString*)(sLinks + iLinkIndex);
                        if( sHoverLink == *pStr ) clrColor = pManager->GetDefaultLinkHoverFontColor();
                    }
                    //else if( prcLinks == NULL ) {
                    //    if( ::PtInRect(&rc, ptMouse) )
                    //        clrColor = pManager->GetDefaultLinkHoverFontColor();
                    //}
                    aColorArray.Add((LPVOID)clrColor);
                    ::SetTextColor(hDC,  RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                    TFontInfo* pFontInfo = pManager->GetFontInfo(iDefaultFont);
                    if( aFontArray.GetSize() > 0 ) pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( pFontInfo->bUnderline == false ) {
                        HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
						if( hFont == NULL ) {
							hFont = pManager->AddFont(g_iFontID, pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
							g_iFontID += 1;
						}
                        pFontInfo = pManager->GetFontInfo(hFont);
                        aFontArray.Add(pFontInfo);
                        pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->hFont);
                        cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                    ptLinkStart = pt;
                    bInLink = true;
                }
                break;
            case _T('b'):  // Bold
                {
                    pstrText++;
                    TFontInfo* pFontInfo = pManager->GetFontInfo(iDefaultFont);
                    if( aFontArray.GetSize() > 0 ) pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( pFontInfo->bBold == false ) {
                        HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, true, pFontInfo->bUnderline, pFontInfo->bItalic);
						if( hFont == NULL ) {
							hFont = pManager->AddFont(g_iFontID, pFontInfo->sFontName, pFontInfo->iSize, true, pFontInfo->bUnderline, pFontInfo->bItalic);
							g_iFontID += 1;
						}
                        pFontInfo = pManager->GetFontInfo(hFont);
                        aFontArray.Add(pFontInfo);
                        pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->hFont);
                        cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                }
                break;
            case _T('c'):  // Color
                {
                    pstrText++;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    if( *pstrText == _T('#')) pstrText++;
                    DWORD clrColor = _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 16);
                    aColorArray.Add((LPVOID)clrColor);
                    ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                }
                break;
            case _T('f'):  // Font
                {
                    pstrText++;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    LPCTSTR pstrTemp = pstrText;
                    int iFont = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                    //if( isdigit(*pstrText) ) { // debug版本会引起异常
                    if( pstrTemp != pstrText ) {
                        TFontInfo* pFontInfo = pManager->GetFontInfo(iFont);
                        aFontArray.Add(pFontInfo);
                        pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->hFont);
                    }
                    else {
                        CDuiString sFontName;
                        int iFontSize = 10;
                        CDuiString sFontAttr;
                        bool bBold = false;
                        bool bUnderline = false;
                        bool bItalic = false;
                        while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') && *pstrText != _T(' ') ) {
                            pstrTemp = ::CharNext(pstrText);
                            while( pstrText < pstrTemp) {
                                sFontName += *pstrText++;
                            }
                        }
                        while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                        if( isdigit(*pstrText) ) {
                            iFontSize = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                        }
                        while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                        while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') ) {
                            pstrTemp = ::CharNext(pstrText);
                            while( pstrText < pstrTemp) {
                                sFontAttr += *pstrText++;
                            }
                        }
                        sFontAttr.MakeLower();
                        if( sFontAttr.Find(_T("bold")) >= 0 ) bBold = true;
                        if( sFontAttr.Find(_T("underline")) >= 0 ) bUnderline = true;
                        if( sFontAttr.Find(_T("italic")) >= 0 ) bItalic = true;
                        HFONT hFont = pManager->GetFont(sFontName, iFontSize, bBold, bUnderline, bItalic);
						if( hFont == NULL ) {
							hFont = pManager->AddFont(g_iFontID, sFontName, iFontSize, bBold, bUnderline, bItalic);
							g_iFontID += 1;
						}
                        TFontInfo* pFontInfo = pManager->GetFontInfo(hFont);
                        aFontArray.Add(pFontInfo);
                        pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->hFont);
                    }
                    cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                }
                break;
            case _T('i'):  // Italic or Image
                {    
                    pstrNextStart = pstrText - 1;
                    pstrText++;
					CDuiString sImageString = pstrText;
                    int iWidth = 0;
                    int iHeight = 0;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    const TImageInfo* pImageInfo = NULL;
                    CDuiString sName;
                    while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') && *pstrText != _T(' ') ) {
                        LPCTSTR pstrTemp = ::CharNext(pstrText);
                        while( pstrText < pstrTemp) {
                            sName += *pstrText++;
                        }
                    }
                    if( sName.IsEmpty() ) { // Italic
                        pstrNextStart = NULL;
                        TFontInfo* pFontInfo = pManager->GetFontInfo(iDefaultFont);
                        if( aFontArray.GetSize() > 0 ) pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                        if( pFontInfo->bItalic == false ) {
                            HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, pFontInfo->bUnderline, true);
							if( hFont == NULL ) {
								hFont = pManager->AddFont(g_iFontID, pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, pFontInfo->bUnderline, true);
								g_iFontID += 1;
							}
                            pFontInfo = pManager->GetFontInfo(hFont);
                            aFontArray.Add(pFontInfo);
                            pTm = &pFontInfo->tm;
                            ::SelectObject(hDC, pFontInfo->hFont);
                            cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                        }
                    }
                    else {
                        while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                        int iImageListNum = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
						if( iImageListNum <= 0 ) iImageListNum = 1;
						while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
						int iImageListIndex = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
						if( iImageListIndex < 0 || iImageListIndex >= iImageListNum ) iImageListIndex = 0;

						if( _tcsstr(sImageString.GetData(), _T("file=\'")) != NULL || _tcsstr(sImageString.GetData(), _T("res=\'")) != NULL ) {
							CDuiString sImageResType;
							CDuiString sImageName;
							LPCTSTR pStrImage = sImageString.GetData();
							CDuiString sItem;
							CDuiString sValue;
							while( *pStrImage != _T('\0') ) {
								sItem.Empty();
								sValue.Empty();
								while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
								while( *pStrImage != _T('\0') && *pStrImage != _T('=') && *pStrImage > _T(' ') ) {
									LPTSTR pstrTemp = ::CharNext(pStrImage);
									while( pStrImage < pstrTemp) {
										sItem += *pStrImage++;
									}
								}
								while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
								if( *pStrImage++ != _T('=') ) break;
								while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
								if( *pStrImage++ != _T('\'') ) break;
								while( *pStrImage != _T('\0') && *pStrImage != _T('\'') ) {
									LPTSTR pstrTemp = ::CharNext(pStrImage);
									while( pStrImage < pstrTemp) {
										sValue += *pStrImage++;
									}
								}
								if( *pStrImage++ != _T('\'') ) break;
								if( !sValue.IsEmpty() ) {
									if( sItem == _T("file") || sItem == _T("res") ) {
										sImageName = sValue;
									}
									else if( sItem == _T("restype") ) {
										sImageResType = sValue;
									}
								}
								if( *pStrImage++ != _T(' ') ) break;
							}

							pImageInfo = pManager->GetImageEx((LPCTSTR)sImageName, sImageResType);
						}
						else
							pImageInfo = pManager->GetImageEx((LPCTSTR)sName);

						if( pImageInfo ) {
							iWidth = pImageInfo->nX;
							iHeight = pImageInfo->nY;
							if( iImageListNum > 1 ) iWidth /= iImageListNum;

                            if( pt.x + iWidth > rc.right && pt.x > rc.left && (uStyle & DT_SINGLELINE) == 0 ) {
                                bLineEnd = true;
								cxLine = pt.x - rc.left;
                            }
                            else {
                                pstrNextStart = NULL;
                                if( bDraw && bLineDraw ) {
                                    CDuiRect rcImage(pt.x + cxOffset, pt.y + cyLineHeight - iHeight, pt.x + + cxOffset + iWidth, pt.y + cyLineHeight);
									iVAlign = DT_BOTTOM;
									if (aVAlignArray.GetSize() > 0) iVAlign = (UINT)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1); 
									if (iVAlign == DT_VCENTER) {
										if( iHeight < cyLineHeight ) { 
											rcImage.bottom -= (cyLineHeight - iHeight) / 2;
											rcImage.top = rcImage.bottom -  iHeight;
										}
									}
									else if (iVAlign == DT_TOP) {
										if( iHeight < cyLineHeight ) { 
											rcImage.bottom = pt.y + iHeight;
											rcImage.top = pt.y;
										}
									}

                                    CDuiRect rcBmpPart(0, 0, iWidth, iHeight);
                                    rcBmpPart.left = iWidth * iImageListIndex;
                                    rcBmpPart.right = iWidth * (iImageListIndex + 1);
                                    CDuiRect rcCorner(0, 0, 0, 0);
                                    DrawImage(hDC, pImageInfo->hBitmap, rcImage, rcImage, rcBmpPart, rcCorner, \
                                        pImageInfo->bAlpha, 255);
                                }

                                cyLine = MAX(iHeight, cyLine);
                                pt.x += iWidth;
								cxMaxWidth = MAX(cxMaxWidth, pt.x);
								cxLine = pt.x - rc.left;
                                cyMinHeight = pt.y + iHeight;
                            }
                        }
                        else pstrNextStart = NULL;
                    }
                }
                break;
            case _T('n'):  // Newline
                {
                    pstrText++;
                    if( (uStyle & DT_SINGLELINE) != 0 ) break;
                    bLineEnd = true;
                }
                break;
            case _T('p'):  // Paragraph
                {
                    pstrText++;
                    if( pt.x > rc.left ) bLineEnd = true;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    int cyLineExtra = (int)_tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                    aPIndentArray.Add((LPVOID)cyLineExtra);
                    cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + cyLineExtra);
                }
                break;
			case _T('v'):  // Vertical Align
				{
					pstrText++;
					while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
					CDuiString sVAlignStyle;
					while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') ) {
						LPCTSTR pstrTemp = ::CharNext(pstrText);
						while( pstrText < pstrTemp) {
							sVAlignStyle += *pstrText++;
						}
					}

					UINT iVAlign = DT_BOTTOM;
					if (sVAlignStyle.CompareNoCase(_T("center")) == 0) iVAlign = DT_VCENTER;
					else if (sVAlignStyle.CompareNoCase(_T("top")) == 0) iVAlign = DT_TOP;
					aVAlignArray.Add((LPVOID)iVAlign);
				}
				break;
            case _T('r'):  // Raw Text
                {
                    pstrText++;
                    bInRaw = true;
                }
                break;
            case _T('s'):  // Selected text background color
                {
                    pstrText++;
                    bInSelected = !bInSelected;
                    if( bDraw && bLineDraw ) {
                        if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
                        else ::SetBkMode(hDC, TRANSPARENT);
                    }
                }
                break;
            case _T('u'):  // Underline text
                {
                    pstrText++;
                    TFontInfo* pFontInfo = pManager->GetFontInfo(iDefaultFont);
                    if( aFontArray.GetSize() > 0 ) pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( pFontInfo->bUnderline == false ) {
                        HFONT hFont = pManager->GetFont(pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
						if( hFont == NULL ) {
							hFont = pManager->AddFont(g_iFontID, pFontInfo->sFontName, pFontInfo->iSize, pFontInfo->bBold, true, pFontInfo->bItalic);
							g_iFontID += 1;
						}
                        pFontInfo = pManager->GetFontInfo(hFont);
                        aFontArray.Add(pFontInfo);
                        pTm = &pFontInfo->tm;
                        ::SelectObject(hDC, pFontInfo->hFont);
                        cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                    }
                }
                break;
            case _T('x'):  // X Indent
                {
                    pstrText++;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    int iWidth = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                    pt.x += iWidth;
                }
                break;
            case _T('y'):  // Y Indent
                {
                    pstrText++;
                    while( *pstrText > _T('\0') && *pstrText <= _T(' ') ) pstrText = ::CharNext(pstrText);
                    cyLine = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
                }
                break;
                }
            if( pstrNextStart != NULL ) pstrText = pstrNextStart;
            else {
                while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') ) pstrText = ::CharNext(pstrText);
                pstrText = ::CharNext(pstrText);
            }
        }
        else if( !bInRaw && ( *pstrText == _T('<') || *pstrText == _T('{') ) && pstrText[1] == _T('/') )
        {
            pstrText++;
            pstrText++;
            switch( *pstrText )
            {
            case _T('c'):
                {
                    pstrText++;
                    aColorArray.Remove(aColorArray.GetSize() - 1);
                    DWORD clrColor = dwTextColor;
                    if( aColorArray.GetSize() > 0 ) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                    ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                }
                break;
            case _T('p'):
                pstrText++;
                if( pt.x > rc.left ) bLineEnd = true;
                aPIndentArray.Remove(aPIndentArray.GetSize() - 1);
                cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                break;
			case _T('v'):
				pstrText++;
 				aVAlignArray.Remove(aVAlignArray.GetSize() - 1);
				break;
            case _T('s'):
                {
                    pstrText++;
                    bInSelected = !bInSelected;
                    if( bDraw && bLineDraw ) {
                        if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
                        else ::SetBkMode(hDC, TRANSPARENT);
                    }
                }
                break;
            case _T('a'):
                {
                    if( iLinkIndex < nLinkRects ) {
                        if( !bLineDraw ) ::SetRect(&prcLinks[iLinkIndex], ptLinkStart.x, ptLinkStart.y, MIN(pt.x, rc.right), pt.y + pTm->tmHeight + pTm->tmExternalLeading);
                        iLinkIndex++;
                    }
                    aColorArray.Remove(aColorArray.GetSize() - 1);
                    DWORD clrColor = dwTextColor;
                    if( aColorArray.GetSize() > 0 ) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                    ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                    bInLink = false;
                }
            case _T('b'):
            case _T('f'):
            case _T('i'):
            case _T('u'):
                {
                    pstrText++;
                    aFontArray.Remove(aFontArray.GetSize() - 1);
                    TFontInfo* pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                    if( pFontInfo == NULL ) pFontInfo = pManager->GetFontInfo(iDefaultFont);
                    if( pTm->tmItalic && pFontInfo->bItalic == false ) {
                        ABC abc;
                        ::GetCharABCWidths(hDC, _T(' '), _T(' '), &abc);
                        pt.x += abc.abcC / 2; // 简单修正一下斜体混排的问题, 正确做法应该是http://support.microsoft.com/kb/244798/en-us
					}
                    pTm = &pFontInfo->tm;
                    ::SelectObject(hDC, pFontInfo->hFont);
                    cyLine = MAX(cyLine, pTm->tmHeight + pTm->tmExternalLeading + (int)aPIndentArray.GetAt(aPIndentArray.GetSize() - 1));
                }
                break;
            }
            while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('}') ) pstrText = ::CharNext(pstrText);
            pstrText = ::CharNext(pstrText);
        }
        else if( !bInRaw &&  *pstrText == _T('<') && pstrText[2] == _T('>') && (pstrText[1] == _T('{')  || pstrText[1] == _T('}')) )
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32(hDC, &pstrText[1], 1, &szSpace);
            if( bDraw && bLineDraw ) {
				iVAlign = DT_BOTTOM;
				if (aVAlignArray.GetSize() > 0) iVAlign = (UINT)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1); 
				if (iVAlign == DT_VCENTER) ::TextOut(hDC, pt.x + cxOffset, pt.y + (cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading)/2, &pstrText[1], 1);
				else if (iVAlign == DT_TOP) ::TextOut(hDC, pt.x + cxOffset, pt.y, &pstrText[1], 1);
				else ::TextOut(hDC, pt.x + cxOffset, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, &pstrText[1], 1);
			}
			pt.x += szSpace.cx;
			cxMaxWidth = MAX(cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText++;pstrText++;pstrText++;
        }
        else if( !bInRaw &&  *pstrText == _T('{') && pstrText[2] == _T('}') && (pstrText[1] == _T('<')  || pstrText[1] == _T('>')) )
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32(hDC, &pstrText[1], 1, &szSpace);
            if( bDraw && bLineDraw ) {
				iVAlign = DT_BOTTOM;
				if (aVAlignArray.GetSize() > 0) iVAlign = (UINT)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1); 
				if (iVAlign == DT_VCENTER) ::TextOut(hDC, pt.x + cxOffset, pt.y + (cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading)/2, &pstrText[1], 1);
				else if (iVAlign == DT_TOP) ::TextOut(hDC, pt.x + cxOffset, pt.y, &pstrText[1], 1);
				else ::TextOut(hDC, pt.x + cxOffset, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, &pstrText[1], 1);
			}
			pt.x += szSpace.cx;
			cxMaxWidth = MAX(cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText++;pstrText++;pstrText++;
        }
        else if( !bInRaw &&  *pstrText == _T(' ') )
        {
            SIZE szSpace = { 0 };
            ::GetTextExtentPoint32(hDC, _T(" "), 1, &szSpace);
            // Still need to paint the space because the font might have
            // underline formatting.
            if( bDraw && bLineDraw ) {
				iVAlign = DT_BOTTOM;
				if (aVAlignArray.GetSize() > 0) iVAlign = (UINT)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1); 
				if (iVAlign == DT_VCENTER) ::TextOut(hDC, pt.x + cxOffset, pt.y + (cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading)/2, _T(" "), 1);
				else if (iVAlign == DT_TOP) ::TextOut(hDC, pt.x + cxOffset, pt.y, _T(" "), 1);
				else ::TextOut(hDC, pt.x + cxOffset, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, _T(" "), 1);
			}
            pt.x += szSpace.cx;
			cxMaxWidth = MAX(cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText++;
        }
        else
        {
            int cchChars = 0;
            int cchSize = 0;
            int cchLastGoodWord = 0;
            int cchLastGoodSize = 0;
            LPCTSTR p = pstrText;
            LPCTSTR pstrNext;
            SIZE szText = { 0 };
            if( !bInRaw && *p == _T('<') || *p == _T('{') ) p++, cchChars++, cchSize++;
            while( *p != _T('\0') && *p != _T('\n') ) {
                // This part makes sure that we're word-wrapping if needed or providing support
                // for DT_END_ELLIPSIS. Unfortunately the GetTextExtentPoint32() call is pretty
                // slow when repeated so often.
                // TODO: Rewrite and use GetTextExtentExPoint() instead!
                if( bInRaw ) {
                    if( ( *p == _T('<') || *p == _T('{') ) && p[1] == _T('/') 
                        && p[2] == _T('r') && ( p[3] == _T('>') || p[3] == _T('}') ) ) {
                            p += 4;
                            bInRaw = false;
                            break;
                    }
                }
                else {
                    if( *p == _T('<') || *p == _T('{') ) break;
                }
                pstrNext = ::CharNext(p);
                cchChars++;
                cchSize += (int)(pstrNext - p);
                szText.cx = cchChars * pTm->tmMaxCharWidth;
                if( pt.x + szText.cx >= rc.right ) {
                    ::GetTextExtentPoint32(hDC, pstrText, cchSize, &szText);
                }
                if( pt.x + szText.cx > rc.right ) {
                    if( pt.x + szText.cx > rc.right && cchChars > 1) {
                        cchChars--;
                        cchSize -= (int)(pstrNext - p);
                    }
                    if( (uStyle & DT_WORDBREAK) != 0 && cchLastGoodWord > 0 ) {
                        cchChars = cchLastGoodWord;
                        cchSize = cchLastGoodSize;                 
                    }
                    if( (uStyle & DT_END_ELLIPSIS) != 0 && cchChars > 0 ) {
                        cchChars -= 1;
                        LPCTSTR pstrPrev = ::CharPrev(pstrText, p);
                        if( cchChars > 0 ) {
                            cchChars -= 1;
                            pstrPrev = ::CharPrev(pstrText, pstrPrev);
                            cchSize -= (int)(p - pstrPrev);
                        }
                        else 
                            cchSize -= (int)(p - pstrPrev);
                        pt.x = rc.right;
                    }
                    bLineEnd = true;
					cxMaxWidth = MAX(cxMaxWidth, pt.x);
                    cxLine = pt.x - rc.left;
                    break;
                }
                if (!( ( p[0] >= _T('a') && p[0] <= _T('z') ) || ( p[0] >= _T('A') && p[0] <= _T('Z') ) )) {
                    cchLastGoodWord = cchChars;
                    cchLastGoodSize = cchSize;
                }
                if( *p == _T(' ') ) {
                    cchLastGoodWord = cchChars;
                    cchLastGoodSize = cchSize;
                }
                p = ::CharNext(p);
            }
            
            ::GetTextExtentPoint32(hDC, pstrText, cchSize, &szText);
            if( bDraw && bLineDraw ) {
				iVAlign = DT_BOTTOM;
				if (aVAlignArray.GetSize() > 0) iVAlign = (UINT)aVAlignArray.GetAt(aVAlignArray.GetSize() - 1); 
				if (iVAlign == DT_VCENTER) ::TextOut(hDC, pt.x + cxOffset, pt.y + (cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading)/2, pstrText, cchSize);
				else if (iVAlign == DT_TOP) ::TextOut(hDC, pt.x + cxOffset, pt.y, pstrText, cchSize);
				else ::TextOut(hDC, pt.x + cxOffset, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, pstrText, cchSize);

				if( pt.x >= rc.right && (uStyle & DT_END_ELLIPSIS) != 0 ) {
					if (iVAlign == DT_VCENTER) ::TextOut(hDC, pt.x + cxOffset + szText.cx, pt.y + (cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading)/2, _T("..."), 3);
					else if (iVAlign == DT_TOP) ::TextOut(hDC, pt.x + cxOffset + szText.cx, pt.y, _T("..."), 3);
					else ::TextOut(hDC, pt.x + cxOffset + szText.cx, pt.y + cyLineHeight - pTm->tmHeight - pTm->tmExternalLeading, _T("..."), 3);
				}
            }
            pt.x += szText.cx;
			cxMaxWidth = MAX(cxMaxWidth, pt.x);
            cxLine = pt.x - rc.left;
            pstrText += cchSize;
        }

        if( pt.x >= rc.right || *pstrText == _T('\n') || *pstrText == _T('\0') ) bLineEnd = true;
        if( bDraw && bLineEnd ) {
            if( !bLineDraw ) {
                aFontArray.Resize(aLineFontArray.GetSize());
                ::CopyMemory(aFontArray.GetData(), aLineFontArray.GetData(), aLineFontArray.GetSize() * sizeof(LPVOID));
                aColorArray.Resize(aLineColorArray.GetSize());
                ::CopyMemory(aColorArray.GetData(), aLineColorArray.GetData(), aLineColorArray.GetSize() * sizeof(LPVOID));
                aPIndentArray.Resize(aLinePIndentArray.GetSize());
                ::CopyMemory(aPIndentArray.GetData(), aLinePIndentArray.GetData(), aLinePIndentArray.GetSize() * sizeof(LPVOID));
				aVAlignArray.Resize(aLineVAlignArray.GetSize());
				::CopyMemory(aVAlignArray.GetData(), aLineVAlignArray.GetData(), aLineVAlignArray.GetSize() * sizeof(LPVOID));

				cxLineWidth = cxLine;
                cyLineHeight = cyLine;
                pstrText = pstrLineBegin;
                bInRaw = bLineInRaw;
                bInSelected = bLineInSelected;

                DWORD clrColor = dwTextColor;
                if( aColorArray.GetSize() > 0 ) clrColor = (int)aColorArray.GetAt(aColorArray.GetSize() - 1);
                ::SetTextColor(hDC, RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
                TFontInfo* pFontInfo = (TFontInfo*)aFontArray.GetAt(aFontArray.GetSize() - 1);
                if( pFontInfo == NULL ) pFontInfo = pManager->GetFontInfo(iDefaultFont);
                pTm = &pFontInfo->tm;
                ::SelectObject(hDC, pFontInfo->hFont);
                if( bInSelected ) ::SetBkMode(hDC, OPAQUE);
            }
            else {
                aLineFontArray.Resize(aFontArray.GetSize());
                ::CopyMemory(aLineFontArray.GetData(), aFontArray.GetData(), aFontArray.GetSize() * sizeof(LPVOID));
                aLineColorArray.Resize(aColorArray.GetSize());
                ::CopyMemory(aLineColorArray.GetData(), aColorArray.GetData(), aColorArray.GetSize() * sizeof(LPVOID));
                aLinePIndentArray.Resize(aPIndentArray.GetSize());
                ::CopyMemory(aLinePIndentArray.GetData(), aPIndentArray.GetData(), aPIndentArray.GetSize() * sizeof(LPVOID));
				aLineVAlignArray.Resize(aVAlignArray.GetSize());
				::CopyMemory(aLineVAlignArray.GetData(), aVAlignArray.GetData(), aVAlignArray.GetSize() * sizeof(LPVOID));

				pstrLineBegin = pstrText;
                bLineInSelected = bInSelected;
                bLineInRaw = bInRaw;
            }
        }

        ASSERT(iLinkIndex<=nLinkRects);
    }

    nLinkRects = iLinkIndex;

    // Return size of text when requested
    if( (uStyle & DT_CALCRECT) != 0 ) {
        rc.bottom = MAX(cyMinHeight, pt.y + cyLine);
        rc.right = MIN(rc.right, cxMaxWidth);
    }

    if( bDraw ) ::SelectClipRgn(hDC, hOldRgn);
    ::DeleteObject(hOldRgn);
    ::DeleteObject(hRgn);

    ::SelectObject(hDC, hOldFont);
}

HBITMAP CRenderEngine::GenerateBitmap(CPaintManagerUI* pManager, RECT rc, CControlUI* pStopControl, DWORD dwFilterColor)
{
	if (pManager == NULL) return NULL;
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;

	bool bUseOffscreenBitmap = true;
	HDC hPaintDC = ::CreateCompatibleDC(pManager->GetPaintDC());
	ASSERT(hPaintDC);
	HBITMAP hPaintBitmap = NULL;
	if (pStopControl == NULL && !pManager->IsLayered()) hPaintBitmap = pManager->GetPaintOffscreenBitmap();
	if( hPaintBitmap == NULL ) {
		bUseOffscreenBitmap = false;
		hPaintBitmap = ::CreateCompatibleBitmap(pManager->GetPaintDC(), rc.right, rc.bottom);
		ASSERT(hPaintBitmap);
	}
	HBITMAP hOldPaintBitmap = (HBITMAP) ::SelectObject(hPaintDC, hPaintBitmap);
	if (!bUseOffscreenBitmap) {
		CControlUI* pRoot = pManager->GetRoot();
		pRoot->Paint(hPaintDC, rc, pStopControl);
	}

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = cx;
	bmi.bmiHeader.biHeight = cy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = cx * cy * sizeof(DWORD);
	LPDWORD pDest = NULL;
	HDC hCloneDC = ::CreateCompatibleDC(pManager->GetPaintDC());
	HBITMAP hBitmap = ::CreateDIBSection(pManager->GetPaintDC(), &bmi, DIB_RGB_COLORS, (LPVOID*) &pDest, NULL, 0);
	ASSERT(hCloneDC);
	ASSERT(hBitmap);
	if( hBitmap != NULL )
	{
		HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
		::BitBlt(hCloneDC, 0, 0, cx, cy, hPaintDC, rc.left, rc.top, SRCCOPY);
		RECT rcClone = {0, 0, cx, cy};
		if (dwFilterColor > 0x00FFFFFF) DrawColor(hCloneDC, rcClone, dwFilterColor);
		::SelectObject(hCloneDC, hOldBitmap);
		::DeleteDC(hCloneDC);  
		::GdiFlush();
	}

	// Cleanup
	::SelectObject(hPaintDC, hOldPaintBitmap);
	if (!bUseOffscreenBitmap) ::DeleteObject(hPaintBitmap);
	::DeleteDC(hPaintDC);

	return hBitmap;
}

HBITMAP CRenderEngine::GenerateBitmap(CPaintManagerUI* pManager, CControlUI* pControl, RECT rc, DWORD dwFilterColor)
{
	if (pManager == NULL || pControl == NULL) return NULL;
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;

    HDC hPaintDC = ::CreateCompatibleDC(pManager->GetPaintDC());
    HBITMAP hPaintBitmap = ::CreateCompatibleBitmap(pManager->GetPaintDC(), rc.right, rc.bottom);
    ASSERT(hPaintDC);
    ASSERT(hPaintBitmap);
    HBITMAP hOldPaintBitmap = (HBITMAP) ::SelectObject(hPaintDC, hPaintBitmap);
    pControl->Paint(hPaintDC, rc, NULL);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = cx;
    bmi.bmiHeader.biHeight = cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = cx * cy * sizeof(DWORD);
    LPDWORD pDest = NULL;
    HDC hCloneDC = ::CreateCompatibleDC(pManager->GetPaintDC());
    HBITMAP hBitmap = ::CreateDIBSection(pManager->GetPaintDC(), &bmi, DIB_RGB_COLORS, (LPVOID*) &pDest, NULL, 0);
    ASSERT(hCloneDC);
    ASSERT(hBitmap);
    if( hBitmap != NULL )
    {
        HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
        ::BitBlt(hCloneDC, 0, 0, cx, cy, hPaintDC, rc.left, rc.top, SRCCOPY);
		RECT rcClone = {0, 0, cx, cy};
		if (dwFilterColor > 0x00FFFFFF) DrawColor(hCloneDC, rcClone, dwFilterColor);
        ::SelectObject(hCloneDC, hOldBitmap);
        ::DeleteDC(hCloneDC);  
        ::GdiFlush();
    }

    // Cleanup
    ::SelectObject(hPaintDC, hOldPaintBitmap);
    ::DeleteObject(hPaintBitmap);
    ::DeleteDC(hPaintDC);

    return hBitmap;
}

SIZE CRenderEngine::GetTextSize( HDC hDC, CPaintManagerUI* pManager , LPCTSTR pstrText, int iFont, UINT uStyle )
{
	SIZE size = {0,0};
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	if( pstrText == NULL || pManager == NULL ) return size;
	::SetBkMode(hDC, TRANSPARENT);
	HFONT hOldFont = (HFONT)::SelectObject(hDC, pManager->GetFont(iFont));
	GetTextExtentPoint32(hDC, pstrText, _tcslen(pstrText) , &size);
	::SelectObject(hDC, hOldFont);
	return size;
}

void CRenderEngine::RestoreAlphaColor(LPBYTE pBits, int bitsWidth, PRECT rc)
{
    for (int i = rc->top; i < rc->bottom; ++i)
    {
        for (int j = rc->left; j < rc->right; ++j)
        {
            int x = (i * bitsWidth + j) * 4;
            if ((pBits[x + 3] == 0) && (pBits[x + 0] != 0 || pBits[x + 1] != 0 || pBits[x + 2] != 0))
                pBits[x + 3] = 255;
        }
    }
}

int GetScaleInt(int iValue, int nScale)
{
    return MulDiv(iValue, nScale, 100);
}

SIZE GetScaleSize(const SIZE& sz, int nScale)
{
    SIZE rtn;
    rtn.cx = MulDiv(sz.cx, nScale, 100);
    rtn.cy = MulDiv(sz.cy, nScale, 100);

    return rtn;
}

RECT GetScaleRect(const RECT& rc, int nScale)
{
    RECT rtn;
    rtn.left = MulDiv(rc.left, nScale, 100);
    rtn.top = MulDiv(rc.top, nScale, 100);
    rtn.right = MulDiv(rc.right, nScale, 100);
    rtn.bottom = MulDiv(rc.bottom, nScale, 100);

    return rtn;
}

bool ParseDrawInfo(CPaintManagerUI* pManager, TDrawInfo& drawInfo, int nScale)
{
    bool bUseRes = false;
    CDuiString sImageName = drawInfo.sDrawString;
    CDuiString sImageResType;
    DWORD dwMask = 0;
    bool bUseHSL = false;

    CDuiString sItem;
    CDuiString sValue;
    LPTSTR pstr = NULL;
    LPCTSTR pstrImage = drawInfo.sDrawString.GetData();
    while (*pstrImage != _T('\0')) {
        sItem.Empty();
        sValue.Empty();
        while (*pstrImage > _T('\0') && *pstrImage <= _T(' ')) pstrImage = ::CharNext(pstrImage);
        while (*pstrImage != _T('\0') && *pstrImage != _T('=') && *pstrImage > _T(' ')) {
            LPTSTR pstrTemp = ::CharNext(pstrImage);
            while (pstrImage < pstrTemp) {
                sItem += *pstrImage++;
            }
        }
        while (*pstrImage > _T('\0') && *pstrImage <= _T(' ')) pstrImage = ::CharNext(pstrImage);
        if (*pstrImage++ != _T('=')) break;
        while (*pstrImage > _T('\0') && *pstrImage <= _T(' ')) pstrImage = ::CharNext(pstrImage);
        if (*pstrImage++ != _T('\'')) break;
        while (*pstrImage != _T('\0') && *pstrImage != _T('\'')) {
            LPTSTR pstrTemp = ::CharNext(pstrImage);
            while (pstrImage < pstrTemp) {
                sValue += *pstrImage++;
            }
        }
        if (*pstrImage++ != _T('\'')) break;
        if (!sValue.IsEmpty()) {
            if (sItem == _T("file")) {
                sImageName = sValue;
            }
            else if (sItem == _T("res")) {
                bUseRes = true;
                sImageName = sValue;
            }
            else if (sItem == _T("restype")) {
                sImageResType = sValue;
            }
            else if (sItem == _T("color")) {
                bUseRes = true;
                sImageResType = RES_TYPE_COLOR;
                sImageName = sValue;
            }
            else if (sItem == _T("dest")) {
                drawInfo.rcDestOffset.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);
                drawInfo.rcDestOffset.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
                drawInfo.rcDestOffset.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
                drawInfo.rcDestOffset.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
            }
            else if (sItem == _T("source")) {
                drawInfo.rcBmpPart.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);
                drawInfo.rcBmpPart.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
                drawInfo.rcBmpPart.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
                drawInfo.rcBmpPart.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
            }
            else if (sItem == _T("corner") || sItem == _T("scale9")) {
                drawInfo.rcScale9.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);
                drawInfo.rcScale9.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
                drawInfo.rcScale9.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
                drawInfo.rcScale9.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
            }
            else if (sItem == _T("mask")) {
                if (sValue[0] == _T('#')) dwMask = _tcstoul(sValue.GetData() + 1, &pstr, 16);
                else dwMask = _tcstoul(sValue.GetData(), &pstr, 16);
            }
            else if (sItem == _T("fade")) {
                drawInfo.uFade = (BYTE)_tcstoul(sValue.GetData(), &pstr, 10);
            }
            else if (sItem == _T("hole")) {
                drawInfo.bHole = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
            else if (sItem == _T("xtiled")) {
                drawInfo.bTiledX = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
            else if (sItem == _T("ytiled")) {
                drawInfo.bTiledY = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
            else if (sItem == _T("hsl")) {
                bUseHSL = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
            else if (sItem == _T("scaled")) {
                drawInfo.bSupportScale = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
        }
        if (*pstrImage++ != _T(' ')) break;
    }

    // 调整DPI资源
    drawInfo.sImageName = sImageName;
    CDuiString scaleImgName = sImageName;
    if (nScale != 100) {
        CDuiString sScale;
        sScale.Format(_T("@%d."), nScale);
        scaleImgName.Replace(_T("."), sScale);
    }

    //if (CPaintManagerUI::IsResourceFileExisted(scaleImgName)) {
    if (drawInfo.bSupportScale) {
        sImageName = scaleImgName;
        drawInfo.rcBmpPart = GetScaleRect(drawInfo.rcBmpPart, nScale);
        drawInfo.rcScale9 = GetScaleRect(drawInfo.rcScale9, nScale);
    }
    if (nScale != 100)
        drawInfo.rcDestOffset = GetScaleRect(drawInfo.rcDestOffset, nScale);

    const TImageInfo* data = NULL;
    if (bUseRes == false) {
        data = pManager->GetImageEx((LPCTSTR)sImageName, NULL, dwMask, bUseHSL);
    }
    else {
        data = pManager->GetImageEx((LPCTSTR)sImageName, (LPCTSTR)sImageResType, dwMask, bUseHSL);
    }
    if (!data) return false;

    drawInfo.pImageInfo = data;
    if (drawInfo.rcBmpPart.left == 0 && drawInfo.rcBmpPart.right == 0 &&
        drawInfo.rcBmpPart.top == 0 && drawInfo.rcBmpPart.bottom == 0) {
        drawInfo.rcBmpPart.right = data->nX;
        drawInfo.rcBmpPart.bottom = data->nY;
    }

    return true;
}

bool ParseDrawImageList(CPaintManagerUI* pManager, TDrawImgList& drawList, int nScale)
{
    bool bUseRes = false;
    CDuiString sImageName = drawList.sDrawString;
    CDuiString sImageResType;
    bool bUseHSL = false;

    CDuiString sItem;
    CDuiString sValue;
    LPTSTR pstr = NULL;
    LPCTSTR pstrImage = drawList.sDrawString.GetData();
    while (*pstrImage != _T('\0')) {
        sItem.Empty();
        sValue.Empty();
        while (*pstrImage > _T('\0') && *pstrImage <= _T(' ')) pstrImage = ::CharNext(pstrImage);
        while (*pstrImage != _T('\0') && *pstrImage != _T('=') && *pstrImage > _T(' ')) {
            LPTSTR pstrTemp = ::CharNext(pstrImage);
            while (pstrImage < pstrTemp) {
                sItem += *pstrImage++;
            }
        }
        while (*pstrImage > _T('\0') && *pstrImage <= _T(' ')) pstrImage = ::CharNext(pstrImage);
        if (*pstrImage++ != _T('=')) break;
        while (*pstrImage > _T('\0') && *pstrImage <= _T(' ')) pstrImage = ::CharNext(pstrImage);
        if (*pstrImage++ != _T('\'')) break;
        while (*pstrImage != _T('\0') && *pstrImage != _T('\'')) {
            LPTSTR pstrTemp = ::CharNext(pstrImage);
            while (pstrImage < pstrTemp) {
                sValue += *pstrImage++;
            }
        }
        if (*pstrImage++ != _T('\'')) break;
        if (!sValue.IsEmpty()) {
            if (sItem == _T("file")) {
                sImageName = sValue;
            }
            else if (sItem == _T("res")) {
                bUseRes = true;
                sImageName = sValue;
            }
            else if (sItem == _T("restype")) {
                sImageResType = sValue;
            }
            else if (sItem == _T("source")) {
                drawList.rcBmpPart.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);
                drawList.rcBmpPart.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
                drawList.rcBmpPart.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
                drawList.rcBmpPart.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
            }
            else if (sItem == _T("count")) {
                drawList.count = _tcstol(sValue.GetData(), &pstr, 10);
            }
            else if (sItem == _T("mask")) {
                if (sValue[0] == _T('#')) drawList.dwMask = _tcstoul(sValue.GetData() + 1, &pstr, 16);
                else drawList.dwMask = _tcstoul(sValue.GetData(), &pstr, 16);
            }
            else if (sItem == _T("hsl")) {
                bUseHSL = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
            else if (sItem == _T("scaled")) {
                drawList.bSupportScale = (_tcscmp(sValue.GetData(), _T("true")) == 0);
            }
        }
        if (*pstrImage++ != _T(' ')) break;
    }

    // 调整DPI资源
    drawList.sImageName = sImageName;
    CDuiString scaleImgName = sImageName;
    if (nScale != 100) {
        CDuiString sScale;
        sScale.Format(_T("@%d."), nScale);
        scaleImgName.Replace(_T("."), sScale);
    }

    //if (CPaintManagerUI::IsResourceFileExisted(scaleImgName)) {
    if (drawList.bSupportScale) {
        sImageName = scaleImgName;
        drawList.rcBmpPart = GetScaleRect(drawList.rcBmpPart, nScale);
    }

    const TImageInfo* data = NULL;
    if (bUseRes == false) {
        data = pManager->GetImageEx((LPCTSTR)sImageName, NULL, drawList.dwMask, bUseHSL);
    }
    else {
        data = pManager->GetImageEx((LPCTSTR)sImageName, (LPCTSTR)sImageResType, drawList.dwMask, bUseHSL);
    }
    if (!data) return false;

    drawList.pImageInfo = data;
    if (::IsRectEmpty(&drawList.rcBmpPart))
    {
        drawList.rcBmpPart.left = drawList.rcBmpPart.top = 0;
        drawList.rcBmpPart.right = drawList.pImageInfo->nX;
        drawList.rcBmpPart.bottom = drawList.pImageInfo->nY;
    }
    return true;
}

} // namespace DuiLib
