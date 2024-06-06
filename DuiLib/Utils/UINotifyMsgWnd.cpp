#include "stdafx.h"
#include "UINotifyMsgWnd.h"


static UINT countTimerId = 0x15;
static int ShowStep = 10;

void CNotifyMsgWnd::AdjustWindowByCurrentDPI()
{
	int iDPI = CDPI::GetMainMonitorDPI();

	int scale1 = m_pm.GetDPIObj()->GetScale();
	m_pm.ResetDPIResource(iDPI);
	int scale2 = m_pm.GetDPIObj()->GetScale();
	RECT rcWnd = { 0 };
	::GetWindowRect(m_hWnd, &rcWnd);
	RECT*  prcNewWindow = &rcWnd;
	if (!::IsZoomed(m_hWnd)) {
		RECT rc = rcWnd;
		rc.right = rcWnd.left + (rcWnd.right - rcWnd.left) * scale2 / scale1;
		rc.bottom = rcWnd.top + (rcWnd.bottom - rcWnd.top) * scale2 / scale1;
		prcNewWindow = &rc;
	}
	SetWindowPos(m_hWnd, NULL, prcNewWindow->left, prcNewWindow->top, prcNewWindow->right - prcNewWindow->left, prcNewWindow->bottom - prcNewWindow->top, SWP_NOZORDER | SWP_NOACTIVATE);

	if (m_pm.GetRoot() != NULL)
		m_pm.GetRoot()->NeedUpdate();
	//::PostMessage(GetPaintWindow(), UIMSG_SET_DPI, 0, 0);
}

void CNotifyMsgWnd::LoadSkinXML(STRINGorID xml, LPCTSTR pSkinType)
{
	if (pSkinType != NULL)
		m_sType = pSkinType;

	m_xml = xml;
	Create(::GetDesktopWindow(), NULL, /*WS_VISIBLE | */WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_TOOLWINDOW);
	::ShowWindow(m_hWnd, SW_SHOW);

	HWND hWndParent = ::GetParent(m_hWnd); /*只获取直接父窗口即可*/
	::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
}

void CNotifyMsgWnd::AdjustPostion()
{
	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	int cx = rect.right - rect.left;
	int cy = rect.bottom - rect.top;
		SIZE initSize = m_pm.GetInitSize();

	if (m_Process == 0)
	{
		int showbl = int((ShowStep - m_nMsElapse) * (initSize.cy / ShowStep) + 0.5);
		CDuiRect rcWnd;
		rcWnd.left = (cx - initSize.cx - 15);
		rcWnd.top = (cy - showbl - 35);
		rcWnd.right = rcWnd.left + initSize.cx;
		rcWnd.bottom = rcWnd.top + showbl;
		::SetWindowPos(m_hWnd, HWND_TOPMOST, rcWnd.left, rcWnd.top, rcWnd.GetWidth(), rcWnd.GetHeight(), SWP_NOACTIVATE);
	}
	if (m_Process == 2)
	{
		int showbl = int(m_nMsElapse * (initSize.cy / ShowStep) + 0.5);
		CDuiRect rcWnd;
		rcWnd.left = (cx - initSize.cx - 15);
		rcWnd.top = (cy - showbl - 35);
		rcWnd.right = rcWnd.left + initSize.cx;
		rcWnd.bottom = rcWnd.top + showbl;
		::SetWindowPos(m_hWnd, HWND_TOPMOST, rcWnd.left, rcWnd.top, rcWnd.GetWidth(), rcWnd.GetHeight(), SWP_NOACTIVATE);
	}
}

void CNotifyMsgWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("itemselect")) {
		Close();
	}
	else if (_tcsicmp(msg.sType, kTimer) == 0)
	{
		OnTimer(msg);
	}

	//CWindowWnd::Notify(msg);
}

LRESULT CNotifyMsgWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch (uMsg) {
	case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
	case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
	default:
		bHandled = FALSE;
	}
	if (bHandled) return lRes;
	if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CNotifyMsgWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pm.Init(m_hWnd);
	CDialogBuilder builder;
	CControlUI* pRoot = builder.Create(m_xml, m_sType.GetData(), NULL, &m_pm);
	ASSERT(pRoot && "Failed to parse XML");
	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);
	m_pm.SetRoundCorner(3, 3);

	CShadowUI *pShadow = m_pm.GetShadow();
	pShadow->ShowShadow(pShadow->IsShowShadow());
	pShadow->Create(&m_pm);

	CTextUI* pTime = static_cast<CTextUI*>(m_pm.FindControl(_T("title")));
	if (pTime)
	{
		m_pm.SetTimer(pTime, countTimerId, 100);
	}
	CTextUI* pMessage = static_cast<CTextUI*>(m_pm.FindControl(_T("message")));
	if (pMessage)
	{
		pMessage->SetText(m_Message);
	}

	m_Process = 0;
	m_nMsElapse = 10;
	AdjustPostion();
	AdjustWindowByCurrentDPI();
	return 0;
}

LRESULT CNotifyMsgWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

void CNotifyMsgWnd::OnTimer(TNotifyUI& msg)
{
	if (msg.wParam == countTimerId)
	{
		if (m_Process == 0)
		{
			m_nMsElapse--;
			AdjustPostion();
			if (m_nMsElapse < 0)
			{
				m_Process = 1;
				m_nMsElapse = 50;
			}
		}
		else if (m_Process == 1)
		{
			m_nMsElapse--;
			if (m_nMsElapse < 0)
			{
				m_Process = 2;
				m_nMsElapse = 10;
			}
		}
		else
		{
			m_nMsElapse--;
			AdjustPostion();
			if (m_nMsElapse < 0)
			{
				//RECT rcWnd;
				//GetWindowRect(&rcWnd);
				//SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, MAKELPARAM(rcWnd.left, rcWnd.top));
				ShowWindow(SW_HIDE);
				//PostMessage(WM_CLOSE);
				Close();
			}
		}
	}
}

void ShowNotifyMsgWnd(LPCTSTR msg)
{
	CNotifyMsgWnd* pWnd = new CNotifyMsgWnd();
	//::SetForegroundWindow(m_hHostWnd);//it is important that ...
	pWnd->SetMessage(msg);							  //point.y  -= 10;
	pWnd->LoadSkinXML(_T("notify_msg_wnd.xml"), 0);
}

