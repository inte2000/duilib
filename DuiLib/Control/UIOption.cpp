#include "stdafx.h"
#include "UIOption.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(COptionUI)

	COptionUI::COptionUI() : m_bSelected(false), m_dwSelectedBkColor(0), m_dwSelectedTextColor(0)
	{
	}

	COptionUI::~COptionUI()
	{
		if( !m_sGroupName.IsEmpty() && m_pManager ) m_pManager->RemoveOptionGroup(m_sGroupName, this);
	}

	LPCTSTR COptionUI::GetClass() const
	{
		return DUI_CTR_OPTION;
	}

	LPVOID COptionUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_OPTION) == 0 ) return static_cast<COptionUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	void COptionUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CControlUI::SetManager(pManager, pParent, bInit);
		if( bInit && !m_sGroupName.IsEmpty() ) {
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
		}
	}

	LPCTSTR COptionUI::GetGroup() const
	{
		return m_sGroupName;
	}

	void COptionUI::SetGroup(LPCTSTR pStrGroupName)
	{
		if( pStrGroupName == NULL ) {
			if( m_sGroupName.IsEmpty() ) return;
			m_sGroupName.Empty();
		}
		else {
			if( m_sGroupName == pStrGroupName ) return;
			if (!m_sGroupName.IsEmpty() && m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
			m_sGroupName = pStrGroupName;
		}

		if( !m_sGroupName.IsEmpty() ) {
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
		}
		else {
			if (m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
		}

		Selected(m_bSelected);
	}

	bool COptionUI::IsSelected() const
	{
		return m_bSelected;
	}

	void COptionUI::Selected(bool bSelected, bool bTriggerEvent)
	{
		if( m_bSelected == bSelected ) return;
		m_bSelected = bSelected;
		if( m_bSelected ) m_uButtonState |= UISTATE_SELECTED;
		else m_uButtonState &= ~UISTATE_SELECTED;

		if( m_pManager != NULL ) {
			if( !m_sGroupName.IsEmpty() ) {
				if( m_bSelected ) {
					CDuiPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
					for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
						COptionUI* pControl = static_cast<COptionUI*>(aOptionGroup->GetAt(i));
						if( pControl != this ) {
							pControl->Selected(false, bTriggerEvent);
						}
					}
					if (bTriggerEvent) m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
				}
			}
			else {
				if (bTriggerEvent) m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
			}
		}

		Invalidate();
	}

	bool COptionUI::Activate()
	{
		if( !CButtonUI::Activate() ) return false;
		if( !m_sGroupName.IsEmpty() ) Selected(true);
		else Selected(!m_bSelected);

		return true;
	}

	void COptionUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			if( m_bSelected ) m_uButtonState = UISTATE_SELECTED;
			else m_uButtonState = 0;
		}
	}

	LPCTSTR COptionUI::GetSelectedImage()
	{
		return m_diSelected.sDrawString;
	}

	void COptionUI::SetSelectedImage(LPCTSTR pStrImage)
	{
		if( m_diSelected.sDrawString == pStrImage && m_diSelected.pImageInfo != NULL ) return;
		m_diSelected.Clear();
		m_diSelected.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diSelected, m_nScale);
		Invalidate();
	}

	LPCTSTR COptionUI::GetSelectedHotImage()
	{
		return m_diSelectedHot.sDrawString;
	}

	void COptionUI::SetSelectedHotImage( LPCTSTR pStrImage )
	{
		if( m_diSelectedHot.sDrawString == pStrImage && m_diSelectedHot.pImageInfo != NULL ) return;
		m_diSelectedHot.Clear();
		m_diSelectedHot.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diSelectedHot, m_nScale);
		Invalidate();
	}

	void COptionUI::SetSelectedTextColor(DWORD dwTextColor)
	{
		m_dwSelectedTextColor = dwTextColor;
	}

	DWORD COptionUI::GetSelectedTextColor()
	{
		if (m_dwSelectedTextColor == 0) m_dwSelectedTextColor = m_pManager->GetDefaultFontColor();
		return m_dwSelectedTextColor;
	}

	void COptionUI::SetSelectedBkColor( DWORD dwBkColor )
	{
		m_dwSelectedBkColor = dwBkColor;
	}

	DWORD COptionUI::GetSelectedBkColor()
	{
		return m_dwSelectedBkColor;
	}

	DUI_DEPRECATED DWORD COptionUI::GetSelectBkColor()
	{
		return this->GetSelectedBkColor();
	}

	SIZE COptionUI::EstimateSize(SIZE szAvailable)
	{
        return CButtonUI::EstimateSize(szAvailable);
	}

	void COptionUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("group")) == 0 ) SetGroup(pstrValue);
		else if( _tcscmp(pstrName, _T("selected")) == 0 ) Selected(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("selectedimage")) == 0 ) SetSelectedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("selectedhotimage")) == 0 ) SetSelectedHotImage(pstrValue);
		else if( _tcscmp(pstrName, _T("selectedbkcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedBkColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("selectedtextcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedTextColor(clrColor);
		}
		else CButtonUI::SetAttribute(pstrName, pstrValue);
	}

	void COptionUI::PaintBkColor(HDC hDC)
	{
		if(IsSelected()) {
			if(m_dwSelectedBkColor != 0) {
				CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwSelectedBkColor));
			}
		}
		else {
			return CButtonUI::PaintBkColor(hDC);
		}
	}

	void COptionUI::PaintStatusImage(HDC hDC)
	{
		UINT uSavedState = 0;
		if( (m_uButtonState & UISTATE_SELECTED) != 0 ) {
			if ((m_uButtonState & UISTATE_HOT) != 0)
			{
				if (DrawImage(hDC, m_diSelectedHot)) goto Label_ForeImage;
			}

			if( DrawImage(hDC, m_diSelected) ) goto Label_ForeImage;
			else if(m_dwSelectedBkColor != 0) {
				CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwSelectedBkColor));
				goto Label_ForeImage;
			}	
		}

		uSavedState = m_uButtonState;
		m_uButtonState &= ~UISTATE_PUSHED;
		CButtonUI::PaintStatusImage(hDC);
		m_uButtonState = uSavedState;

Label_ForeImage:
		DrawImage(hDC, m_diFore);
	}

	void COptionUI::PaintText(HDC hDC)
	{
		if( (m_uButtonState & UISTATE_SELECTED) != 0 )
		{
			DWORD oldTextColor = m_dwTextColor;

		    if (((m_uButtonState & UISTATE_HOT) != 0) && (m_dwHotTextColor != 0))
				m_dwTextColor = m_dwHotTextColor;
			else
			    m_dwTextColor = m_dwSelectedTextColor;

			if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
			if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

			CDuiString sText = GetText();
			if( sText.IsEmpty() ) return;
			int nLinks = 0;
			RECT rc = m_rcItem;
            RECT rcTextPadding = GetScaleRect(m_rcTextPadding, m_nScale);
			rc.left += rcTextPadding.left;
			rc.right -= rcTextPadding.right;
			rc.top += rcTextPadding.top;
			rc.bottom -= rcTextPadding.bottom;

			if( m_bShowHtml )
				CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
				NULL, NULL, nLinks, m_iFont, m_uTextStyle);
			else
				CRenderEngine::DrawText(hDC, m_pManager, rc, sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
				m_iFont, m_uTextStyle);

			m_dwTextColor = oldTextColor;
		}
		else
		{
			UINT uSavedState = m_uButtonState;
			m_uButtonState &= ~UISTATE_PUSHED;
			CButtonUI::PaintText(hDC);
			m_uButtonState = uSavedState;
		}
	}

}