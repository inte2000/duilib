#include "StdAfx.h"
#include "UIIPAddress.h"
#include "..\Utils\DuiFunctions.h"

namespace DuiLib
{
#define IP_NONE   0
#define IP_UPDATE 1
#define IP_DELETE 2
#define IP_KEEP   3

	class CIPAddressWnd : public CWindowWnd
	{
	public:
		CIPAddressWnd();

		void Init(CIPAddressUI* pOwner);
		RECT CalPos();

		LPCTSTR GetWindowClassName() const;
		LPCTSTR GetSuperClassName() const;
		void OnFinalMessage(HWND hWnd);

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		void UpdateWindowShow(HWND hWnd);
		void UpdateDrawCaret(HWND hWnd, HDC hDC);
		void UpdateDotDraw(HWND hWnd, HDC hDC);
	protected:
		enum {
			DEFAULT_TIMERID = 20,
		};

		CIPAddressUI* m_pOwner;
		HBRUSH m_hBkBrush;
		bool m_bInit;
		bool m_bDrawCaret;
		bool m_bShowWindow; //是否显示伴生 ip 地址窗口
	};

	CIPAddressWnd::CIPAddressWnd() : m_pOwner(NULL), m_hBkBrush(NULL), m_bInit(false), m_bDrawCaret(false), m_bShowWindow(false)
	{
	}

	void CIPAddressWnd::Init(CIPAddressUI* pOwner)
	{
		m_pOwner = pOwner;
		m_pOwner->m_nIPUpdateFlag = IP_NONE;

		m_bShowWindow = true; // 设置显示标志
		if (m_hWnd == NULL)
		{
			INITCOMMONCONTROLSEX   CommCtrl;
			CommCtrl.dwSize = sizeof(CommCtrl);
			CommCtrl.dwICC = ICC_INTERNET_CLASSES;//指定Class
			if (InitCommonControlsEx(&CommCtrl))
			{
				RECT rcPos = CalPos();
				UINT uStyle = WS_CHILD | WS_TABSTOP | WS_GROUP;
				Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
				SetWindowFont(m_hWnd, m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->hFont, TRUE);

				::SendMessage(m_hWnd, IPM_SETRANGE, 0, MAKEIPRANGE(0, 255));
				::SendMessage(m_hWnd, IPM_SETRANGE, 1, MAKEIPRANGE(0, 255));
				::SendMessage(m_hWnd, IPM_SETRANGE, 2, MAKEIPRANGE(0, 255));
				::SendMessage(m_hWnd, IPM_SETRANGE, 3, MAKEIPRANGE(0, 255));
			}
		}
		else
		{
			RECT rcPos = CalPos();
			::SetWindowPos(m_hWnd, NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left,
				rcPos.bottom - rcPos.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		}

		::SendMessage(m_hWnd, IPM_SETADDRESS, 0, m_pOwner->m_dwIP);
		::ShowWindow(m_hWnd, SW_SHOW);//SW_SHOWNOACTIVATE
		::SetFocus(m_hWnd);

		m_bInit = true;    
	}

	RECT CIPAddressWnd::CalPos()
	{
		CDuiRect rcPos = m_pOwner->GetPos();

		//几个edit子窗口本身就会有几个像素的偏移，这里加上 textpadding 就太多了，所以只为 border 留点空间即可
		
		RECT rcBorder = m_pOwner->GetBorderSize();
		rcPos.left += rcBorder.left;
		rcPos.top += rcBorder.top;
		rcPos.right -= rcBorder.right;
		rcPos.bottom -= rcBorder.bottom;
		/*
		RECT rcTextPadding = m_pOwner->GetTextPadding();
		rcPos.left += rcTextPadding.left;
		rcPos.top += rcTextPadding.top;
		rcPos.right -= rcTextPadding.right;
		rcPos.bottom -= rcTextPadding.bottom;
		*/
		/*
		CControlUI* pParent = m_pOwner;
		RECT rcParent;
		while (pParent = pParent->GetParent()) {
			if (!pParent->IsVisible()) {
				rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
				break;
			}
			rcParent = pParent->GetClientPos();
			if (!::IntersectRect(&rcPos, &rcPos, &rcParent)) {
				rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
				break;
			}
		}
		*/
		return rcPos;
	}

	LPCTSTR CIPAddressWnd::GetWindowClassName() const
	{
		return _T("IPAddressWnd");
	}

	LPCTSTR CIPAddressWnd::GetSuperClassName() const
	{
		return WC_IPADDRESS;
	}

	void CIPAddressWnd::OnFinalMessage(HWND hWnd)
	{
		// Clear reference and die
		if (m_hBkBrush != NULL) ::DeleteObject(m_hBkBrush);
		m_pOwner->GetManager()->RemoveNativeWindow(hWnd);
		m_pOwner->m_pWindow = NULL;
		delete this;
	}

	LRESULT CIPAddressWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		//DUI__Trace(_T("CIPAddressWnd::HandleMessage(m_hWnd=%x, uMsg=0x%x, wParam=%x)\n"), m_hWnd, uMsg, wParam);

		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		if (uMsg == WM_CREATE) {
			m_pOwner->GetManager()->AddNativeWindow(m_pOwner, m_hWnd);
			if (m_pOwner->GetManager()->IsLayered()) {
				::SetTimer(m_hWnd, DEFAULT_TIMERID, ::GetCaretBlinkTime(), NULL);
			}
			bHandled = FALSE;
		}
		else if (uMsg == WM_DESTROY) {
			::KillTimer(m_hWnd, DEFAULT_TIMERID);
			bHandled = FALSE;
		}
		else if( uMsg == WM_KILLFOCUS ) {
			bHandled = TRUE;
			return 0;
			//lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		}
		else if (uMsg == WM_CTLCOLOREDIT || uMsg == WM_CTLCOLORSTATIC) {
			if (m_pOwner->GetManager()->IsLayered() && !m_pOwner->GetManager()->IsPainting()) {
				m_pOwner->GetManager()->AddNativeWindow(m_pOwner, m_hWnd);
			}
			//if (m_pOwner->GetManager()->IsLayered())
			{
				DWORD clrColor = m_pOwner->GetBkColor();
				//if (clrColor == 0xFFFFFFFF) return 0;
				::SetBkMode((HDC)wParam, TRANSPARENT);
				DWORD dwTextColor = m_pOwner->GetTextColor();
				::SetTextColor((HDC)wParam, RGB(GetBValue(dwTextColor), GetGValue(dwTextColor), GetRValue(dwTextColor)));
				//if (clrColor < 0xFF000000) {
			    if (clrColor == 0) {
						if (m_hBkBrush != NULL) ::DeleteObject(m_hBkBrush);
					RECT rcWnd = m_pOwner->GetManager()->GetNativeWindowRect(m_hWnd);
					HBITMAP hBmpEditBk = CRenderEngine::GenerateBitmap(m_pOwner->GetManager(), rcWnd, m_pOwner, clrColor);
					m_hBkBrush = ::CreatePatternBrush(hBmpEditBk);
					::DeleteObject(hBmpEditBk);
				}
				else {
					if (m_hBkBrush == NULL) {
						m_hBkBrush = ::CreateSolidBrush(RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
					}
				}
				return (LRESULT)m_hBkBrush;
			}

			bHandled = FALSE;
		}
		else if (uMsg == WM_ERASEBKGND) {
			//lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
			return 1;
		}
		else if (uMsg == WM_NCPAINT) {
			/*阻止伴生窗口画黑丝窗口边框*/
			lRes = OnNcPaint(uMsg, wParam, lParam, bHandled);
		}
		else if (uMsg == WM_PAINT) {
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

				DWORD clrColor = m_pOwner->GetBkColor();
				clrColor |= 0xFF000000; // WM_CTLCOLOREDIT 中创建的背景颜色刷是不支持透明度的，为了显示一致性，这里画背景的时候也将透明度设置为 0xFF
				CRenderEngine::DrawColor((HDC)wParam, rcWnd, clrColor);
				//DWORD dwPenColor = m_pOwner->GetBorderColor();
				//CRenderEngine::DrawRect((HDC)wParam, rcClient, 1, dwPenColor);
				lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
				UpdateDotDraw(m_hWnd, (HDC)wParam);
				if (m_pOwner->IsEnabled() && m_bDrawCaret) { // todo:判断是否enabled
					UpdateDrawCaret(m_hWnd, (HDC)wParam);
				}				
				return lRes;
			}
			bHandled = FALSE;
		}
		else if (uMsg == WM_KEYUP && (wParam == VK_DELETE || wParam == VK_BACK)) {
			LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
			m_pOwner->m_nIPUpdateFlag = IP_DELETE;
			m_pOwner->UpdateText();
			PostMessage(WM_CLOSE);
			return lRes;
		}
		else if (uMsg == WM_KEYUP && wParam == VK_ESCAPE) {
			LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
			m_pOwner->m_nIPUpdateFlag = IP_KEEP;
			PostMessage(WM_CLOSE);
			return lRes;
		}
		else if( uMsg == OCM_COMMAND ) {
			if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS ) {
				lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
			}
		}
		else if (uMsg == WM_WINDOWPOSCHANGING) {
			/*当窗口大小发生变化的时候，伴随这个消息的 flags 参数会带有SWP_SHOWWINDOW，
			此时要根据显示标志判断是否要清除这个标志，避免伴生窗口被不正确地显示出来*/
			LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;
			if ((pwp->flags & SWP_SHOWWINDOW) == SWP_SHOWWINDOW) {
				if (!m_bShowWindow) {
					pwp->flags &=  ~SWP_SHOWWINDOW;
				}
			}
		}
		else if (uMsg == WM_TIMER) {
			if (wParam == DEFAULT_TIMERID) {
				m_bDrawCaret = !m_bDrawCaret;
				UpdateWindowShow(m_hWnd);
				return 0;
			}
			bHandled = FALSE;
		}
		else bHandled = FALSE;
		if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		return lRes;
	}

	LRESULT CIPAddressWnd::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//HDC hDC = ::GetDCEx(m_hWnd, (HRGN)wParam, DCX_WINDOW | DCX_INTERSECTRGN);
		HDC hDC = ::GetWindowDC(m_hWnd);
		RECT rcWnd;
		::GetWindowRect(m_hWnd, &rcWnd);
		::OffsetRect(&rcWnd, -rcWnd.left, -rcWnd.top);

		//DWORD clrBack = m_pOwner->GetBkColor();
		DWORD clrBack = m_pOwner->GetNativeBkColor();
		for (int i = 0; i < 2; i++)
		{
			CRenderEngine::DrawRect(hDC, rcWnd, 1, clrBack);
			::InflateRect(&rcWnd, -1, -1);
		}

		::ReleaseDC(m_hWnd, hDC);
		
		bHandled = TRUE;
		return 0;
	}

	LRESULT CIPAddressWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		if (!m_pOwner->GetManager()->IsLayered()) //layer 窗口在 WM_PRINT 里画
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(m_hWnd, &ps);

			RECT rcWnd;
			::GetWindowRect(m_hWnd, &rcWnd);
			RECT rcClient = { 0, 0, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top };
			//DWORD clrColor = m_pOwner->GetBkColor();
			DWORD clrColor = m_pOwner->GetNativeBkColor();

			CRenderEngine::DrawColor(hDC, rcClient, clrColor);
			UpdateDotDraw(m_hWnd, hDC);

			EndPaint(m_hWnd, &ps);
			bHandled = TRUE;
		}

		return 0;
	}

	LRESULT CIPAddressWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HWND hWndFocus = GetFocus();
		while (hWndFocus)
		{
			if (GetFocus() == m_hWnd)
			{
				bHandled = TRUE;
				return 0;
			}
			hWndFocus = GetParent(hWndFocus);
		}

		LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		if (m_pOwner->m_nIPUpdateFlag == IP_NONE)
		{
			DWORD dwNewIP = 0;
			::SendMessage(m_hWnd, IPM_GETADDRESS, 0, (LPARAM)&dwNewIP);
			if (m_pOwner->m_dwIP != dwNewIP)
			{
				m_pOwner->m_dwIP = dwNewIP;
				m_pOwner->m_nIPUpdateFlag = IP_UPDATE;
				m_pOwner->UpdateText();
				m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_EVT_IPADDRESSCHANGED, (WPARAM)m_pOwner->m_dwIP);
			}
		}

		::ShowWindow(m_hWnd, SW_HIDE);
		m_bShowWindow = false; // 清除显示标志
		return lRes;
	}
	/*
	static BOOL InvalidChildEdit(HWND hwnd, LPARAM lParam)
	{
		RECT rcClient;
		::GetClientRect(hwnd, &rcClient);
		::InvalidateRect(hwnd, &rcClient, FALSE);

		return TRUE;
	}
	*/
	void CIPAddressWnd::UpdateWindowShow(HWND hWnd)
	{
		if (::IsWindowVisible(hWnd))
		{
			RECT rcClient;
			::GetClientRect(hWnd, &rcClient);
			::InvalidateRect(hWnd, &rcClient, TRUE);
		}

		//::EnumChildWindows(hWnd, InvalidChildEdit, NULL);
	}

	typedef struct tagUpdateChildParam
	{
		HWND hIpAddr;
		HWND hFocusEdit;
		HDC hDC;
	}UpdateChildParam;

	static BOOL CALLBACK UpdateChildEditCaret(HWND hwnd, LPARAM lParam)
	{
		UpdateChildParam* pParam = (UpdateChildParam*)lParam;
		if (hwnd == pParam->hFocusEdit)
		{
			POINT ptCaret;
			::GetCaretPos(&ptCaret);

			::ClientToScreen(hwnd, &ptCaret);
			::ScreenToClient(pParam->hIpAddr, &ptCaret);

			RECT rcClient;
			::GetClientRect(pParam->hIpAddr, &rcClient);
			RECT rcCaret = { ptCaret.x + 2, ptCaret.y + 2, ptCaret.x + 2, ptCaret.y + rcClient.bottom - rcClient.top };
			CRenderEngine::DrawLine(pParam->hDC, rcCaret, 1, 0xFF000000);
		}

		return TRUE;
	}

	void CIPAddressWnd::UpdateDrawCaret(HWND hWnd, HDC hDC)
	{
		UpdateChildParam param;

		if (::IsWindowVisible(hWnd))
		{
			param.hFocusEdit = ::GetFocus();
			param.hIpAddr = hWnd;
			param.hDC = hDC;
			::EnumChildWindows(hWnd, UpdateChildEditCaret, (LPARAM)&param);
		}
	}

	typedef struct tagUpdateDotDrawParam
	{
		HWND hIpAddr;
		HDC  hDC;
		RECT rcIpAddr;
	}UpdateDotDrawParam;

	static BOOL CALLBACK UpdateIpAddrDot(HWND hwnd, LPARAM lParam)
	{
		UpdateDotDrawParam* pParam = (UpdateDotDrawParam*)lParam;
		
		RECT rcEdit;
		::GetWindowRect(hwnd, &rcEdit);

		//这个edit 子窗口右边界到整个窗口的右边界距离大于一个子窗口的宽度，说明这个地方需要画一个点
		if ((pParam->rcIpAddr.right - rcEdit.right) > (rcEdit.right - rcEdit.left))
		{
			int btLvl = (pParam->rcIpAddr.bottom - pParam->rcIpAddr.top) / 4 + 3;
			POINT center = { rcEdit.right + 1, pParam->rcIpAddr.bottom - btLvl };
			::ScreenToClient(pParam->hIpAddr, &center);
			CRenderEngine::DrawEllipseColor(pParam->hDC, center.x, center.y, 3, 3, 0xFF000000);
		}

		return TRUE;
	}

	void CIPAddressWnd::UpdateDotDraw(HWND hWnd, HDC hDC)
	{
		UpdateDotDrawParam param;

		param.hIpAddr = hWnd;
		param.hDC = hDC;
		::GetWindowRect(hWnd, &param.rcIpAddr);

		::EnumChildWindows(hWnd, UpdateIpAddrDot, (LPARAM)&param);
	}
	
	//////////////////////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CIPAddressUI)

	CIPAddressUI::CIPAddressUI()
	{
		m_dwIP = 0x0;
		m_bReadOnly = false;
		m_pWindow = NULL;
		m_nIPUpdateFlag=IP_UPDATE;
		UpdateText();
		m_nIPUpdateFlag = IP_NONE;
		m_dwNativeBkColor = 0xFFFFFFFF;
	}

	LPCTSTR CIPAddressUI::GetClass() const
	{
		return DUI_CTR_IPADDRESS;
	}

	LPVOID CIPAddressUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_IPADDRESS) == 0 ) return static_cast<CIPAddressUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CIPAddressUI::GetControlFlags() const
	{
		return UIFLAG_TABSTOP;
	}

	HWND CIPAddressUI::GetNativeWindow() const
	{
		if (m_pWindow) return m_pWindow->GetHWND();
		return NULL;
	}

	DWORD CIPAddressUI::GetIP() const
	{
		return m_dwIP;
	}

	void CIPAddressUI::SetIP(DWORD dwIP)
	{
		m_dwIP = dwIP;
		UpdateText();
	}

	CDuiString CIPAddressUI::GetIPString() const
	{
		return GetText();
	}

	void CIPAddressUI::SetIPString(LPCTSTR ipStr)
	{
		std::vector<CDuiString> ipPart = SplitString(ipStr, _T("."));
		if (ipPart.size() == 4)
		{
			BYTE nFirst = _ttoi(ipPart[0]);
			BYTE nSecond = _ttoi(ipPart[1]);
			BYTE nThird = _ttoi(ipPart[2]);
			BYTE nFourth = _ttoi(ipPart[3]);

			m_dwIP = MAKELONG(MAKEWORD(nFourth, nThird), MAKEWORD(nSecond, nFirst));

			SetText(ipStr);
			Invalidate();
		}
	}

	void CIPAddressUI::SetReadOnly(bool bReadOnly)
	{
		m_bReadOnly = bReadOnly;
		Invalidate();
	}

	bool CIPAddressUI::IsReadOnly() const
	{
		return m_bReadOnly;
	}

	void CIPAddressUI::SetNativeBkColor(DWORD dwBkColor)
	{
		m_dwNativeBkColor = dwBkColor;
	}

	DWORD CIPAddressUI::GetNativeBkColor() const
	{
		return m_dwNativeBkColor;
	}

	void CIPAddressUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CLabelUI::SetPos(rc, bNeedInvalidate);
		if (m_pWindow != NULL) {
			RECT rcPos = m_pWindow->CalPos();
			if (::IsRectEmpty(&rcPos)) ::ShowWindow(m_pWindow->GetHWND(), SW_HIDE);
			else {
				::SetWindowPos(m_pWindow->GetHWND(), NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left,
					rcPos.bottom - rcPos.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
			}
		}
	}

	void CIPAddressUI::Move(SIZE szOffset, bool bNeedInvalidate)
	{
		CLabelUI::Move(szOffset, bNeedInvalidate);
		if (m_pWindow != NULL) {
			RECT rcPos = m_pWindow->CalPos();
			if (::IsRectEmpty(&rcPos)) ::ShowWindow(m_pWindow->GetHWND(), SW_HIDE);
			else {
				::SetWindowPos(m_pWindow->GetHWND(), NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left,
					rcPos.bottom - rcPos.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
			}
		}
	}

	void CIPAddressUI::DoEvent(TEventUI& event)
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
		if( event.Type == UIEVENT_SETFOCUS && IsEnabled() && !IsReadOnly())
		{
			// 其他情况应该让基类继续处理，以便正确设置 focus 状态 orbit 20230306
			if( m_pWindow == NULL) 
			{
				m_pWindow = new CIPAddressWnd();
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
			if( IsEnabled() && !IsReadOnly()) {
				GetManager()->ReleaseCapture();
				if( IsFocused() && m_pWindow == NULL )
				{
					m_pWindow = new CIPAddressWnd();
					ASSERT(m_pWindow);
				}
				if( m_pWindow != NULL )
				{
					m_pWindow->Init(this);
					m_pWindow->ShowWindow();
					Invalidate(); //20230306 解决边框显示异常问题
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
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			return;
		}

		CLabelUI::DoEvent(event);
	}

	SIZE CIPAddressUI::EstimateSize(SIZE szAvailable)
	{
		if (m_cxyFixed.cx > 0 && m_cxyFixed.cy > 0) {
			return CControlUI::EstimateSize(szAvailable);
			//return m_cxyFixed;
		}

		if (szAvailable.cx != m_szAvailableLast.cx || szAvailable.cy != m_szAvailableLast.cy) {
			m_bNeedEstimateSize = true;
		}

		CDuiString sText = _T("000"); //每段最多三个数字
		if (m_bNeedEstimateSize) {
			m_bNeedEstimateSize = false;
			m_szAvailableLast = szAvailable;
			m_cxyFixedLast = m_cxyFixed;
			if (m_cxyFixedLast.cy == 0) {
				m_cxyFixedLast.cy = m_pManager->GetFontInfo(m_iFont)->tm.tmHeight + 4; //+8 20220419 orbit
				m_cxyFixedLast.cy += (m_rcTextPadding.top + m_rcTextPadding.bottom);
			}
			if (m_cxyFixedLast.cx == 0) {
				RECT rcText = { 0, 0, 9999, m_cxyFixedLast.cy };
				if (m_bShowHtml) {
					int nLinks = 0;
					CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), m_pManager, rcText, sText, 0, NULL, NULL, nLinks, m_iFont, DT_CALCRECT | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				}
				else {
					CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, sText, 0, m_iFont, DT_CALCRECT | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				}

				m_cxyFixedLast.cx = m_rcTextPadding.left + m_rcTextPadding.right;
				m_cxyFixedLast.cx += (rcText.right - rcText.left) * 4;
				m_cxyFixedLast.cx += 12;
			}
		}

		return GetScaleSize(m_cxyFixedLast, m_nScale);
	}

	void CIPAddressUI::SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue )
	{
        if (_tcscmp(pstrName, _T("ip")) == 0) SetIPString(pstrValue);
		else if (_tcscmp(pstrName, _T("readonly")) == 0) SetReadOnly(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("nativebkcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetNativeBkColor(clrColor);
		}
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CIPAddressUI::PaintText(HDC hDC)
	{
		if (m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if (m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		DWORD mCurTextColor = IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor;
		
		CDuiString ipPart[4];
		SplitIp2Text(m_dwIP, ipPart);

		RECT rc = m_rcItem;
		RECT rcBorder = GetScaleRect(m_rcBorderSize, m_nScale);
		RECT rcTextPadding = GetScaleRect(m_rcTextPadding, m_nScale);
		rc.left += rcTextPadding.left;
		rc.right -= rcTextPadding.right;
		rc.top += rcTextPadding.top;
		rc.bottom -= rcTextPadding.bottom;

		int ptDev = (m_rcItem.bottom - m_rcItem.top - rcBorder.top - rcBorder.bottom) / 4 + 3;
		if (GetManager()->IsLayered())
			ptDev += 2;

		int width = (rc.right - rc.left - 12) / 4;
		RECT rcText = { rc.left + 4, rc.top, rc.left + width, rc.bottom };
		CRenderEngine::DrawText(hDC, m_pManager, rcText, ipPart[0], mCurTextColor, m_iFont, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		POINT center = { rcText.right + 2, m_rcItem.bottom - rcBorder.bottom - ptDev };
		CRenderEngine::DrawEllipseColor(hDC, center.x, center.y, 3, 3, 0xFF000000);

		rcText.left = rcText.right + 3;
		rcText.right = rcText.left + width;
		CRenderEngine::DrawText(hDC, m_pManager, rcText, ipPart[1], mCurTextColor, m_iFont, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		center = { rcText.right + 2, m_rcItem.bottom - rcBorder.bottom - ptDev };
		CRenderEngine::DrawEllipseColor(hDC, center.x, center.y, 3, 3, 0xFF000000);

		rcText.left = rcText.right + 3;
		rcText.right = rcText.left + width;
		CRenderEngine::DrawText(hDC, m_pManager, rcText, ipPart[2], mCurTextColor, m_iFont, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		center = { rcText.right + 2, m_rcItem.bottom - rcBorder.bottom - ptDev };
		CRenderEngine::DrawEllipseColor(hDC, center.x, center.y, 3, 3, 0xFF000000);

		rcText.left = rcText.right + 3;
		rcText.right = rcText.left + width;
		CRenderEngine::DrawText(hDC, m_pManager, rcText, ipPart[3], mCurTextColor, m_iFont, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}

	void CIPAddressUI::UpdateText()
	{
		if (m_nIPUpdateFlag == IP_DELETE)
			SetText(_T(""));
		else if (m_nIPUpdateFlag == IP_UPDATE)
		{
			TCHAR szIP[MAX_PATH] = { 0 };
			in_addr addr;
			addr.S_un.S_addr = m_dwIP;
			_stprintf(szIP, _T("%d.%d.%d.%d"), addr.S_un.S_un_b.s_b4, addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b2, addr.S_un.S_un_b.s_b1);
			SetText(szIP);
		}
	}

	void CIPAddressUI::SplitIp2Text(DWORD dwIp, CDuiString* strArray)
	{
		in_addr addr;
		addr.S_un.S_addr = m_dwIP;

		strArray[0].Format(_T("%d"), addr.S_un.S_un_b.s_b4);
		strArray[1].Format(_T("%d"), addr.S_un.S_un_b.s_b3);
		strArray[2].Format(_T("%d"), addr.S_un.S_un_b.s_b2);
		strArray[3].Format(_T("%d"), addr.S_un.S_un_b.s_b1);
	}
}
