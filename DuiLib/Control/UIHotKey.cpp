#include "stdafx.h"
#include "UIHotKey.h"

namespace DuiLib{

	static CDuiString GetKeyName(UINT vk, BOOL fExtended)
	{
		UINT nScanCode = ::MapVirtualKeyEx(vk, 0, ::GetKeyboardLayout(0));
		switch (vk)
		{
			// Keys which are "extended" (except for Return which is Numeric Enter as extended)
		case VK_INSERT:
		case VK_DELETE:
		case VK_HOME:
		case VK_END:
		case VK_NEXT: // Page down
		case VK_PRIOR: // Page up
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			nScanCode |= 0x100; // Add extended bit
		}
		if (fExtended)
			nScanCode |= 0x01000000L;

		TCHAR szStr[MAX_PATH] = { 0 };
		::GetKeyNameText(nScanCode << 16, szStr, MAX_PATH);

		return CDuiString(szStr);
	}

	static CDuiString GetHotKeyName(WORD wCode, WORD wModifiers)
	{
		CDuiString strKeyName;
		const TCHAR szPlus[] = _T(" + ");

		if (wCode != 0 || wModifiers != 0)
		{
			if (wModifiers & HOTKEYF_CONTROL)
			{
				strKeyName += GetKeyName(VK_CONTROL, FALSE);
				strKeyName += szPlus;
			}


			if (wModifiers & HOTKEYF_SHIFT)
			{
				strKeyName += GetKeyName(VK_SHIFT, FALSE);
				strKeyName += szPlus;
			}

			if (wModifiers & HOTKEYF_ALT)
			{
				strKeyName += GetKeyName(VK_MENU, FALSE);
				strKeyName += szPlus;
			}

			strKeyName += GetKeyName(wCode, wModifiers & HOTKEYF_EXT);
		}

		return strKeyName;
	}

	CHotKeyWnd::CHotKeyWnd(void) : m_pOwner(NULL), m_hBkBrush(NULL), m_bInit(false), m_bDrawCaret(false)
	{
	}
	void CHotKeyWnd::Init(CHotKeyUI * pOwner)
	{
		m_pOwner = pOwner;

		if (m_hWnd == NULL)
		{
			RECT rcPos = CalPos();
			UINT uStyle = WS_CHILD | ES_AUTOHSCROLL;
			HWND hWnd = Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
			SetWindowFont(m_hWnd, m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->hFont, TRUE);
		}
		else
		{
			RECT rcPos = CalPos();
			::SetWindowPos(m_hWnd, NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left,
				rcPos.bottom - rcPos.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		}

		SetHotKey(m_pOwner->m_wVirtualKeyCode, m_pOwner->m_wModifiers);

		::EnableWindow(m_hWnd, m_pOwner->IsEnabled() == true);
		::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
		::SetFocus(m_hWnd);

		m_bInit = true;
	}

	RECT CHotKeyWnd::CalPos()
	{
		CDuiRect rcPos = m_pOwner->GetPos();

		RECT rcInset = m_pOwner->GetTextPadding();
		rcPos.left += rcInset.left;
		rcPos.top += rcInset.top;
		rcPos.right -= rcInset.right;
		rcPos.bottom -= rcInset.bottom;

		LONG lHeight = m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->tm.tmHeight;
		if( lHeight < rcPos.GetHeight() ) {
			rcPos.top += (rcPos.GetHeight() - lHeight) / 2;
			rcPos.bottom = rcPos.top + lHeight;
		}
		return rcPos;
	}


	LPCTSTR CHotKeyWnd::GetWindowClassName() const
	{
		return _T("HotKeyClass");
	}

	LPCTSTR CHotKeyWnd::GetSuperClassName() const
	{
		return HOTKEY_CLASS;
	}

	void CHotKeyWnd::OnFinalMessage(HWND hWnd)
	{
		// Clear reference and die
		if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
		m_pOwner->GetManager()->RemoveNativeWindow(hWnd);
		m_pOwner->m_pWindow = NULL;
		delete this;
	}

	void CHotKeyWnd::DrawHotKeyContent(HDC hDC, RECT& rcClient)
	{
		DWORD dwTextColor = m_pOwner->GetTextColor();
		DWORD dwBkColor = m_pOwner->GetNativeBkColor();
		WORD wCode = 0;
        WORD wModifiers = 0;

		GetHotKey(wCode, wModifiers);
		CDuiString strText = GetHotKeyName(wCode, wModifiers);
		::SetBkMode(hDC, TRANSPARENT);
		::SetTextColor(hDC, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));

		CRenderEngine::DrawColor(hDC, rcClient, dwBkColor);
		HFONT hOldFont = (HFONT)SelectObject(hDC, GetWindowFont(m_hWnd));
		::SIZE size = { 0 };
		::GetTextExtentPoint32(hDC, strText.GetData(), strText.GetLength(), &size);
		::DrawText(hDC, strText.GetData(), -1, &rcClient, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
		::SelectObject(hDC, hOldFont);
		::SetCaretPos(size.cx, 0);
	}

	void UpdateDrawCaret(HDC hDC, RECT& rcClient)
	{
		POINT ptCaret;
		::GetCaretPos(&ptCaret);

		RECT rcCaret = { ptCaret.x + 1, ptCaret.y + 2, ptCaret.x + 1, ptCaret.y + (rcClient.bottom - rcClient.top - 2) };
		CRenderEngine::DrawLine(hDC, rcCaret, 1, 0xFF000000);
	}

	LRESULT CHotKeyWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		//DUI__Trace(_T("CHotKeyWnd::HandleMessage(m_hWnd=%x, uMsg=0x%x, wParam=%x)\n"), m_hWnd, uMsg, wParam);

		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		if (uMsg == WM_CREATE) {
			m_pOwner->GetManager()->AddNativeWindow(m_pOwner, m_hWnd);
			if (m_pOwner->GetManager()->IsLayered()) {
				::SetTimer(m_hWnd, DEFAULT_TIMERID, ::GetCaretBlinkTime(), NULL);
			}
			bHandled = FALSE;
		}
		else if( uMsg == WM_KILLFOCUS ) lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		else if( uMsg == OCM_COMMAND ) {
			if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ) lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
			else if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE ) {
				RECT rcClient;
				::GetClientRect(m_hWnd, &rcClient);
				::InvalidateRect(m_hWnd, &rcClient, FALSE);
			}
		}
		else if( uMsg == WM_KEYDOWN && wParam == VK_RETURN ) {
			m_pOwner->GetManager()->SendNotify(m_pOwner, _T("return")); //暂时保留
		}
		else if ( (uMsg == WM_NCACTIVATE) || (uMsg == WM_NCACTIVATE) || (uMsg == WM_NCCALCSIZE) )
		{
			return 0;
		}
		else if (uMsg == WM_ERASEBKGND) {
			//lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
			return 1;
		}
		else if (uMsg == WM_NCPAINT) {
			lRes = OnNcPaint(uMsg, wParam, lParam, bHandled);
		}
		else if (uMsg == WM_PAINT)
		{
			if (m_pOwner->GetManager()->IsLayered()) {
				m_pOwner->GetManager()->AddNativeWindow(m_pOwner, m_hWnd);
			}
			lRes = OnPaint(uMsg, wParam, lParam, bHandled);
		}
		else if (uMsg == WM_PRINT) {
			if (m_pOwner->GetManager()->IsLayered()) {

				RECT rcWnd;
				::GetWindowRect(m_hWnd, &rcWnd);
				::OffsetRect(&rcWnd, -rcWnd.left, -rcWnd.top);

				HDC hDC = (HDC)wParam;
				DrawHotKeyContent(hDC, rcWnd);
				//lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
				lRes = 0;
				if (m_pOwner->IsEnabled() && m_bDrawCaret) { // todo:判断是否enabled
					UpdateDrawCaret(hDC, rcWnd);
				}
				return lRes;
			}
			bHandled = FALSE;
		}
		else if (uMsg == WM_TIMER) {
			if (wParam == DEFAULT_TIMERID) {
				m_bDrawCaret = !m_bDrawCaret;
				RECT rcClient;
				::GetClientRect(m_hWnd, &rcClient);
				::InvalidateRect(m_hWnd, &rcClient, TRUE);

				return 0;
			}
			bHandled = FALSE;
		}
		else bHandled = FALSE;
		if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		return lRes;
	}

	LRESULT CHotKeyWnd::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//HDC hDC = ::GetDCEx(m_hWnd, (HRGN)wParam, DCX_WINDOW | DCX_INTERSECTRGN);
		HDC hDC = ::GetWindowDC(m_hWnd);
		RECT rcWnd;
		::GetWindowRect(m_hWnd, &rcWnd);
		::OffsetRect(&rcWnd, -rcWnd.left, -rcWnd.top);

		RECT rcClient;
		::GetClientRect(m_hWnd, &rcClient);

		/*
		DWORD clrBack = m_pOwner->GetBkColor();
		for (int i = 0; i < 2; i++)
		{
			CRenderEngine::DrawRect(hDC, rcWnd, 1, clrBack);
			::InflateRect(&rcWnd, -1, -1);
		}
		*/

		::ReleaseDC(m_hWnd, hDC);

		bHandled = TRUE;
		return 0;
	}

	LRESULT CHotKeyWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		if (!m_pOwner->GetManager()->IsLayered()) //layer 窗口在 WM_PRINT 里画
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(m_hWnd, &ps);
			
			RECT rect;
			::GetClientRect(m_hWnd, &rect);
			DrawHotKeyContent(hDC, rect);

			EndPaint(m_hWnd, &ps);
			bHandled = TRUE;
		}

		return 0;
	}

	LRESULT CHotKeyWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);

		WORD wVirtualKeyCode = 0;
		WORD wModifiers = 0;
		GetHotKey(wVirtualKeyCode, wModifiers);
		if ((wVirtualKeyCode == 0) && (wModifiers == 0))
		{
			m_pOwner->SetHotKey(wVirtualKeyCode, wModifiers);
		}

		::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
		return lRes;
	}

	LRESULT CHotKeyWnd::OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if( !m_bInit ) return 0;
		if( m_pOwner == NULL ) return 0;
		WORD wVirtualKeyCode = 0;
		WORD wModifiers = 0;
		GetHotKey(wVirtualKeyCode, wModifiers);
		if (wVirtualKeyCode == 0) {
			m_pOwner->SetHotKey(wVirtualKeyCode, wModifiers);
			if(wModifiers == 0)
				m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_EVT_HOTKEYCHANGED);
		}
		else {
			if ((wVirtualKeyCode != m_pOwner->m_wVirtualKeyCode) || (wModifiers != m_pOwner->m_wModifiers))
			{
				m_pOwner->SetHotKey(wVirtualKeyCode, wModifiers);
				m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_EVT_HOTKEYCHANGED);
			}
		}
		return 0;
	}


	void CHotKeyWnd::SetHotKey(WORD wVirtualKeyCode, WORD wModifiers)
	{
		ASSERT(::IsWindow(m_hWnd));  
		::SendMessage(m_hWnd, HKM_SETHOTKEY, MAKEWORD(wVirtualKeyCode, wModifiers), 0L);
	}

	DWORD CHotKeyWnd::GetHotKey() const
	{
		ASSERT(::IsWindow(m_hWnd));
		return DWORD(::SendMessage(m_hWnd, HKM_GETHOTKEY, 0, 0L));
	}

	void CHotKeyWnd::GetHotKey(WORD &wVirtualKeyCode, WORD &wModifiers) const
	{
		DWORD dw = GetHotKey();
		wVirtualKeyCode = LOBYTE(LOWORD(dw));
		wModifiers = HIBYTE(LOWORD(dw));
	}

	void CHotKeyWnd::SetRules(WORD wInvalidComb, WORD wModifiers)
	{ 
		ASSERT(::IsWindow(m_hWnd));  
		::SendMessage(m_hWnd, HKM_SETRULES, wInvalidComb, MAKELPARAM(wModifiers, 0)); 
	}

	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CHotKeyUI)

	CHotKeyUI::CHotKeyUI() : m_pWindow(NULL), m_wVirtualKeyCode(0), m_wModifiers(0)
		                     , m_uButtonState(0), m_dwNativeBkColor(0xFFFFFFFF)
		                     , m_dwHotBorderColor(0)
	{
		m_sText = _T("none");
	}

	LPCTSTR CHotKeyUI::GetClass() const
	{
		return DUI_CTR_HOTKEY;
	}

	LPVOID CHotKeyUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_HOTKEY) == 0 ) return static_cast<CHotKeyUI *>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CHotKeyUI::GetControlFlags() const
	{
		if( !IsEnabled() ) return CControlUI::GetControlFlags();

		return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
	}

	void CHotKeyUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
		{
			::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
			return;
		}
		if( event.Type == UIEVENT_WINDOWSIZE )
		{
			if( m_pWindow != NULL ) m_pManager->SetFocusNeeded(this);
		}
		if( event.Type == UIEVENT_SCROLLWHEEL )
		{
			if( m_pWindow != NULL ) return;
		}
		if( event.Type == UIEVENT_SETFOCUS && IsEnabled() ) 
		{
			// 其他情况应该让基类继续处理，以便正确设置 focus 状态 orbit 20230306
			if (m_pWindow == NULL)
			{
				m_pWindow = new CHotKeyWnd();
				ASSERT(m_pWindow);
				m_pWindow->Init(this);
				m_pWindow->ShowWindow();
			}
		}

		if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() ) 
		{
			Invalidate();
		}

		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN) 
		{
			if( IsEnabled() ) {
				GetManager()->ReleaseCapture();
				if( IsFocused() && m_pWindow == NULL ) {
					m_pWindow = new CHotKeyWnd();
					ASSERT(m_pWindow);
				}
				if (m_pWindow != NULL)
				{
					m_pWindow->Init(this);
					m_pWindow->ShowWindow();
				}

			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE ) 
		{
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP ) 
		{
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CLabelUI::DoEvent(event);
	}

	void CHotKeyUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = 0;
		}
	}

	void CHotKeyUI::SetText(LPCTSTR pstrText)
	{
		m_sText = pstrText;
		if( m_pWindow != NULL ) Edit_SetText(*m_pWindow, m_sText);
		Invalidate();
	}

	void CHotKeyUI::SetNativeBkColor(DWORD dwBkColor)
	{
		m_dwNativeBkColor = dwBkColor;
	}

	DWORD CHotKeyUI::GetNativeBkColor() const
	{
		return m_dwNativeBkColor;
	}

	void CHotKeyUI::SetHotBorderColor(DWORD dwColor)
	{
		m_dwHotBorderColor = dwColor;
	}

	DWORD CHotKeyUI::GetHotBorderColor() const
	{
		return m_dwHotBorderColor;
	}

	void CHotKeyUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
		if( m_pWindow != NULL ) {
			RECT rcPos = m_pWindow->CalPos();
			::SetWindowPos(m_pWindow->GetHWND(), NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left, 
				rcPos.bottom - rcPos.top, SWP_NOZORDER | SWP_NOACTIVATE);        
		}
	}

	void CHotKeyUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
		if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
	}

	void CHotKeyUI::SetInternVisible(bool bVisible)
	{
		if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
	}

	SIZE CHotKeyUI::EstimateSize(SIZE szAvailable)
	{
		if( m_cxyFixed.cy == 0 ) return CDuiSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 6);
		return CControlUI::EstimateSize(szAvailable);
	}

	void CHotKeyUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("nativebkcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetNativeBkColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("hotbordercolor")) == 0)
		{
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotBorderColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("bindkey")) == 0) {
			SetHotKey(pstrValue);
		}
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	DWORD CHotKeyUI::GetStatusBorderColor() const
	{
		if (((m_uButtonState & UISTATE_HOT) != 0) && (m_dwHotBorderColor != 0)) {
		if (!IsFocused())
			return m_dwHotBorderColor;
		}
		
		return CControlUI::GetStatusBorderColor();
	}

	void CHotKeyUI::PaintText(HDC hDC)
	{
		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();
		CDuiString sText = GetText();
		if( sText.IsEmpty() ) return;

		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;
		DWORD dwTextColor = m_dwTextColor;
		if(!IsEnabled())dwTextColor = m_dwDisabledTextColor;

		CRenderEngine::DrawText(hDC, m_pManager, rc, sText, dwTextColor, m_iFont, DT_SINGLELINE | m_uTextStyle);
	}

	void CHotKeyUI::PaintBkColor(HDC hDC)
	{
		/* orbit 20230311
		当伴生窗口出现时，伴生窗口不能设置透明背景颜色（会出现显示异常），所以当伴生窗口出现时，
		用伴生窗口的 Native 背景色填充背景，可以填充伴生窗口没有覆盖到的空隙
		*/
		if (m_pWindow != NULL)
		{
			DWORD bkColor = GetNativeBkColor();
			CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(bkColor));
		}
		else
		{
			CLabelUI::PaintBkColor(hDC);
		}
	}

	DWORD CHotKeyUI::GetHotKey() const
	{
		return (MAKEWORD(m_wVirtualKeyCode, m_wModifiers));
	}

	void CHotKeyUI::GetHotKey(WORD &wVirtualKeyCode, WORD &wModifiers) const
	{
		wVirtualKeyCode = m_wVirtualKeyCode;
		wModifiers = m_wModifiers;
	}

	void CHotKeyUI::SetHotKey(WORD wVirtualKeyCode, WORD wModifiers)
	{
		m_wVirtualKeyCode = wVirtualKeyCode;
		m_wModifiers = wModifiers;
		
		if (m_wVirtualKeyCode == 0)
		{
			m_wModifiers = 0;
			m_sText = _T("none");
		}
		else
		{
			m_sText = GetHotKeyName(m_wVirtualKeyCode, m_wModifiers);
		}
		/*
		if( m_pWindow ) return;
		m_pWindow = new CHotKeyWnd();
		ASSERT(m_pWindow);
		m_pWindow->Init(this);
		*/
		Invalidate();
	}

	void CHotKeyUI::SetHotKey(LPCTSTR pstrKey)
	{
		WORD wVirtualKeyCode = 0;
		WORD wModifiers = 0;
		std::vector<CDuiString> parts = SplitString(pstrKey, _T("+"));
		if ((parts.size() < 2) || (parts[parts.size() - 1].GetLength() != 1))
		{
			wVirtualKeyCode = 0;
			wModifiers = 0;
		}
		else
		{
			wModifiers = 0;
			for (std::size_t i = 0; i < parts.size() - 1; i++)
			{
				if (_tcsicmp(parts[i], _T("ctrl")) == 0)
					wModifiers |= HOTKEYF_CONTROL;
				if (_tcsicmp(parts[i], _T("shift")) == 0)
					wModifiers |= HOTKEYF_SHIFT;
				if (_tcsicmp(parts[i], _T("alt")) == 0)
					wModifiers |= HOTKEYF_ALT;
			}
			wVirtualKeyCode = parts[parts.size() - 1][0];
			if ((wVirtualKeyCode >= 97) && (wVirtualKeyCode <= 122))
				wVirtualKeyCode -= 32;
		}

		SetHotKey(wVirtualKeyCode, wModifiers);
	}

}// Duilib