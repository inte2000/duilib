#include "StdAfx.h"
#include "UIGifAnim.h"


namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CGifAnimUI)

	CGifAnimUI::CGifAnimUI(void)
	{
		m_pGifImage			=	NULL;
		m_pPropertyItem		=	NULL;
		m_nFrameCount		=	0;	
		m_nFramePosition	=	0;	
		m_bIsAutoPlay		=	true;
		m_bIsAutoSize		=	false;
		m_bIsPlaying		=	false;
	}


	CGifAnimUI::~CGifAnimUI(void)
	{
		DeleteGif();
		m_pManager->KillTimer( this, EVENT_TIEM_ID );

	}

	LPCTSTR CGifAnimUI::GetClass() const
	{
		return DUI_CTR_GIFANIM;
	}

	LPVOID CGifAnimUI::GetInterface( LPCTSTR pstrName )
	{
		if( _tcscmp(pstrName, DUI_CTR_GIFANIM) == 0 ) return static_cast<CGifAnimUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	void CGifAnimUI::DoInit()
	{
		InitGifImage();
	}

	bool CGifAnimUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl)
	{
		if ( NULL == m_pGifImage )
		{		
			InitGifImage();
		}
		DrawFrame( hDC );
        return true;
	}

	void CGifAnimUI::DoEvent( TEventUI& event )
	{
		if( event.Type == UIEVENT_TIMER )
			OnTimer( (UINT_PTR)event.wParam );
	}

	void CGifAnimUI::SetVisible(bool bVisible /* = true */)
	{
		CControlUI::SetVisible(bVisible);
		if (bVisible)
			PlayGif();
		else
			StopGif();
	}

	void CGifAnimUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("bkimage")) == 0 ) SetBkImage(pstrValue);
		else if( _tcscmp(pstrName, _T("autoplay")) == 0 ) {
			SetAutoPlay(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcscmp(pstrName, _T("autosize")) == 0 ) {
			SetAutoSize(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else
			CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CGifAnimUI::SetBkImage(LPCTSTR pStrImage)
	{
		if( m_sBkImage == pStrImage || NULL == pStrImage) return;

		m_sBkImage = pStrImage;

		StopGif();
		DeleteGif();

		Invalidate();

	}

	LPCTSTR CGifAnimUI::GetBkImage()
	{
		return m_sBkImage.GetData();
	}

	void CGifAnimUI::SetAutoPlay(bool bIsAuto)
	{
		m_bIsAutoPlay = bIsAuto;
	}

	bool CGifAnimUI::IsAutoPlay() const
	{
		return m_bIsAutoPlay;
	}

	void CGifAnimUI::SetAutoSize(bool bIsAuto)
	{
		m_bIsAutoSize = bIsAuto;
	}

	bool CGifAnimUI::IsAutoSize() const
	{
		return m_bIsAutoSize;
	}

	void CGifAnimUI::PlayGif()
	{
		if (m_bIsPlaying || m_pGifImage == NULL)
		{
			return;
		}

		long lPause = ((long*) m_pPropertyItem->value)[m_nFramePosition] * 10;
		if ( lPause == 0 ) lPause = 100;
		m_pManager->SetTimer( this, EVENT_TIEM_ID, lPause );

		m_bIsPlaying = true;
	}

	void CGifAnimUI::PauseGif()
	{
		if (!m_bIsPlaying || m_pGifImage == NULL)
		{
			return;
		}

		m_pManager->KillTimer(this, EVENT_TIEM_ID);
		this->Invalidate();
		m_bIsPlaying = false;
	}

	void CGifAnimUI::StopGif()
	{
		if (!m_bIsPlaying)
		{
			return;
		}

		m_pManager->KillTimer(this, EVENT_TIEM_ID);
		m_nFramePosition = 0;
		this->Invalidate();
		m_bIsPlaying = false;
	}

	void CGifAnimUI::InitGifImage()
	{
		//m_pGifImage = LoadGifFromFile(GetBkImage());
		m_pGifImage = CRenderEngine::GdiplusLoadImage(GetBkImage(), m_nScale);

		if ( NULL == m_pGifImage ) return;
		UINT nCount	= 0;
		nCount	=	m_pGifImage->GetFrameDimensionsCount();
		GUID* pDimensionIDs	=	new GUID[ nCount ];
		m_pGifImage->GetFrameDimensionsList( pDimensionIDs, nCount );
		m_nFrameCount	=	m_pGifImage->GetFrameCount( &pDimensionIDs[0] );
		int nSize		=	m_pGifImage->GetPropertyItemSize( PropertyTagFrameDelay );
		m_pPropertyItem	=	(Gdiplus::PropertyItem*) malloc( nSize );
		m_pGifImage->GetPropertyItem( PropertyTagFrameDelay, nSize, m_pPropertyItem );
		delete[] pDimensionIDs;
		pDimensionIDs = NULL;

		if (m_bIsAutoSize)
		{
			SetFixedWidth(m_pGifImage->GetWidth());
			SetFixedHeight(m_pGifImage->GetHeight());
		}
		if (m_bIsAutoPlay && nSize > 0)
		{
			PlayGif();
		}
	}

	void CGifAnimUI::DeleteGif()
	{
		if ( m_pGifImage != NULL )
		{
			delete m_pGifImage;
			m_pGifImage = NULL;
		}

		if ( m_pPropertyItem != NULL )
		{
			free( m_pPropertyItem );
			m_pPropertyItem = NULL;
		}
		m_nFrameCount		=	0;	
		m_nFramePosition	=	0;	
	}

	void CGifAnimUI::OnTimer( UINT_PTR idEvent )
	{
		if ( idEvent != EVENT_TIEM_ID )
			return;
		m_pManager->KillTimer( this, EVENT_TIEM_ID );
		this->Invalidate();

		m_nFramePosition = (++m_nFramePosition) % m_nFrameCount;

		long lPause = ((long*) m_pPropertyItem->value)[m_nFramePosition] * 10;
		if ( lPause == 0 ) lPause = 100;
		m_pManager->SetTimer( this, EVENT_TIEM_ID, lPause );
	}

	void CGifAnimUI::DrawFrame( HDC hDC )
	{
		if ( NULL == hDC || NULL == m_pGifImage ) return;
		GUID pageGuid = Gdiplus::FrameDimensionTime;
		Gdiplus::Graphics graphics( hDC );
		graphics.DrawImage( m_pGifImage, (int)m_rcItem.left, m_rcItem.top, m_rcItem.right-m_rcItem.left, m_rcItem.bottom-m_rcItem.top );
		m_pGifImage->SelectActiveFrame( &pageGuid, m_nFramePosition );
	}

}
