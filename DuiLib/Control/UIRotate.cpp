#include "StdAfx.h"
#include "UIRotate.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CRotateUI)

	CRotateUI::CRotateUI() : m_fCurAngle(0.0f), m_pBkimage(NULL)
	{
	}

	CRotateUI::~CRotateUI()
	{
		if(m_pManager) m_pManager->KillTimer(this, RING_TIMERID);

		DeleteImage();
	}

	LPCTSTR CRotateUI::GetClass() const
	{
		return DUI_CTR_ROTATE;
	}

	LPVOID CRotateUI::GetInterface( LPCTSTR pstrName )
	{
		if( _tcscmp(pstrName, DUI_CTR_ROTATE) == 0 ) return static_cast<CRotateUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	void CRotateUI::SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue )
	{
		if( _tcscmp(pstrName, _T("bkimage")) == 0 ) SetBkImage(pstrValue);
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CRotateUI::SetBkImage( LPCTSTR pStrImage )
	{
		DeleteImage();
		InitImage(pStrImage);
		Invalidate();
	}

	void CRotateUI::PaintBkImage( HDC hDC )
	{
		if(m_pBkimage != NULL) {
			RECT rcItem = m_rcItem;
			int iWidth = rcItem.right - rcItem.left;
			int iHeight = rcItem.bottom - rcItem.top;
			Gdiplus::PointF centerPos((Gdiplus::REAL)(rcItem.left + iWidth/2), (Gdiplus::REAL)(rcItem.top + iHeight/2));

			Gdiplus::Graphics graphics(hDC);
			graphics.TranslateTransform(centerPos.X,centerPos.Y);
			graphics.RotateTransform(m_fCurAngle);
			graphics.TranslateTransform(-centerPos.X, -centerPos.Y);//还原源点
			graphics.DrawImage(m_pBkimage,rcItem.left,rcItem.top,iWidth,iHeight);
		}
	}

	void CRotateUI::DoEvent( TEventUI& event )
	{
		if( event.Type == UIEVENT_TIMER && event.wParam == RING_TIMERID ) {
			if(m_fCurAngle > 359) {
				m_fCurAngle = 0;
			}
			m_fCurAngle += 36.0;
			Invalidate();
		}
		else {
			CLabelUI::DoEvent(event);
		}
	}

	void CRotateUI::InitImage(LPCTSTR pStrImage)
	{
		m_strImgName = pStrImage;
		m_pBkimage = CRenderEngine::GdiplusLoadImage(pStrImage, m_nScale);
		if ( NULL == m_pBkimage ) return;
		if(m_pManager) m_pManager->SetTimer(this, RING_TIMERID, 100);
	}

	void CRotateUI::DeleteImage()
	{
		if ( m_pBkimage != NULL )
		{
			delete m_pBkimage;
			m_pBkimage = NULL;
		}
	}
}
