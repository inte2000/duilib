#ifndef __UITEXTSCROLLH__
#define __UITEXTSCROLLH__

#pragma once

namespace DuiLib
{
	#define ROLLTEXT_LEFT		0
	#define ROLLTEXT_RIGHT		1
	#define ROLLTEXT_UP 		2
	#define ROLLTEXT_DOWN		3

	#define ROLLTEXT_TIMERID			23
	#define ROLLTEXT_TIMERID_SPAN		50U

	#define ROLLTEXT_ROLL_END			24
	#define ROLLTEXT_ROLL_END_SPAN		1000*6U

	class DUILIB_API CRollTextUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CRollTextUI)
	public:
		CRollTextUI(void);
		virtual ~CRollTextUI(void);

	public:
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		int GetRollDirection() const;
		void SetRollDirection(int nDirect);
		int GetRollEndSpan() const;
		void SetRollEndSpan(int nEndSpan);
		bool GetAutoRoll() const;
		void SetAutoRoll(bool autoroll);
	public:
		virtual void PaintText(HDC hDC);
		void DoInit() override;
		virtual void DoEvent(TEventUI& event);
		virtual void SetPos(RECT rc);
		virtual void SetText(LPCTSTR pstrText);
		virtual void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	public:
		void BeginRoll(int nDirect = ROLLTEXT_RIGHT, LONG lTimeSpan = ROLLTEXT_TIMERID_SPAN, LONG lMaxTimeLimited = 60);
		void EndRoll();
		int DirectionFromString(LPCTSTR strDir);
	private:    
		int m_nStep;
		int m_nScrollPos;
		BOOL m_bUseRoll;
		int m_nRollDirection;
		int m_nRollEndSpan;
		bool m_bAutoRoll;
		int m_nText_W_H;
	};

}	// namespace DuiLib

#endif // __UITEXTSCROLLH__