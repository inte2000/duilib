#include "StdAfx.h"
#include "UIExpandButton.h"

namespace DuiLib {

	IMPLEMENT_DUICONTROL(CExpandButtonUI)

	CExpandButtonUI::CExpandButtonUI(): m_nAnimationState(0)
		, m_timerInterval(10)
		, m_framesCount(10)
		, m_expandRate(1.5f)
	{
		m_oriPos.left = m_oriPos.right = m_oriPos.top = m_oriPos.bottom = 0;
		Attach(this);
	}

	CExpandButtonUI::~CExpandButtonUI()
	{
		StopAnimation();
	}

	LPCTSTR CExpandButtonUI::GetClass() const
	{
		return DUI_CTR_EXPANDBUTTON;
	}

	LPVOID CExpandButtonUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, DUI_CTR_EXPANDBUTTON) == 0) return static_cast<CExpandButtonUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	void CExpandButtonUI::DoInit()
	{
	}

	void CExpandButtonUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_TIMER ) {
			OnTimer(  event.wParam );
		}
		else if(IsEnabled()) {
			if(event.Type == UIEVENT_MOUSEENTER)
			{
				m_uButtonState |= UISTATE_HOT;
				if (m_nAnimationState == 0 || m_nAnimationState == 3)
				{
					m_nAnimationState = 1;
					if (IsAnimationRunning(EXPAND_TIMER_ID))
						StopAnimation(EXPAND_TIMER_ID);
					
					StartAnimation(m_timerInterval, m_framesCount, EXPAND_TIMER_ID);
				}
			}
			else if(event.Type == UIEVENT_MOUSELEAVE)
			{
				m_uButtonState &= ~UISTATE_HOT;
				if (m_nAnimationState == 1 || m_nAnimationState == 2)
				{
					m_nAnimationState = 3;
					if (IsAnimationRunning(EXPAND_TIMER_ID))
						StopAnimation(EXPAND_TIMER_ID);
						
					StartAnimation(m_timerInterval, m_framesCount, EXPAND_TIMER_ID);
				}
			}
		}

		CButtonUI::DoEvent(event);
	}

	void CExpandButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("timeinterval")) == 0)
			SetTimerInterval(_ttoi(pstrValue));
		else if (_tcscmp(pstrName, _T("frames")) == 0)
			SetFramesCount(_ttoi(pstrValue));
		else if (_tcscmp(pstrName, _T("expand")) == 0)
			SetExpandRate((float)_ttof(pstrValue));
		else
			CButtonUI::SetAttribute(pstrName, pstrValue);
	}

	void CExpandButtonUI::OnTimer(UINT_PTR nTimerID)
	{
		OnAnimationElapse((int)nTimerID);
	}

	void CExpandButtonUI::OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) 
	{
		if(::IsRectEmpty(&m_oriPos))
		{
			m_oriPos = GetPos();
		}
	}

	void CExpandButtonUI::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID)
	{
		int xStep = int(((m_expandRate - 1.0f) * (m_oriPos.right - m_oriPos.left)) / (nTotalFrame * 2.0f) + 0.5f);
		int yStep = int(((m_expandRate - 1.0f) * (m_oriPos.bottom - m_oriPos.top)) / (nTotalFrame * 2.0f) + 0.5f);

		if (m_nAnimationState == 1) //鼠标进入,正在变大
		{
			RECT rcCur = GetPos();
			RECT rcNew = { rcCur.left - xStep,rcCur.top - yStep,rcCur.right + xStep,rcCur.bottom + yStep };
			SetPos(rcNew);
		}
		else if (m_nAnimationState == 3) //鼠标离开,正在变小
		{
			/*测试效果用的代码*/
			RECT rcCur = GetPos();
			RECT rcNew = { rcCur.left + xStep,rcCur.top + yStep,rcCur.right - xStep,rcCur.bottom - yStep };
			SetPos(rcNew);
		}

		Invalidate();
	}

	void CExpandButtonUI::OnAnimationStop(INT nAnimationID) 
	{ 
		if (m_nAnimationState == 1) //变大过程结束
		{
			m_nAnimationState = 2;
		}
		else if (m_nAnimationState == 3) //变小过程结束
		{
			m_nAnimationState = 0;
			SetPos(m_oriPos); //恢复原位置
		}
	}


} // namespace DuiLib