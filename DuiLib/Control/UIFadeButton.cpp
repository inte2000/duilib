#include "StdAfx.h"
#include "UIFadeButton.h"

namespace DuiLib {

	IMPLEMENT_DUICONTROL(CFadeButtonUI)

	CFadeButtonUI::CFadeButtonUI(): m_bMouseHove(FALSE)
		, m_bMouseLeave(FALSE)
		, m_timerInterval(20)
		, m_framesCount(10)
	{
		Attach(this);
	}

	CFadeButtonUI::~CFadeButtonUI()
	{
		StopAnimation();
	}

	LPCTSTR CFadeButtonUI::GetClass() const
	{
		return DUI_CTR_FADEBUTTON;
	}

	LPVOID CFadeButtonUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, DUI_CTR_FADEBUTTON) == 0) return static_cast<CFadeButtonUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	void CFadeButtonUI::DoInit()
	{
		m_diLastImage = m_diNormal; //设置成与 normal 一样
	}

	void CFadeButtonUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_TIMER ) {
			OnTimer(  event.wParam );
		}
		else if(IsEnabled()) {
			if( event.Type == UIEVENT_MOUSEENTER && !IsAnimationRunning( FADE_IN_ID ) )
			{
				m_nFadeAlpha = 0;
				m_bMouseHove = TRUE;
				StopAnimation(FADE_OUT_ID);
				StartAnimation(m_timerInterval, m_framesCount, FADE_IN_ID);
				Invalidate();
				return;
			}
			else if( event.Type == UIEVENT_MOUSELEAVE && !IsAnimationRunning( FADE_OUT_ID ) )
			{
				m_nFadeAlpha = 0;
				m_bMouseLeave = TRUE;
				StopAnimation(FADE_IN_ID);
				StartAnimation(m_timerInterval, m_framesCount, FADE_OUT_ID);
				Invalidate();
				return;
			}
		}
		CButtonUI::DoEvent( event );
	}

	void CFadeButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("timeinterval")) == 0)
			SetTimerInterval(_ttoi(pstrValue));
		else if (_tcscmp(pstrName, _T("frames")) == 0)
			SetFramesCount(_ttoi(pstrValue));
		else
			CButtonUI::SetAttribute(pstrName, pstrValue);
	}

	void CFadeButtonUI::OnTimer(UINT_PTR nTimerID)
	{
		OnAnimationElapse((int)nTimerID);
	}

	void CFadeButtonUI::PaintStatusImage(HDC hDC)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if ((m_uButtonState & UISTATE_DISABLED) != 0) {
			if (DrawImage(hDC, m_diDisabled)) goto Label_ForeImage;
		}
		else if ((m_uButtonState & UISTATE_PUSHED) != 0) {
			if (!DrawImage(hDC, m_diPushed))
				DrawImage(hDC, m_diNormal);
			if (DrawImage(hDC, m_diPushedFore)) return;
			else goto Label_ForeImage;
		}
		else if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (!DrawImage(hDC, m_diHot))
				DrawImage(hDC, m_diNormal);

			if (DrawImage(hDC, m_diHotFore)) return;
			else if (m_dwHotBkColor != 0) {
				CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwHotBkColor));
				return;
			}
			else goto Label_ForeImage;
		}
		else if ((m_uButtonState & UISTATE_FOCUSED) != 0) {
			if (DrawImage(hDC, m_diFocused)) goto Label_ForeImage;
		}

		//画 normal 时的背景
		if (m_diNormal.pImageInfo != NULL) {
			if (m_bMouseHove) {
				m_bMouseHove = FALSE;
				m_diLastImage = m_diHot; //共享同一个 pImageInfo
			}

			if (m_bMouseLeave) {
				m_bMouseLeave = FALSE;
				m_diLastImage = m_diNormal; //共享同一个 pImageInfo
			}

			if (IsAnimationRunning(FADE_IN_ID) || IsAnimationRunning(FADE_OUT_ID)) {
				m_diOldImage = m_diNormal;
				m_diNewImage = m_diHot;
				if (IsAnimationRunning(FADE_OUT_ID)) {
					m_diOldImage = m_diHot;
					m_diNewImage = m_diNormal;
				}
				m_diOldImage.uFade = 255 - m_nFadeAlpha;
				m_diNewImage.uFade = m_nFadeAlpha;
				DrawImage(hDC, m_diOldImage);
				DrawImage(hDC, m_diNewImage);
			}
			else {
				if (m_diLastImage.pImageInfo == NULL) 
					m_diLastImage = m_diNormal;
				DrawImage(hDC, m_diLastImage);
			}
		}

	Label_ForeImage:
		DrawImage(hDC, m_diFore);
	}

	void CFadeButtonUI::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID)
	{
		m_nFadeAlpha = (BYTE)((nCurFrame * 255.0f / (float)nTotalFrame));
		m_nFadeAlpha = (m_nFadeAlpha == 0) ? 16 : m_nFadeAlpha;
		Invalidate();
	}

} // namespace DuiLib