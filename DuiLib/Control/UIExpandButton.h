#ifndef __UIEXPANDBUTTON_H__
#define __UIEXPANDBUTTON_H__


#include "UIAnimation.h"

namespace DuiLib {

	class DUILIB_API CExpandButtonUI : public CButtonUI, public CUIAnimation
	{
		DECLARE_DUICONTROL(CExpandButtonUI)
	public:
		CExpandButtonUI();
		virtual ~CExpandButtonUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		UINT GetTimerInterval() const { return m_timerInterval; }
		void SetTimerInterval(UINT interval) { m_timerInterval = interval; }
		UINT GetFramesCount() const { return m_framesCount; }
		void SetFramesCount(UINT count) { m_framesCount = count; }
		float GetExpandRate() const { return m_expandRate; }
		void SetExpandRate(float rate) { m_expandRate = (rate < 1) ? 1 : rate; }

		void DoInit();
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void OnTimer(UINT_PTR nTimerID);

		virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop);
		virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
		virtual void OnAnimationStop(INT nAnimationID);
		
	protected:
		UINT       m_timerInterval;
		UINT       m_framesCount;
		float      m_expandRate;
		RECT       m_oriPos;
		int        m_nAnimationState;		//����״̬ 0-ԭʼ״̬ 1-������,���ڱ�� 2-���������,�����״̬ 3-����뿪,���ڱ�С
		//�����������С����ԭ�ȵ�maxwidth��minwidth��
		enum{
			EXPAND_TIMER_ID			= 12,
		};
	};

} // namespace DuiLib

#endif // __UIEXPANDBUTTON_H__