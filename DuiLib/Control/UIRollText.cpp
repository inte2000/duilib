#include "stdafx.h"
#include "UIRollText.h"

namespace DuiLib
{
	IMPLEMENT_DUICONTROL(CRollTextUI)

	CRollTextUI::CRollTextUI(void)
	{
		m_nScrollPos = 1; //orbit 20230311 从 0 开始会导致第一次绘制出现 0 宽度的范围，使得 gdi 绘制字符串出错
		m_nText_W_H = 0;
		m_nStep = 5;
		m_bUseRoll = FALSE;
		m_nRollDirection = ROLLTEXT_LEFT;
		m_bAutoRoll = false;
		m_nRollEndSpan = ROLLTEXT_ROLL_END_SPAN;
	}

	CRollTextUI::~CRollTextUI(void)
	{
		m_pManager->KillTimer(this, ROLLTEXT_ROLL_END);
		m_pManager->KillTimer(this, ROLLTEXT_TIMERID);
	}

	LPCTSTR CRollTextUI::GetClass() const
	{
		return DUI_CTR_ROLLTEXT;
	}

	LPVOID CRollTextUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcsicmp(pstrName, DUI_CTR_ROLLTEXT) == 0 ) return static_cast<CRollTextUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	int CRollTextUI::GetRollDirection() const
	{
		return m_nRollDirection;
	}
	void CRollTextUI::SetRollDirection(int nDirect)
	{
		m_nRollDirection = nDirect;
		if (m_bUseRoll) // 重新开始
		{
			BeginRoll(m_nRollDirection, ROLLTEXT_TIMERID_SPAN, m_nRollEndSpan);
		}
	}

	int CRollTextUI::GetRollEndSpan() const
	{
		return m_nRollEndSpan;
	}

	void CRollTextUI::SetRollEndSpan(int nEndSpan)
	{
		m_nRollEndSpan = nEndSpan;
		if (m_bUseRoll) // 重新开始
		{
			BeginRoll(m_nRollDirection, ROLLTEXT_TIMERID_SPAN, m_nRollEndSpan);
		}
	}

	bool CRollTextUI::GetAutoRoll() const
	{
		return m_bAutoRoll;
	}

	void CRollTextUI::SetAutoRoll(bool autoroll)
	{
		m_bAutoRoll = autoroll;
	}

	void CRollTextUI::BeginRoll(int nDirect, LONG lTimeSpan, LONG lMaxTimeLimited)
	{
		m_nRollDirection = nDirect;
		if (m_bUseRoll)
		{
			EndRoll();
		}
		m_nText_W_H = 0;
		m_nScrollPos = 1;
	
		m_pManager->KillTimer(this, ROLLTEXT_TIMERID);
		m_pManager->SetTimer(this, ROLLTEXT_TIMERID, lTimeSpan);
	
		m_pManager->KillTimer(this, ROLLTEXT_ROLL_END);
		m_pManager->SetTimer(this, ROLLTEXT_ROLL_END, lMaxTimeLimited*1000);

		m_bUseRoll = TRUE;
	}

	void CRollTextUI::EndRoll()
	{
		if (!m_bUseRoll) return;

		m_pManager->KillTimer(this, ROLLTEXT_ROLL_END);
		m_pManager->KillTimer(this, ROLLTEXT_TIMERID);
		
		m_bUseRoll = FALSE;
	}

	void CRollTextUI::SetPos(RECT rc)
	{
		CLabelUI::SetPos(rc);
		m_nText_W_H = 0;			//布局变化重新计算
	}

	void CRollTextUI::SetText( LPCTSTR pstrText )
	{
		CLabelUI::SetText(pstrText);
		m_nText_W_H = 0;			//文本变化重新计算
	}

	void CRollTextUI::DoInit()
	{
		if (m_bAutoRoll)
		{
			BeginRoll(m_nRollDirection, ROLLTEXT_TIMERID_SPAN, m_nRollEndSpan);
		}
	}

	void CRollTextUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_TIMER && event.wParam == ROLLTEXT_ROLL_END)
		{
			m_pManager->KillTimer(this, ROLLTEXT_ROLL_END);
			m_pManager->SendNotify(this, DUI_MSGTYPE_TEXTROLLEND);
		}
		else if( event.Type == UIEVENT_TIMER && event.wParam == ROLLTEXT_TIMERID ) 
		{
			Invalidate();
			return;
		}
		CLabelUI::DoEvent(event);
	}

	void CRollTextUI::PaintText(HDC hDC)
	{
		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();
		DWORD dwTextColor = IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor;
		CDuiString sText = GetText();
		if( sText.IsEmpty() ) return;

		RECT rcTextPadding = GetTextPadding();
		CDuiRect  rcClient;
		rcClient = m_rcItem;
		rcClient.left += rcTextPadding.left;
		rcClient.right -= rcTextPadding.right;
		rcClient.top += rcTextPadding.top;
		rcClient.bottom -= rcTextPadding.bottom;

		CDuiRect rc = rcClient;

		if(m_nText_W_H > 0)
		{
			int nScrollRange = 0;

			if (m_nRollDirection == ROLLTEXT_LEFT || m_nRollDirection == ROLLTEXT_RIGHT) {	//左面移动
				nScrollRange = m_nText_W_H + rcClient.GetWidth();

				rc.Offset((m_nRollDirection == ROLLTEXT_LEFT?rcClient.GetWidth():-rcClient.GetWidth()), 0);
				rc.Offset((m_nRollDirection == ROLLTEXT_LEFT?-m_nScrollPos:m_nScrollPos), 0);
				rc.right += (m_nText_W_H - rcClient.GetWidth());
				if (rc.right > rcClient.right)
					rc.right = rcClient.right;
			}
			else {																		//上下移动
				nScrollRange = m_nText_W_H + rcClient.GetHeight();
				rc.Offset(0, (m_nRollDirection == ROLLTEXT_UP?rcClient.GetHeight():-rcClient.GetHeight()));
				rc.Offset(0, (m_nRollDirection == ROLLTEXT_UP?-m_nScrollPos:m_nScrollPos));
				rc.bottom += (m_nText_W_H - rcClient.GetHeight());
				if (rc.bottom > rcClient.bottom)
					rc.bottom = rcClient.bottom;
			}

			m_nScrollPos += m_nStep;
			if (m_nScrollPos > nScrollRange) {
				m_nScrollPos = 1;
			}
		}

		UINT uTextStyle = DT_WORDBREAK | DT_EDITCONTROL;
		if (m_nRollDirection == ROLLTEXT_LEFT || m_nRollDirection == ROLLTEXT_RIGHT)
			uTextStyle |= (DT_VCENTER | DT_SINGLELINE);
		else
			uTextStyle |= DT_CENTER;

		if(m_nText_W_H == 0) {
			uTextStyle |= DT_CALCRECT;				//第一次计算文本宽度或高度
			if (m_nRollDirection == ROLLTEXT_LEFT || m_nRollDirection == ROLLTEXT_RIGHT) {	//左面移动
				rc.right += 10000;
			}
			else {																		//上下移动
				rc.bottom += 10000;
			}
		}

		if( m_bShowHtml ) {
			int nLinks = 0;
			CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, dwTextColor, NULL, NULL, nLinks, m_iFont, uTextStyle);
		}
		else {
			CRenderEngine::DrawText(hDC, m_pManager, rc, sText, dwTextColor, m_iFont, uTextStyle);
		}

		if(m_nText_W_H == 0) {
			m_nText_W_H = (m_nRollDirection == ROLLTEXT_LEFT || m_nRollDirection == ROLLTEXT_RIGHT)?(rc.right - rc.left):(rc.bottom - rc.top);		//计算文本宽度或高度
		}
	}

	void CRollTextUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("direction")) == 0) {
			int dir = DirectionFromString(pstrValue);
			SetRollDirection(dir);
		}
		else if (_tcscmp(pstrName, _T("autoroll")) == 0) {
			SetAutoRoll(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcscmp(pstrName, _T("rollendlimit")) == 0) {
			SetRollEndSpan(_ttoi(pstrValue));
		}
		else
			CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	int CRollTextUI::DirectionFromString(LPCTSTR strDir)
	{
		if (_tcscmp(strDir, _T("up")) == 0)
			return ROLLTEXT_UP;
		else if (_tcscmp(strDir, _T("down")) == 0)
			return ROLLTEXT_DOWN;
		else if (_tcscmp(strDir, _T("right")) == 0)
			return ROLLTEXT_RIGHT;
		else
			return ROLLTEXT_LEFT;
	}
}