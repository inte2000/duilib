#include "stdafx.h"
#include "UIProgress.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CProgressUI)

	CProgressUI::CProgressUI() : m_bHorizontal(true), m_nMin(0), m_nMax(100), m_nValue(0)
	{
		m_uTextStyle = DT_SINGLELINE | DT_CENTER;
		SetFixedHeight(12);
	}

	LPCTSTR CProgressUI::GetClass() const
	{
		return DUI_CTR_PROGRESS;
	}

	LPVOID CProgressUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_PROGRESS) == 0 ) return static_cast<CProgressUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CProgressUI::GetControlFlags() const
	{
		if (IsEnabled()) return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
		else return 0;
	}

	bool CProgressUI::IsHorizontal()
	{
		return m_bHorizontal;
	}

	void CProgressUI::SetHorizontal(bool bHorizontal)
	{
		if( m_bHorizontal == bHorizontal ) return;

		m_bHorizontal = bHorizontal;
		Invalidate();
	}

	int CProgressUI::GetMinValue() const
	{
		return m_nMin;
	}

	void CProgressUI::SetMinValue(int nMin)
	{
		m_nMin = nMin;
		Invalidate();
	}

	int CProgressUI::GetMaxValue() const
	{
		return m_nMax;
	}

	void CProgressUI::SetMaxValue(int nMax)
	{
		m_nMax = nMax;
		Invalidate();
	}

	int CProgressUI::GetValue() const
	{
		return m_nValue;
	}

	void CProgressUI::SetValue(int nValue)
	{
		m_nValue = nValue;
		if (m_nValue > m_nMax) m_nValue = m_nMax;
		if (m_nValue < m_nMin) m_nValue = m_nMin;
		Invalidate();
	}

	LPCTSTR CProgressUI::GetForeImage() const
	{
		return m_diFore.sDrawString;
	}

	void CProgressUI::SetForeImage(LPCTSTR pStrImage)
	{
		if( m_diFore.sDrawString == pStrImage && m_diFore.pImageInfo != NULL ) return;
		m_diFore.Clear();
		m_diFore.sDrawString = pStrImage;
        ParseDrawInfo(m_pManager, m_diFore, m_nScale);
		Invalidate();
	}

	void CProgressUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
		else if( _tcscmp(pstrName, _T("hor")) == 0 ) SetHorizontal(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("min")) == 0 ) SetMinValue(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("max")) == 0 ) SetMaxValue(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("value")) == 0 ) SetValue(_ttoi(pstrValue));
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CProgressUI::PaintStatusImage(HDC hDC)
	{
		if( m_nMax <= m_nMin ) m_nMax = m_nMin + 1;
		if( m_nValue > m_nMax ) m_nValue = m_nMax;
		if( m_nValue < m_nMin ) m_nValue = m_nMin;

		RECT rc = {0};
        RECT rcItem = m_rcItem;
		if( m_bHorizontal ) {
			rc.right = (m_nValue - m_nMin) * (rcItem.right - rcItem.left) / (m_nMax - m_nMin);
			rc.bottom = rcItem.bottom - rcItem.top;
		}
		else {
			rc.top = (rcItem.bottom - rcItem.top) * (m_nMax - m_nValue) / (m_nMax - m_nMin);
			rc.right = rcItem.right - rcItem.left;
			rc.bottom = rcItem.bottom - rcItem.top;
		}
		m_diFore.rcDestOffset = rc;
		if( DrawImage(hDC, m_diFore) ) return;
	}

}
