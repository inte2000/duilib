#pragma once

#include <memory>
#include "UIlib.h"

#define  DUI_CTR_WZCLOCK                        (_T("WzClock"))

namespace DuiLib {

	struct TimeStruct
	{
		int year, month, days;
		int hour, minutes, seconds;
	};

    class CWzClockUI : public CLabelUI
    {
		DECLARE_DUICONTROL(CWzClockUI)
	public:
		CWzClockUI();
		virtual ~CWzClockUI();
		
		// Base 的基本结构
		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
		void DoInit() override;
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		void SetVisible(bool bVisible = true) override;
		void DoEvent(TEventUI& event) override;
		void SetUsingChsNumber(bool bUsingChs) { m_bUsingChsPanel = bUsingChs; }
		bool GetUsingChsNumber() const { return m_bUsingChsPanel; }
		// 功能函数
		void StartClock();
		void StopClock();
	protected:
		void OnTimer(UINT_PTR nTimerID);
		void SetInternVisible(bool bVisible = true) override;
		//bool DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl) override;
		void PaintText(HDC hDC) override;
		void ResetPanelText();
		void GeneratePanelTextArray(uint32_t from, uint32_t to, std::vector<CDuiString>& txtArray, LPCTSTR lpPanelName);
		void StepDrawTextArray(Gdiplus::Graphics& graph, Gdiplus::Font& font, POINT c, int r, 
			                    const std::vector<CDuiString>& txtArray, int start, int angle);
		void StepRotateTextArray(Gdiplus::Graphics& graph, Gdiplus::Font& font, POINT c, int r, 
			                     const std::vector<CDuiString>& txtArray, int curIdx, int toIdx, int step);
		void DrawDatePanel(Gdiplus::Graphics& graph, Gdiplus::Font& font, POINT c);
		TimeStruct GetCurrentDateTime();
	protected:
		std::vector<CDuiString> m_arHours;
		std::vector<CDuiString> m_arMinute;
		std::vector<CDuiString> m_arSeconds;
		int m_nAnimationState; // 0: hour expand
		int m_timerInterval;
		bool m_bUsingChsPanel;
		int m_nStepAngle;
		int m_nStepRotate;
		TimeStruct m_lastClock, m_stepClock;

		bool m_nClockRunning;
		enum {
			CLOCK_TIMER_ID = 18,
			ANIMATE_TIMER_ID = 19,
		};

    };

}