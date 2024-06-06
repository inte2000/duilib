#ifndef __UIFADEBUTTON_H__
#define __UIFADEBUTTON_H__

#pragma once

#include "UIAnimation.h"

namespace DuiLib {

	class DUILIB_API CFadeButtonUI : public CButtonUI, public CUIAnimation
	{
		DECLARE_DUICONTROL(CFadeButtonUI)
	public:
		CFadeButtonUI();
		virtual ~CFadeButtonUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		UINT GetTimerInterval() const { return m_timerInterval; }
		void SetTimerInterval(UINT interval) { m_timerInterval = interval; }
		UINT GetFramesCount() const { return m_framesCount; }
		void SetFramesCount(UINT count) { m_framesCount = count; }

		void DoInit();
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void OnTimer(UINT_PTR nTimerID);
		void PaintStatusImage(HDC hDC);

		virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
		virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
		virtual void OnAnimationStop(INT nAnimationID) {}

	protected:
		TDrawInfo m_diOldImage;
		TDrawInfo m_diNewImage;
		TDrawInfo m_diLastImage;
		UINT      m_timerInterval;
		UINT      m_framesCount;
		BYTE      m_nFadeAlpha;
		BOOL      m_bMouseHove;
		BOOL      m_bMouseLeave;
		enum{
			FADE_IN_ID			= 8,
			FADE_OUT_ID			= 9,
		};
	};

} // namespace DuiLib

#endif // __UIFADEBUTTON_H__