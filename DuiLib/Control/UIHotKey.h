#ifndef __UIHOTKEY_H__
#define __UIHOTKEY_H__
#pragma once

namespace DuiLib{
	class CHotKeyUI;

	class DUILIB_API CHotKeyWnd : public CWindowWnd
	{
	public:
		CHotKeyWnd(void);

	public:
		void Init(CHotKeyUI * pOwner);
		RECT CalPos();
		LPCTSTR GetWindowClassName() const;
		void OnFinalMessage(HWND hWnd);
		LPCTSTR GetSuperClassName() const;
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	public:
		void SetHotKey(WORD wVirtualKeyCode, WORD wModifiers);
		void GetHotKey(WORD &wVirtualKeyCode, WORD &wModifiers) const;
		DWORD GetHotKey(void) const;
		void SetRules(WORD wInvalidComb, WORD wModifiers);
		void DrawHotKeyContent(HDC hDC, RECT& rcClient);
		
		enum {
			DEFAULT_TIMERID = 27,
		};
	protected:
		CHotKeyUI * m_pOwner;
		HBRUSH m_hBkBrush;
		bool m_bInit;
		bool m_bDrawCaret;
	};

	class DUILIB_API CHotKeyUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CHotKeyUI)
		friend CHotKeyWnd;
	public:
		CHotKeyUI();
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
		void SetEnabled(bool bEnable = true);
		void SetText(LPCTSTR pstrText);
		void SetNativeBkColor(DWORD dwBkColor);
		DWORD GetNativeBkColor() const;
		void SetHotBorderColor(DWORD dwColor);
		DWORD GetHotBorderColor() const;

		void SetPos(RECT rc);
		void SetVisible(bool bVisible = true);
		void SetInternVisible(bool bVisible = true);
		SIZE EstimateSize(SIZE szAvailable);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		virtual DWORD GetStatusBorderColor() const;
		void PaintText(HDC hDC);
		void PaintBkColor(HDC hDC);
	public:
		void GetHotKey(WORD &wVirtualKeyCode, WORD &wModifiers) const;
		DWORD GetHotKey(void) const;
		void SetHotKey(WORD wVirtualKeyCode, WORD wModifiers);
		void SetHotKey(LPCTSTR pstrKey);
	protected:
		CHotKeyWnd * m_pWindow;
		UINT m_uButtonState;
		DWORD m_dwNativeBkColor;
		DWORD m_dwHotBorderColor;

		WORD m_wVirtualKeyCode;
		WORD m_wModifiers;
	};
}


#endif