#include "StdAfx.h"
#include "UIGroupBox.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CGroupBoxUI)

	CGroupBoxUI::CGroupBoxUI(): m_uTextStyle(DT_SINGLELINE | DT_VCENTER | DT_CENTER), m_dwTextColor(0), 
		m_dwDisabledTextColor(0), m_iFont(-1)
	{
		SetInset(CDuiRect(12, 20, 12, 6));
	}

	CGroupBoxUI::~CGroupBoxUI()
	{
	}

	LPCTSTR CGroupBoxUI::GetClass() const
	{
		return DUI_CTR_GROUPBOX;
	}

	LPVOID CGroupBoxUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_GROUPBOX) == 0 ) return static_cast<CGroupBoxUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}
	void CGroupBoxUI::SetTextColor(DWORD dwTextColor)
	{
		if (dwTextColor)
		{
			BYTE A = dwTextColor >> 24;
			if (A == 0)
				dwTextColor += 0xFF000000;
		}

		m_dwTextColor = dwTextColor;
		Invalidate();
	}

	DWORD CGroupBoxUI::GetTextColor() const
	{
		return m_dwTextColor;
	}
	void CGroupBoxUI::SetDisabledTextColor(DWORD dwTextColor)
	{
		m_dwDisabledTextColor = dwTextColor;
		Invalidate();
	}

	DWORD CGroupBoxUI::GetDisabledTextColor() const
	{
		return m_dwDisabledTextColor;
	}
	void CGroupBoxUI::SetFont(int index)
	{
		m_iFont = index;
		Invalidate();
	}

	int CGroupBoxUI::GetFont() const
	{
		return m_iFont;
	}

	void CGroupBoxUI::PaintText(HDC hDC)
	{
		CDuiString sText = GetText();
		if( sText.IsEmpty() ) {
			return;
		}

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		CDuiRect rcText = CalcTextRange(m_rcItem);

		DWORD dwTextColor = m_dwTextColor;
		if(!IsEnabled()) dwTextColor = m_dwDisabledTextColor;
		CRenderEngine::DrawText(hDC, m_pManager, rcText, sText, dwTextColor, m_iFont, m_uTextStyle, GetAdjustColor(m_dwBackColor));
	}

	void CGroupBoxUI::PaintBorder(HDC hDC)
	{
		//CControlUI::PaintBorder(hDC);
		SIZE cxyBorderRound = GetScaleSize(m_cxyBorderRound, m_nScale);
		RECT rcBorderSize = GetScaleRect(m_rcBorderSize, m_nScale);

		if(rcBorderSize.left > 0 )
		{
			CDuiRect rcText = CalcTextRange(m_rcItem);
			CDuiRect rcItem = { m_rcItem.left + 1, m_rcItem.top + 10, m_rcItem.right - 1, m_rcItem.bottom - 1 };
			DWORD dwBorderColor = m_dwBorderColor;
			/*
			if (IsFocused() && m_dwFocusBorderColor != 0)
				dwBorderColor = m_dwFocusBorderColor;
            */
			if( cxyBorderRound.cx > 0 || cxyBorderRound.cy > 0 )//画圆角边框
			{
				DrawRoundBorder(hDC, rcItem, rcText, rcBorderSize.left, cxyBorderRound.cx, cxyBorderRound.cy, GetAdjustColor(dwBorderColor));
			}
			else
			{
				DrawRectBorder(hDC, rcItem, rcText, rcBorderSize.left, GetAdjustColor(dwBorderColor));
			}
		}

		//PaintText(hDC);
	}

	SIZE CGroupBoxUI::CalcrectSize(SIZE szAvailable)
	{
		SIZE cxyFixed = GetFixedXY();
		RECT rcText = { 0, 0, MAX(szAvailable.cx, cxyFixed.cx), 20 };
		
		CDuiString sText = GetText();

		CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, sText, m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle);
		SIZE cXY = {rcText.right - rcText.left, rcText.bottom - rcText.top};
		return cXY;
	}

	CDuiRect CGroupBoxUI::CalcTextRange(const RECT& rcItem)
	{
		CDuiRect rcText = rcItem;

		rcText.Deflate(4, 4);
		SIZE szAvailable = { rcText.right - rcText.left, rcText.bottom - rcText.top };
		SIZE sz = CalcrectSize(szAvailable);

		//计算文字区域
		rcText.left = rcText.left + 15;
		rcText.top = rcText.top - 4;
		rcText.right = rcText.left + sz.cx;
		rcText.bottom = rcText.top + sz.cy;

		return rcText;
	}

	void CGroupBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcsicmp(pstrName, _T("textcolor")) == 0 ) 
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetTextColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("disabledtextcolor")) == 0 ) 
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDisabledTextColor(clrColor);
		}
		else if( _tcsicmp(pstrName, _T("font")) == 0 ) 
		{
			SetFont(_ttoi(pstrValue));
		}

		CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
	}

	void CGroupBoxUI::DrawRoundBorder(HDC hDC, const RECT& rcBorder, const RECT& rcText, int nSize, int width, int height, DWORD dwPenColor, int nStyle)
	{
#ifdef _USE_GDIPLUS
		Gdiplus::Graphics graphics(hDC);
		Gdiplus::Pen pen(Gdiplus::Color(dwPenColor), (Gdiplus::REAL)nSize);
		pen.SetAlignment(Gdiplus::PenAlignmentInset);

		//左
		graphics.DrawLine(&pen, Gdiplus::Point(rcBorder.left, rcBorder.bottom - height), Gdiplus::Point(rcBorder.left, rcBorder.top + height));
		//左上
		graphics.DrawArc(&pen, rcBorder.left, rcBorder.top, width * 2, height * 2, (Gdiplus::REAL)180, (Gdiplus::REAL)90);

		//上
		graphics.DrawLine(&pen, Gdiplus::Point(rcBorder.left + width, rcBorder.top), Gdiplus::Point(rcText.left - 2, rcBorder.top));
		graphics.DrawLine(&pen, Gdiplus::Point(rcText.right + 2, rcBorder.top), Gdiplus::Point(rcBorder.right - width, rcBorder.top));
		//右上
		graphics.DrawArc(&pen, rcBorder.right - width * 2 - 1, rcBorder.top, width * 2, height * 2, (Gdiplus::REAL)270, (Gdiplus::REAL)90);

		//右
		graphics.DrawLine(&pen, Gdiplus::Point(rcBorder.right - 1, rcBorder.top + height), Gdiplus::Point(rcBorder.right - 1, rcBorder.bottom - height));
		//右下
		graphics.DrawArc(&pen, rcBorder.right - width * 2 - 1, rcBorder.bottom - height * 2 - 1, width * 2, height * 2, (Gdiplus::REAL)0, (Gdiplus::REAL)90);

		//下
		graphics.DrawLine(&pen, Gdiplus::Point(rcBorder.right - width, rcBorder.bottom - 1), Gdiplus::Point(rcBorder.left + width, rcBorder.bottom - 1));
		//左下
		graphics.DrawArc(&pen, rcBorder.left, rcBorder.bottom - height * 2 - 1, width * 2, height * 2, (Gdiplus::REAL)90, (Gdiplus::REAL)90);
#else
		HPEN hPen = ::CreatePen(nStyle | PS_INSIDEFRAME, nSize, RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor)));
		HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
		::SelectObject(hDC, ::GetStockObject(HOLLOW_BRUSH));

		::MoveToEx(hDC, rcBorder.left, rcBorder.bottom - height, NULL);
		//左
		::LineTo(hDC, rcBorder.left, rcBorder.top + height);
		//左上
		//graphics.DrawArc(&pen, rcBorder.left, rcBorder.top, width * 2, height * 2, (Gdiplus::REAL)180, (Gdiplus::REAL)90);
		::SetArcDirection(hDC, AD_CLOCKWISE);
		::Arc(hDC, rcBorder.left, rcBorder.top, rcBorder.left + width * 2, rcBorder.top + height * 2,
			rcBorder.left, rcBorder.top + height, rcBorder.left + width, rcBorder.top);

		::MoveToEx(hDC, rcBorder.left + width, rcBorder.top, NULL);
		//上
		::LineTo(hDC, rcText.left - 2, rcBorder.top);
		::MoveToEx(hDC, rcText.right + 2, rcBorder.top, NULL);
		::LineTo(hDC, rcBorder.right - width - 1, rcBorder.top);
		//右上
		//graphics.DrawArc(&pen, rcBorder.right - width * 2 - 1, rcBorder.top, width * 2, height * 2, (Gdiplus::REAL)270, (Gdiplus::REAL)90);
		::Arc(hDC, rcBorder.right - width * 2 - 1, rcBorder.top, rcBorder.right - 1, rcBorder.top + height * 2,
			rcBorder.right - width - 1, rcBorder.top, rcBorder.right - 1, rcBorder.top + height);

		//右
		::MoveToEx(hDC, rcBorder.right - 1, rcBorder.top + height, NULL);
		::LineTo(hDC, rcBorder.right - 1, rcBorder.bottom - height);
		//右下
		//graphics.DrawArc(&pen, rcBorder.right - width * 2 - 1, rcBorder.bottom - height * 2 - 1, width * 2, height * 2, (Gdiplus::REAL)0, (Gdiplus::REAL)90);
		::Arc(hDC, rcBorder.right - width * 2 - 1, rcBorder.bottom - height * 2 - 1, rcBorder.right - 1, rcBorder.bottom - 1,
			rcBorder.right - 1, rcBorder.bottom - height - 1, rcBorder.right - width - 1, rcBorder.bottom - 1);
		
		//下
		::MoveToEx(hDC, rcBorder.right - width - 1, rcBorder.bottom - 1, NULL);
		::LineTo(hDC, rcBorder.left + width, rcBorder.bottom - 1);
		//左下
		//graphics.DrawArc(&pen, rcBorder.left, rcBorder.bottom - height * 2 - 1, width * 2, height * 2, (Gdiplus::REAL)90, (Gdiplus::REAL)90);
		::Arc(hDC, rcBorder.left, rcBorder.bottom - height * 2 - 1, rcBorder.left + width * 2, rcBorder.bottom - 1,
			rcBorder.left + width, rcBorder.bottom - 1, rcBorder.left, rcBorder.bottom - height - 1);

		::SelectObject(hDC, hOldPen);
		::DeleteObject(hPen);
#endif
	}

	void CGroupBoxUI::DrawRectBorder(HDC hDC, const RECT& rcBorder, const RECT& rcText, int nSize, DWORD dwPenColor, int nStyle)
	{
#ifdef _USE_GDIPLUS
		Gdiplus::Graphics graphics(hDC);
		Gdiplus::Pen pen(Gdiplus::Color(dwPenColor), (Gdiplus::REAL)nSize);
		pen.SetAlignment(Gdiplus::PenAlignmentInset);

		//左
		graphics.DrawLine(&pen, Gdiplus::Point(rcBorder.left, rcBorder.bottom - 1), Gdiplus::Point(rcBorder.left, rcBorder.top));
		
		//上
		graphics.DrawLine(&pen, Gdiplus::Point(rcBorder.left, rcBorder.top), Gdiplus::Point(rcText.left - 2, rcBorder.top));
		graphics.DrawLine(&pen, Gdiplus::Point(rcText.right + 2, rcBorder.top), Gdiplus::Point(rcBorder.right - 1, rcBorder.top));

		//右
		graphics.DrawLine(&pen, Gdiplus::Point(rcBorder.right - 1, rcBorder.top), Gdiplus::Point(rcBorder.right - 1, rcBorder.bottom - 1));

		//下
		graphics.DrawLine(&pen, Gdiplus::Point(rcBorder.right - 1, rcBorder.bottom - 1), Gdiplus::Point(rcBorder.left, rcBorder.bottom - 1));
#else
		HPEN hPen = ::CreatePen(nStyle | PS_INSIDEFRAME, nSize, RGB(GetBValue(dwPenColor), GetGValue(dwPenColor), GetRValue(dwPenColor)));
		HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
		::SelectObject(hDC, ::GetStockObject(HOLLOW_BRUSH));

		::MoveToEx(hDC, rcBorder.left, rcBorder.bottom - 1, NULL);
		//左
		::LineTo(hDC, rcBorder.left, rcBorder.top);

		//上
		::LineTo(hDC, rcText.left - 2, rcBorder.top);
		::MoveToEx(hDC, rcText.right + 2, rcBorder.top, NULL);
		::LineTo(hDC, rcBorder.right - 1, rcBorder.top);

		//右
		::LineTo(hDC, rcBorder.right - 1, rcBorder.bottom - 1);

		//下
		::LineTo(hDC, rcBorder.left, rcBorder.bottom - 1);

		::SelectObject(hDC, hOldPen);
		::DeleteObject(hPen);
#endif
	}
}
