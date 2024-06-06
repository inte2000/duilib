#ifndef __UIIPADDRESS_H__
#define __UIIPADDRESS_H__

#pragma once

//给该控件添加一个属性dtstyle

namespace DuiLib
{
	class CIPAddressWnd;
	 
	/// 时间选择控件
	class DUILIB_API CIPAddressUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CIPAddressUI)
		friend class CIPAddressWnd;
	public:
		CIPAddressUI();
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		HWND GetNativeWindow() const;

		DWORD GetIP() const;
		void SetIP(DWORD dwIP);
		CDuiString GetIPString() const;
		void SetIPString(LPCTSTR ipStr);

		void SetReadOnly(bool bReadOnly);
		bool IsReadOnly() const;
		void SetNativeBkColor(DWORD dwBkColor);
		DWORD GetNativeBkColor() const;


		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void Move(SIZE szOffset, bool bNeedInvalidate = true);

		void DoEvent(TEventUI& event);
		SIZE EstimateSize(SIZE szAvailable);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void PaintText(HDC hDC);
	protected:
		void UpdateText();
		void SplitIp2Text(DWORD dwIp, CDuiString* strArray);
	protected:
		DWORD	m_dwIP;
		bool    m_bReadOnly;
		int		m_nIPUpdateFlag;
		DWORD   m_dwNativeBkColor;

		CIPAddressWnd* m_pWindow;
	};
}
#endif // __UIIPADDRESS_H__