#include "framework.h"
#include <string>
#include <map>
#include <chrono>
#include <atlutil.h>
#include "CWzClockUI.h"


namespace DuiLib {

	static LPCTSTR s_ChnNum[] = { _T("��"), _T("һ"), _T("��"), _T("��"), _T("��"), _T("��"), _T("��"), _T("��"), _T("��"), _T("��"),
	    _T("ʮ"), _T("ʮһ"), _T("ʮ��"), _T("ʮ��"), _T("ʮ��"), _T("ʮ��"), _T("ʮ��"), _T("ʮ��"), _T("ʮ��"), _T("ʮ��"),
		_T("��ʮ"), _T("��ʮһ"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"),
		_T("��ʮ"), _T("��ʮһ"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"),
		_T("��ʮ"), _T("��ʮһ"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"),
		_T("��ʮ"), _T("��ʮһ"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"), _T("��ʮ��"),
		_T("��ʮ") };

	CDuiString ToNumberStr(uint32_t num, bool bChn)
	{
		CDuiString chnNum;
		if (num <= 60)
		{
			if (bChn)
			{
				chnNum = s_ChnNum[num];
			}
			else
			{
				TCHAR tmp[16] = { 0 };
				_itot_s(num, tmp, 16, 10);
				chnNum = tmp;
			}
		}

		return chnNum;
	}


	IMPLEMENT_DUICONTROL(CWzClockUI)

	CWzClockUI::CWzClockUI() : m_nAnimationState(0), 
		                       m_timerInterval(50), 
		                       m_bUsingChsPanel(true),
		                       m_nStepAngle(0),
		                       m_nStepRotate(0),
		                       m_nClockRunning(false)
	{
	}
	
	CWzClockUI::~CWzClockUI()
	{
		StopClock();
	}

	LPCTSTR CWzClockUI::GetClass() const
	{
		return DUI_CTR_WZCLOCK;
	}

	LPVOID CWzClockUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, DUI_CTR_WZCLOCK) == 0) return static_cast<CWzClockUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}
	
	void CWzClockUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("usingchsnum")) == 0) SetUsingChsNumber(_tcscmp(pstrValue, _T("true")) == 0);
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CWzClockUI::DoInit()
	{
		StartClock();
	}

	void CWzClockUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CLabelUI::SetPos(rc, bNeedInvalidate);
	}

	void CWzClockUI::SetInternVisible(bool bVisible)
	{
		CLabelUI::SetInternVisible(bVisible);
	}

	void CWzClockUI::PaintText(HDC hDC)
	{
		if (m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();

		POINT CenterPt = { (m_rcItem.left + m_rcItem.right) / 2 ,(m_rcItem.top + m_rcItem.bottom) / 2 };
		//Gdiplus::FontFamily fontFamily(L"Arial");
		//Gdiplus::Font nFont(&fontFamily, 24, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::Font nFont(hDC, m_pManager->GetFont(GetFont()));
		Gdiplus::Graphics nGraphics(hDC);

		nGraphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		nGraphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias); //Ĭ�Ͽ���ݼ���


		if (m_nAnimationState == 0)
		{
			StepDrawTextArray(nGraphics, nFont, CenterPt, 60, m_arHours, m_lastClock.hour, m_nStepAngle);
		}
		else if (m_nAnimationState == 1)
		{
			StepDrawTextArray(nGraphics, nFont, CenterPt, 60, m_arHours, m_lastClock.hour, 360);
			StepDrawTextArray(nGraphics, nFont, CenterPt, 130, m_arMinute, m_lastClock.minutes, m_nStepAngle);
		}
		else if (m_nAnimationState == 2)
		{
			StepDrawTextArray(nGraphics, nFont, CenterPt, 60, m_arHours, m_lastClock.hour, 360);
			StepDrawTextArray(nGraphics, nFont, CenterPt, 130, m_arMinute, m_lastClock.minutes, 360);
			StepDrawTextArray(nGraphics, nFont, CenterPt, 200, m_arSeconds, m_lastClock.seconds, m_nStepAngle);
		}
		else if (m_nAnimationState == 4)
		{
			StepRotateTextArray(nGraphics, nFont, CenterPt, 60, m_arHours, m_lastClock.hour, m_stepClock.hour, m_nStepRotate);
			StepRotateTextArray(nGraphics, nFont, CenterPt, 130, m_arMinute, m_lastClock.minutes, m_stepClock.minutes, m_nStepRotate);
			StepRotateTextArray(nGraphics, nFont, CenterPt, 200, m_arSeconds, m_lastClock.seconds, m_stepClock.seconds, m_nStepRotate);
		}
		else
		{
			StepDrawTextArray(nGraphics, nFont, CenterPt, 60, m_arHours, m_lastClock.hour, 360);
			StepDrawTextArray(nGraphics, nFont, CenterPt, 130, m_arMinute, m_lastClock.minutes, 360);
			StepDrawTextArray(nGraphics, nFont, CenterPt, 200, m_arSeconds, m_lastClock.seconds,360);
		}
			DrawDatePanel(nGraphics, nFont, CenterPt);

	}

	void CWzClockUI::SetVisible(bool bVisible)
	{
		if (IsVisible() == bVisible)
			return;

		CLabelUI::SetVisible(bVisible);
	}

	void CWzClockUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_TIMER) {
			OnTimer(event.wParam);
		}

		CLabelUI::DoEvent(event);
	}

	void CWzClockUI::StartClock()
	{
		if (!m_nClockRunning)
		{
			ResetPanelText();
			m_nStepAngle = 0;
			m_lastClock = GetCurrentDateTime();

			m_pManager->SetTimer(this, CLOCK_TIMER_ID, 1000);
			m_pManager->SetTimer(this, ANIMATE_TIMER_ID, m_timerInterval);
			m_nClockRunning = true;
		}
	}

	void CWzClockUI::StopClock()
	{
		m_pManager->KillTimer(this, CLOCK_TIMER_ID);
		m_pManager->KillTimer(this, ANIMATE_TIMER_ID);
		m_nClockRunning = false;
	}


	void CWzClockUI::OnTimer(UINT_PTR nTimerID)
	{
		if (nTimerID == ANIMATE_TIMER_ID)
		{
			if (m_nAnimationState < 3)
			{
				m_nStepAngle += 36;
				if (m_nStepAngle > 360)
				{
					m_nStepAngle = 0;
					m_nAnimationState++;
				}
				Invalidate();
			}
			else if (m_nAnimationState == 4)
			{
				m_nStepRotate++;
				if (m_nStepRotate > 5)
				{
					m_nAnimationState = 5;
					m_lastClock = m_stepClock;
				}
				Invalidate();
			}
		}
		else if (nTimerID == CLOCK_TIMER_ID)
		{
			if (m_nAnimationState > 2)
			{
				m_stepClock = GetCurrentDateTime();
				if ((m_stepClock.hour != m_lastClock.hour)
					|| (m_stepClock.minutes != m_lastClock.minutes)
					|| (m_stepClock.seconds != m_lastClock.seconds))
				{
					m_nAnimationState = 4;
					m_nStepRotate = 0;
					Invalidate();
				}
			}
		}
	}

	void CWzClockUI::ResetPanelText()
	{
		GeneratePanelTextArray(0, 23, m_arHours, _T("ʱ"));
		GeneratePanelTextArray(0, 59, m_arMinute, _T("��"));
		GeneratePanelTextArray(0, 59, m_arSeconds, _T("��"));
	}

	void CWzClockUI::GeneratePanelTextArray(uint32_t from, uint32_t to, std::vector<CDuiString>& txtArray, LPCTSTR lpPanelName)
	{
		txtArray.clear();
		for (uint32_t num = from; num <= to; ++num)
		{
			CDuiString numStr = ToNumberStr(num, m_bUsingChsPanel);
			numStr += lpPanelName;

			txtArray.push_back(std::move(numStr));
		}
	}

	void CWzClockUI::StepDrawTextArray(Gdiplus::Graphics& graph, Gdiplus::Font& font, POINT c, int r, const std::vector<CDuiString>& txtArray, int start, int angle)
	{
		Gdiplus::PointF origin(c.x, c.y);

		// ����������ɫ
		Gdiplus::Color txtColor(LOBYTE(m_dwTextColor >> 24), GetBValue(m_dwTextColor), GetGValue(m_dwTextColor), GetRValue(m_dwTextColor));
		Gdiplus::SolidBrush brush(txtColor);

		// ʹ�� GDI+ ������ת����
		Gdiplus::PointF txtPt = origin;
		txtPt.X += r;

		float agt = float(angle) / txtArray.size();
		for (std::size_t i = 0; i < txtArray.size(); ++i)
		{
			int txtIdx = (start + i) % txtArray.size();
			Gdiplus::Matrix matrix;
			matrix.RotateAt(-agt * i, origin);  // ��ת�Ƕ�
			//matrix.RotateAt(-angle, origin);  // ��ת�Ƕ�
			graph.SetTransform(&matrix);
			// ��������
			graph.DrawString(txtArray[txtIdx], -1, &font, txtPt, &brush);
			graph.ResetTransform();
		}
	}

	void CWzClockUI::StepRotateTextArray(Gdiplus::Graphics& graph, Gdiplus::Font& font, POINT c, int r, const std::vector<CDuiString>& txtArray, int curIdx, int toIdx, int step)
	{
		if (curIdx == toIdx)
		{
			StepDrawTextArray(graph, font, c, r, txtArray, curIdx, 360);
			return;
		}

		Gdiplus::PointF origin(c.x, c.y);

		// ����������ɫ
		Gdiplus::Color txtColor(LOBYTE(m_dwTextColor >> 24), GetBValue(m_dwTextColor), GetGValue(m_dwTextColor), GetRValue(m_dwTextColor));
		Gdiplus::SolidBrush brush(txtColor);

		// ʹ�� GDI+ ������ת����
		Gdiplus::PointF txtPt = origin;
		txtPt.X += r;

		float agt = 360.0f / txtArray.size();
		float agtRotate = (agt * ((toIdx + txtArray.size() - curIdx) % txtArray.size())) / 5.0f;
		agtRotate = agtRotate * step;
		for (std::size_t i = 0; i < txtArray.size(); ++i)
		{
			int txtIdx = (curIdx + i) % txtArray.size();
			Gdiplus::Matrix matrix;
			matrix.RotateAt(-agt * i + agtRotate, origin);  // ��ת�Ƕ�
			//matrix.RotateAt(-angle, origin);  // ��ת�Ƕ�
			graph.SetTransform(&matrix);
			// ��������
			graph.DrawString(txtArray[txtIdx], -1, &font, txtPt, &brush);
			graph.ResetTransform();
		}
	}

	void CWzClockUI::DrawDatePanel(Gdiplus::Graphics& graph, Gdiplus::Font& font, POINT c)
	{
		CDuiString DateStr;
		if (m_bUsingChsPanel)
			DateStr.Format(_T("%4d��%d��%d��"), m_lastClock.year, m_lastClock.month, m_lastClock.days);
		else
			DateStr.Format(_T("%d/%d/%4d"), m_lastClock.month, m_lastClock.days, m_lastClock.year);

		Gdiplus::PointF origin(c.x, c.y);

		Gdiplus::RectF boundingBox;
		Gdiplus::StringFormat format = Gdiplus::StringFormat::GenericTypographic();;
		format.SetAlignment(Gdiplus::StringAlignmentNear); //left
		format.SetLineAlignment(Gdiplus::StringAlignmentCenter); //vcenter
		graph.MeasureString(DateStr, -1, &font, origin, &format, &boundingBox);
		origin.Y += boundingBox.Height;

		Gdiplus::Color txtColor(LOBYTE(m_dwTextColor >> 24), GetBValue(m_dwTextColor), GetGValue(m_dwTextColor), GetRValue(m_dwTextColor));
		Gdiplus::SolidBrush brush(txtColor);

		// �����̵�
		Gdiplus::Pen pen(txtColor);
		graph.DrawLine(&pen, origin.X, origin.Y, origin.X + 280, origin.Y);

		// �����ĵ�
		float radius = 3.0f;
		graph.FillEllipse(&brush, origin.X - radius, origin.Y - radius, radius * 2, radius * 2);

		//�����
		Gdiplus::PointF txtPt = { c.x - boundingBox.Width / 2, (float)c.y };
		//txtPt.X -= boundingBox.Width / 2;
		graph.DrawString(DateStr, -1, &font, txtPt, &brush);
	}

	using namespace std::chrono;

	TimeStruct CWzClockUI::GetCurrentDateTime()
	{
		const time_zone* tzCurrent = current_zone();
		auto nowPt = tzCurrent->to_local(system_clock::now());
		hh_mm_ss hms{ nowPt.time_since_epoch() % 24h };
		year_month_day ymd{floor<days>(nowPt)};
		TimeStruct tm;
		
		tm.year = int(ymd.year());
		tm.month = unsigned int(ymd.month());
		tm.days = unsigned int(ymd.day());
		tm.hour = hms.hours().count();
		tm.minutes = hms.minutes().count();
		tm.seconds = static_cast<int>(hms.seconds().count());

		return tm;
	}
}

