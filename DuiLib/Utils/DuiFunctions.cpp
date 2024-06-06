#include "stdafx.h"
#include "DuiFunctions.h"

//
namespace DuiLib
{
	BOOL SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR lpFileName)
	{
		HDC					hDC;
		DWORD				dwWritten, dwPaletteSize = 0;
		BITMAP				Bitmap;
		BITMAPFILEHEADER	bmfHdr;
		BITMAPINFOHEADER	bi;
		LPBITMAPINFOHEADER	lpbi;
		HANDLE				fh, hDib, hPal, hOldPal = NULL;

		//����λͼ��Ϣͷ�ṹ
		GetObject(hBitmap, sizeof(BITMAP), &Bitmap);
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = Bitmap.bmWidth;
		bi.biHeight = Bitmap.bmHeight;
		bi.biPlanes = Bitmap.bmPlanes;
		bi.biBitCount = Bitmap.bmBitsPixel;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = Bitmap.bmWidthBytes * Bitmap.bmHeight;
		bi.biXPelsPerMeter = 0;
		bi.biYPelsPerMeter = 0;
		bi.biClrUsed = 0;
		bi.biClrImportant = 0;

		if (bi.biBitCount <= 8)
		{
			dwPaletteSize = (1 << bi.biBitCount) * sizeof(RGBQUAD);
		}

		//Ϊλͼ���ݷ����ڴ�
		hDib = GlobalAlloc(GHND, bi.biSizeImage + dwPaletteSize + sizeof(BITMAPINFOHEADER));

		lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
		*lpbi = bi;

		// �����ɫ��   
		hPal = GetStockObject(DEFAULT_PALETTE);
		if (hPal)
		{
			hDC = ::GetDC(NULL);
			hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
			RealizePalette(hDC);
		}

		// ��ȡ�õ�ɫ�����µ�����ֵ
		GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

		//�ָ���ɫ��   
		if (hOldPal)
		{
			SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
			RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}

		//����λͼ�ļ�    
		fh = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

		if (fh == INVALID_HANDLE_VALUE)
			return FALSE;

		// ����λͼ�ļ�ͷ
		bmfHdr.bfType = 0x4D42;
		bmfHdr.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + bi.biSizeImage;
		bmfHdr.bfReserved1 = 0;
		bmfHdr.bfReserved2 = 0;
		bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

		// д��λͼ�ļ�ͷ
		WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

		// д��λͼ�ļ���������
		WriteFile(fh, (LPSTR)lpbi, bmfHdr.bfSize, &dwWritten, NULL);

		//�����ڴ����  
		GlobalUnlock(hDib);
		GlobalFree(hDib);
		CloseHandle(fh);
		return TRUE;
	}

	BOOL ScreenToClientEx(HWND hWnd, LPRECT lpRect)
	{
		if (!::ScreenToClient(hWnd, (LPPOINT)lpRect))
			return FALSE;
		return ::ScreenToClient(hWnd, ((LPPOINT)lpRect) + 1);
	}

	BOOL ClientToScreenEx(HWND hWnd, LPRECT lpRect)
	{
		if (!::ClientToScreen(hWnd, (LPPOINT)lpRect))
			return FALSE;
		return ::ClientToScreen(hWnd, ((LPPOINT)lpRect) + 1);
	}


	HBITMAP GetWindowSnapBitmap(HWND hWnd)
	{
		HDC hDC = ::GetWindowDC(hWnd);

		RECT rectSrc, rcAdj;
		::GetWindowRect(hWnd, &rcAdj);
		::GetClientRect(hWnd, &rectSrc);
		ScreenToClientEx(hWnd, &rcAdj);

		::OffsetRect(&rectSrc, -rcAdj.left, -rcAdj.top);	// convert to window (not client) relative coordinates
		HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, rectSrc.right - rectSrc.left, rectSrc.bottom - rectSrc.top);
		if (hBitmap != NULL)
		{
			HDC memDC = ::CreateCompatibleDC(hDC);
			if (memDC != NULL)
			{
				HBITMAP hOldBmp = (HBITMAP)::SelectObject(memDC, hBitmap);
				::BitBlt(memDC, 0, 0, rectSrc.right - rectSrc.left, rectSrc.bottom - rectSrc.top, hDC, rectSrc.left, rectSrc.top, SRCCOPY);
				::SelectObject(memDC, hOldBmp);
				::DeleteDC(memDC);
			}
		}

		::ReleaseDC(hWnd, hDC);

		return hBitmap;
	}

	HBITMAP GetDCSnapBitmap(HDC hDC, const RECT rc)
	{
		HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, rc.right - rc.left, rc.bottom - rc.top);
		if (hBitmap != NULL)
		{
			HDC memDC = ::CreateCompatibleDC(hDC);
			if (memDC != NULL)
			{
				HBITMAP hOldBmp = (HBITMAP)::SelectObject(memDC, hBitmap);
				::BitBlt(memDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hDC, 0, 0, SRCCOPY);
				::SelectObject(memDC, hOldBmp);
				::DeleteDC(memDC);
			}
		}

		return hBitmap;
	}
} // namespace DuiLib