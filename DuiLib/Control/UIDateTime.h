#ifndef __UIDATETIME_H__
#define __UIDATETIME_H__

#pragma once

namespace DuiLib
{
	class CDateTimeWnd;

	/// ʱ��ѡ��ؼ�
	class DUILIB_API CDateTimeUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CDateTimeUI)
		friend class CDateTimeWnd;
	public:
		CDateTimeUI();
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		HWND GetNativeWindow() const;

		SYSTEMTIME& GetTime();
		void SetTime(SYSTEMTIME* pst);

		void SetReadOnly(bool bReadOnly);
		bool IsReadOnly() const;
		void SetShowTime(bool bShowTime);
		bool IsShowTime() const;

		void UpdateText();

        void SetPos(RECT rc, bool bNeedInvalidate = true);
        void Move(SIZE szOffset, bool bNeedInvalidate = true);

		void DoEvent(TEventUI& event);
        void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);


	protected:
		SYSTEMTIME m_sysTime;
		int        m_nDTUpdateFlag;
		bool       m_bReadOnly;
		bool       m_bShowTime;

		CDateTimeWnd* m_pWindow;
	};
}
#endif // __UIDATETIME_H__