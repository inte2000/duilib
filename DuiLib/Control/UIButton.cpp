#include "stdafx.h"
#include "UIButton.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CButtonUI)

	CButtonUI::CButtonUI()
		: m_uButtonState(0)
		, m_iHotFont(-1)
		, m_iPushedFont(-1)
		, m_iFocusedFont(-1)
		, m_dwHotTextColor(0)
		, m_dwPushedTextColor(0)
		, m_dwFocusedTextColor(0)
		, m_dwHotBkColor(0)
		, m_dwPushedBkColor(0)
		, m_dwDisabledBkColor(0)
		, m_dwHotBorderColor(0)
	{
		m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
	}

	LPCTSTR CButtonUI::GetClass() const
	{
		return DUI_CTR_BUTTON;
	}

	LPVOID CButtonUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_BUTTON) == 0 ) return static_cast<CButtonUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CButtonUI::GetControlFlags() const
	{
		return (IsKeyboardEnabled() ? UIFLAG_TABSTOP : 0) | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
	}

	void CButtonUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_KEYDOWN )
		{
			if (IsKeyboardEnabled() && IsEnabled()) {
				if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
					Activate();
					return;
				}
			}
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
		{
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				Invalidate();
				//if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_BUTTONDOWN);
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				if( ::PtInRect(&m_rcItem, event.ptMouse) ) m_uButtonState |= UISTATE_PUSHED;
				else m_uButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled()) Activate();
				m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			if( IsContextMenuUsed() && IsEnabled()) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
            if( ::PtInRect(&m_rcItem, event.ptMouse ) ) {
                if( IsEnabled() ) {
                    if( (m_uButtonState & UISTATE_HOT) == 0  ) {
                        m_uButtonState |= UISTATE_HOT;
                        Invalidate();
						//if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER);
                    }
                }
            }
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
            if( !::PtInRect(&m_rcItem, event.ptMouse ) ) {
                if( IsEnabled() ) {
                    if( (m_uButtonState & UISTATE_HOT) != 0  ) {
                        m_uButtonState &= ~UISTATE_HOT;
                        Invalidate();
						//if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE);
                    }
                }
            }
		}

		CLabelUI::DoEvent(event);
	}

	bool CButtonUI::Activate()
	{
		if( !CLabelUI::Activate() ) return false;
		if( m_pManager != NULL ) m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
		return true;
	}

	void CButtonUI::SetEnabled(bool bEnable)
	{
		CLabelUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = UISTATE_DISABLED;
		}
		else {
			m_uButtonState = 0;
		}
	}

	
	void CButtonUI::SetHotFont(int index)
	{
		m_iHotFont = index;
		Invalidate();
	}

	int CButtonUI::GetHotFont() const
	{
		return m_iHotFont;
	}

	void CButtonUI::SetPushedFont(int index)
	{
		m_iPushedFont = index;
		Invalidate();
	}

	int CButtonUI::GetPushedFont() const
	{
		return m_iPushedFont;
	}

	void CButtonUI::SetFocusedFont(int index)
	{
		m_iFocusedFont = index;
		Invalidate();
	}

	int CButtonUI::GetFocusedFont() const
	{
		return m_iFocusedFont;
	}

	void CButtonUI::SetHotBkColor( DWORD dwColor )
	{
		m_dwHotBkColor = dwColor;
		Invalidate();
	}

	DWORD CButtonUI::GetHotBkColor() const
	{
		return m_dwHotBkColor;
	}
	
	void CButtonUI::SetPushedBkColor( DWORD dwColor )
	{
		m_dwPushedBkColor = dwColor;
		Invalidate();
	}

	DWORD CButtonUI::GetPushedBkColor() const
	{
		return m_dwPushedBkColor;
	}
		
	void CButtonUI::SetDisabledBkColor( DWORD dwColor )
	{
		m_dwDisabledBkColor = dwColor;
		Invalidate();
	}

	DWORD CButtonUI::GetDisabledBkColor() const
	{
		return m_dwDisabledBkColor;
	}
	
	void CButtonUI::SetHotTextColor(DWORD dwColor)
	{
		m_dwHotTextColor = dwColor;
	}

	DWORD CButtonUI::GetHotTextColor() const
	{
		return m_dwHotTextColor;
	}

	void CButtonUI::SetPushedTextColor(DWORD dwColor)
	{
		m_dwPushedTextColor = dwColor;
	}

	DWORD CButtonUI::GetPushedTextColor() const
	{
		return m_dwPushedTextColor;
	}

	void CButtonUI::SetFocusedTextColor(DWORD dwColor)
	{
		m_dwFocusedTextColor = dwColor;
	}

	DWORD CButtonUI::GetFocusedTextColor() const
	{
		return m_dwFocusedTextColor;
	}

	void CButtonUI::SetHotBorderColor(DWORD dwColor)
	{
		m_dwHotBorderColor = dwColor;
	}

	DWORD CButtonUI::GetHotBorderColor() const
	{
		return m_dwHotBorderColor;
	}

	LPCTSTR CButtonUI::GetNormalImage()
	{
		return m_diNormal.sDrawString;
	}

	void CButtonUI::SetNormalImage(LPCTSTR pStrImage)
	{
		if( m_diNormal.sDrawString == pStrImage && m_diNormal.pImageInfo != NULL ) return;
		m_diNormal.Clear();
		m_diNormal.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diNormal, m_nScale);
		Invalidate();
	}

	LPCTSTR CButtonUI::GetHotImage()
	{
		return m_diHot.sDrawString;
	}

	void CButtonUI::SetHotImage(LPCTSTR pStrImage)
	{
		if( m_diHot.sDrawString == pStrImage && m_diHot.pImageInfo != NULL ) return;
		m_diHot.Clear();
		m_diHot.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diHot, m_nScale);
		Invalidate();
	}

	LPCTSTR CButtonUI::GetPushedImage()
	{
		return m_diPushed.sDrawString;
	}

	void CButtonUI::SetPushedImage(LPCTSTR pStrImage)
	{
		if( m_diPushed.sDrawString == pStrImage && m_diPushed.pImageInfo != NULL ) return;
		m_diPushed.Clear();
		m_diPushed.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diPushed, m_nScale);
		Invalidate();
	}

	LPCTSTR CButtonUI::GetFocusedImage()
	{
		return m_diFocused.sDrawString;
	}

	void CButtonUI::SetFocusedImage(LPCTSTR pStrImage)
	{
		if( m_diFocused.sDrawString == pStrImage && m_diFocused.pImageInfo != NULL ) return;
		m_diFocused.Clear();
		m_diFocused.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diFocused, m_nScale);
		Invalidate();
	}

	LPCTSTR CButtonUI::GetDisabledImage()
	{
		return m_diDisabled.sDrawString;
	}

	void CButtonUI::SetDisabledImage(LPCTSTR pStrImage)
	{
		if( m_diDisabled.sDrawString == pStrImage && m_diDisabled.pImageInfo != NULL ) return;
		m_diDisabled.Clear();
		m_diDisabled.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diDisabled, m_nScale);
		Invalidate();
	}

	LPCTSTR CButtonUI::GetForeImage()
	{
		return m_diFore.sDrawString;
	}

	void CButtonUI::SetForeImage( LPCTSTR pStrImage )
	{
		if( m_diFore.sDrawString == pStrImage && m_diFore.pImageInfo != NULL ) return;
		m_diFore.Clear();
		m_diFore.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diFore, m_nScale);
		Invalidate();
	}

	LPCTSTR CButtonUI::GetHotForeImage()
	{
		return m_diHotFore.sDrawString;
	}

	void CButtonUI::SetHotForeImage( LPCTSTR pStrImage )
	{
		if( m_diHotFore.sDrawString == pStrImage && m_diHotFore.pImageInfo != NULL ) return;
		m_diHotFore.Clear();
		m_diHotFore.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diHotFore, m_nScale);
		Invalidate();
	}

	void CButtonUI::SetFiveStatusImage(LPCTSTR pStrImage)
	{
		m_diNormal.Clear();
		m_diNormal.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diNormal, m_nScale);
		if (m_diNormal.pImageInfo) {
			LONG width = m_diNormal.pImageInfo->nX / 5;
			LONG height = m_diNormal.pImageInfo->nY;
			m_diNormal.rcBmpPart = CDuiRect(0, 0, width, height);
			if( m_bFloat && m_cxyFixed.cx == 0 && m_cxyFixed.cy == 0 ) {
				m_cxyFixed.cx = width;
				m_cxyFixed.cy = height;
                m_bFixedWidth = true;
                m_bFixedHeight = true;
			}
		}

		m_diPushed.Clear();
		m_diPushed.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diPushed, m_nScale);
		if (m_diPushed.pImageInfo) {
			LONG width = m_diPushed.pImageInfo->nX / 5;
			LONG height = m_diPushed.pImageInfo->nY;
			m_diPushed.rcBmpPart = CDuiRect(width, 0, width*2, height);
		}

		m_diHot.Clear();
		m_diHot.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diHot, m_nScale);
		if (m_diHot.pImageInfo) {
			LONG width = m_diHot.pImageInfo->nX / 5;
			LONG height = m_diHot.pImageInfo->nY;
			m_diHot.rcBmpPart = CDuiRect(width*2, 0, width*3, height);
		}

		m_diFocused.Clear();
		m_diFocused.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diFocused, m_nScale);
		if (m_diFocused.pImageInfo) {
			LONG width = m_diFocused.pImageInfo->nX / 5;
			LONG height = m_diFocused.pImageInfo->nY;
			m_diFocused.rcBmpPart = CDuiRect(width*3, 0, width*4, height);
		}

		m_diDisabled.Clear();
		m_diDisabled.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diDisabled, m_nScale);
		if (m_diDisabled.pImageInfo) {
			LONG width = m_diDisabled.pImageInfo->nX / 5;
			LONG height = m_diDisabled.pImageInfo->nY;
			m_diDisabled.rcBmpPart = CDuiRect(width*4, 0, width*5, height);
		}

		Invalidate();
	}

	SIZE CButtonUI::EstimateSize(SIZE szAvailable)
	{
        return CLabelUI::EstimateSize(szAvailable);
	}

	void CButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
		else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
		else if( _tcscmp(pstrName, _T("pushedimage")) == 0 ) SetPushedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
		else if( _tcscmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
		else if( _tcscmp(pstrName, _T("hotforeimage")) == 0 ) SetHotForeImage(pstrValue);
		else if( _tcscmp(pstrName, _T("fivestatusimage")) == 0 ) SetFiveStatusImage(pstrValue);
		else if( _tcscmp(pstrName, _T("hotbkcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotBkColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("pushedbkcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetPushedBkColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("disabledbkcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDisabledBkColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("hottextcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("pushedtextcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetPushedTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("focusedtextcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetFocusedTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("hotbordercolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotBorderColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("hotfont")) == 0 ) SetHotFont(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("pushedfont")) == 0 ) SetPushedFont(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("focuedfont")) == 0 ) SetFocusedFont(_ttoi(pstrValue));
		
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	DWORD CButtonUI::GetStatusBorderColor() const
	{
		if (((m_uButtonState & UISTATE_HOT) != 0) && (m_dwHotBorderColor != 0)) {
			if(!IsFocused())
			    return m_dwHotBorderColor;
		}

		return CControlUI::GetStatusBorderColor();
	}

	void CButtonUI::PaintText(HDC hDC)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		CDuiString sText = GetText();
		if(sText.IsEmpty() ) return;
		int nLinks = 0;
        RECT rcTextPadding = GetScaleRect(m_rcTextPadding, m_nScale);
		RECT rc = m_rcItem;
		rc.left += rcTextPadding.left;
		rc.right -= rcTextPadding.right;
		rc.top += rcTextPadding.top;
		rc.bottom -= rcTextPadding.bottom;

		DWORD clrColor = m_dwDisabledTextColor; // 默认先给个不可用颜色
		if (IsEnabled())
		{
			clrColor = m_dwTextColor;

			if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0) )
				clrColor = GetPushedTextColor();
			else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0) )
				clrColor = GetHotTextColor();
			else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0) )
				clrColor = GetFocusedTextColor();
		}

		int iFont = GetFont();
		if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedFont() != -1) )
			iFont = GetPushedFont();
		else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotFont() != -1) )
			iFont = GetHotFont();
		else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedFont() != -1) )
			iFont = GetFocusedFont();

		if( m_bShowHtml )
			CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, clrColor, \
			NULL, NULL, nLinks, iFont, m_uTextStyle);
		else
			CRenderEngine::DrawText(hDC, m_pManager, rc, sText, clrColor, \
			iFont, m_uTextStyle);
	}

	void CButtonUI::PaintBkColor(HDC hDC)
	{
		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if(m_dwDisabledBkColor != 0) {
				CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwDisabledBkColor));
				return;
			}
		}
		else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if(m_dwPushedBkColor != 0) {
				CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwPushedBkColor));
				return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if(m_dwHotBkColor != 0) {
				CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwHotBkColor));
				return;
			}
		}

		return CControlUI::PaintBkColor(hDC);
	}

	void CButtonUI::PaintStatusImage(HDC hDC)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;
		if(!::IsWindowEnabled(m_pManager->GetPaintWindow())) {
			m_uButtonState &= UISTATE_DISABLED;
		}
		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if (DrawImage(hDC, m_diDisabled)) goto Label_ForeImage;
		}
		else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if (!DrawImage(hDC, m_diPushed))
				DrawImage(hDC, m_diNormal);
			if (DrawImage(hDC, m_diPushedFore)) return;
			else goto Label_ForeImage;
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if (!DrawImage(hDC, m_diHot))
				DrawImage(hDC, m_diNormal);

			if (DrawImage(hDC, m_diHotFore)) return;
			else goto Label_ForeImage;
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if (DrawImage(hDC, m_diFocused)) goto Label_ForeImage;;
		}

		DrawImage(hDC, m_diNormal);

Label_ForeImage:
		DrawImage(hDC, m_diFore);
	}

}