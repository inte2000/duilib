#include "StdAfx.h"
#include "UIText.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CTextUI)

	CTextUI::CTextUI() : m_nLinks(0), m_nHoverLink(-1)
	{
		m_uTextStyle = DT_WORDBREAK;
		m_rcTextPadding.left = 2;
		m_rcTextPadding.right = 2;
		::ZeroMemory(m_rcLinks, sizeof(m_rcLinks));
	}

	CTextUI::~CTextUI()
	{
	}

	LPCTSTR CTextUI::GetClass() const
	{
		return DUI_CTR_TEXT;
	}

	LPVOID CTextUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_TEXT) == 0 ) return static_cast<CTextUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	void CTextUI::SetText(LPCTSTR pstrText)
	{
		if (m_sText != pstrText)
		{
			CLabelUI::SetText(pstrText);
			if (m_pParent != NULL)
				m_pParent->NeedUpdate();
		}
	}

	UINT CTextUI::GetControlFlags() const
	{
		if( IsEnabled() && m_nLinks > 0 ) return UIFLAG_SETCURSOR;
		else return 0;
	}

	CDuiString* CTextUI::GetLinkContent(int iIndex)
	{
		if( iIndex >= 0 && iIndex < m_nLinks ) return &m_sLinks[iIndex];
		return NULL;
	}
    //orbit add 
    CDuiString* CTextUI::GetLinkContent(const POINT& ptMouse)
    {
        for (int i = 0; i < m_nLinks; i++)
        {
            if (::PtInRect(&m_rcLinks[i], ptMouse))
            {
                return GetLinkContent(i);
            }
        }

        return NULL;
    }

	void CTextUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR ) {
			for( int i = 0; i < m_nLinks; i++ ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, IDC_HAND));
					return;
				}
			}
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK && IsEnabled() ) {
			for( int i = 0; i < m_nLinks; i++ ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					Invalidate();
					return;
				}
			}
		}
		if( event.Type == UIEVENT_BUTTONUP && IsEnabled() ) {
			for( int i = 0; i < m_nLinks; i++ ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_LINK, i);
					return;
				}
			}
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		// When you move over a link
		if( m_nLinks > 0 && event.Type == UIEVENT_MOUSEMOVE && IsEnabled() ) {
			int nHoverLink = -1;
			for( int i = 0; i < m_nLinks; i++ ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					nHoverLink = i;
					break;
				}
			}

			if(m_nHoverLink != nHoverLink) {
				m_nHoverLink = nHoverLink;
				Invalidate();
				return;
			}      
		}
		if( event.Type == UIEVENT_MOUSELEAVE ) {
			if( m_nLinks > 0 && IsEnabled() ) {
				if(m_nHoverLink != -1) {
                    if( !::PtInRect(&m_rcLinks[m_nHoverLink], event.ptMouse) ) {
                        m_nHoverLink = -1;
                        Invalidate();
                        if (m_pManager) m_pManager->RemoveMouseLeaveNeeded(this);
                    }
                    else {
                        if (m_pManager) m_pManager->AddMouseLeaveNeeded(this);
                        return;
                    }
				}
			}
		}

		CLabelUI::DoEvent(event);
	}

    SIZE CTextUI::EstimateSize(SIZE szAvailable)
    {
#if 0
        CDuiString sText = GetText();
        RECT rcTextPadding = GetTextPadding();

        RECT rcText = { 0, 0, (m_cxyFixed.cx != 0) ? GetManager()->GetDPIObj()->Scale(m_cxyFixed.cx) : 9999,
                              (m_cxyFixed.cy != 0) ? GetManager()->GetDPIObj()->Scale(m_cxyFixed.cy) : 9999 };

        rcText.left += rcTextPadding.left;
        rcText.right -= rcTextPadding.right;

        if (m_bShowHtml) {
            int nLinks = 0;
            CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), m_pManager, rcText, sText, m_dwTextColor, NULL, NULL, nLinks, m_iFont, DT_CALCRECT | m_uTextStyle);
        }
        else {
            CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, sText, m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle);
        }
        SIZE cXY = { rcText.right - rcText.left + rcTextPadding.left + rcTextPadding.right,
            rcText.bottom - rcText.top + rcTextPadding.top + rcTextPadding.bottom };
        if ((m_cxyFixed.cx == 0) || (m_cxyFixed.cx < cXY.cx))
            m_cxyFixed.cx = cXY.cx;
        if (m_cxyFixed.cy == 0)
            m_cxyFixed.cy = cXY.cy;
#endif
        return CLabelUI::EstimateSize(szAvailable);
    }

	void CTextUI::PaintText(HDC hDC)
	{
		CDuiString sText = GetText();
		if( sText.IsEmpty() ) {
			m_nLinks = 0;
			return;
		}

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		m_nLinks = lengthof(m_rcLinks);
		RECT rc = m_rcItem;
        RECT rcTextPadding = GetTextPadding();
		rc.left += rcTextPadding.left;
		rc.right -= rcTextPadding.right;
		rc.top += rcTextPadding.top;
		rc.bottom -= rcTextPadding.bottom;
		if( IsEnabled() ) {
			if( m_bShowHtml )
				CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, m_dwTextColor, \
				m_rcLinks, m_sLinks, m_nLinks, m_iFont, m_uTextStyle);
			else
				CRenderEngine::DrawText(hDC, m_pManager, rc, sText, m_dwTextColor, \
				m_iFont, m_uTextStyle);
		}
		else {
			if( m_bShowHtml )
				CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, m_dwDisabledTextColor, \
				m_rcLinks, m_sLinks, m_nLinks, m_iFont, m_uTextStyle);
			else
				CRenderEngine::DrawText(hDC, m_pManager, rc, sText, m_dwDisabledTextColor, \
				m_iFont, m_uTextStyle);
		}
	}

}
