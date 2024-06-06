#include "stdafx.h"
#include "WinImplBase.h"

namespace DuiLib
{

//////////////////////////////////////////////////////////////////////////

const TCHAR* DuiWndCloseButtonName = _T("closebtn");
const TCHAR* DuiWndMinButtonName = _T("minbtn");
const TCHAR* DuiWndMaxButtonName = _T("maxbtn");
const TCHAR* DuiWndRestoreButtonName = _T("restorebtn");

void WindowImplBase::OnFinalMessage( HWND hWnd )
{
	m_PaintManager.RemovePreMessageFilter(this);
	m_PaintManager.RemoveNotifier(this);
	m_PaintManager.ReapObjects(m_PaintManager.GetRoot());
}

CControlUI* WindowImplBase::CreateControl(LPCTSTR pstrClass)
{
	return NULL;
}

LRESULT WindowImplBase::ResponseDefaultKeyEvent(WPARAM wParam)
{
	return FALSE;
}

void WindowImplBase::OnSysCloseButtonClick()
{
	Close(IDCANCEL);
}

UINT WindowImplBase::GetClassStyle() const
{
	return CS_DBLCLKS;
}

LRESULT WindowImplBase::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
	LRESULT lr = 0;
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
		case VK_RETURN:
		case VK_ESCAPE:
			bHandled = true;
			lr = ResponseDefaultKeyEvent(wParam);
			bHandled = (lr != 0) ? true : false;
		default:
			break;
		}
	}
	return lr;
}

LRESULT WindowImplBase::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT WindowImplBase::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT WindowImplBase::OnNcActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	//如果异形动画窗口有异常，可调试这里，参考旧的 Duilib 中CPaintManagerUI::MessageHandler开始部分代码
	//if(m_PaintManager.IsLayered()) 
	if( ::IsIconic(*this) ) bHandled = FALSE;
	return (wParam == 0) ? TRUE : FALSE;
}

LRESULT WindowImplBase::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LPRECT pRect=NULL;

	bHandled = TRUE; //直接返回，不再让窗口默认处理
	if ( wParam == TRUE)
	{
		LPNCCALCSIZE_PARAMS pParam = (LPNCCALCSIZE_PARAMS)lParam;
		pRect=&pParam->rgrc[0];
	}
	else
	{
		pRect=(LPRECT)lParam;
	}

	if ( ::IsZoomed(m_hWnd))
	{	// 最大化时，计算当前显示器最适合宽高度
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTONEAREST), &oMonitor);
		CDuiRect rcWork = oMonitor.rcWork;
		CDuiRect rcMonitor = oMonitor.rcMonitor;
		rcWork.Offset(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);

		pRect->right = pRect->left + rcWork.GetWidth();
		pRect->bottom = pRect->top + rcWork.GetHeight();
		return WVR_REDRAW;
	}

	return 0;
}

LRESULT WindowImplBase::OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	//不调用窗口的默认处理
	bHandled = TRUE;
	return 0;
}

LRESULT WindowImplBase::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(*this, &pt);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);
	
	if( !::IsZoomed(*this) )
	{
		RECT rcSizeBox = m_PaintManager.GetSizeBox();
		if( pt.y < rcClient.top + rcSizeBox.top )
		{
			if( pt.x < rcClient.left + rcSizeBox.left ) return HTTOPLEFT;
			if( pt.x > rcClient.right - rcSizeBox.right ) return HTTOPRIGHT;
			return HTTOP;
		}
		else if( pt.y > rcClient.bottom - rcSizeBox.bottom )
		{
			if( pt.x < rcClient.left + rcSizeBox.left ) return HTBOTTOMLEFT;
			if( pt.x > rcClient.right - rcSizeBox.right ) return HTBOTTOMRIGHT;
			return HTBOTTOM;
		}

		if( pt.x < rcClient.left + rcSizeBox.left ) return HTLEFT;
		if( pt.x > rcClient.right - rcSizeBox.right ) return HTRIGHT;
	}

	RECT rcCaption = m_PaintManager.GetCaptionRect();
	if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(pt));
			if( pControl && _tcsicmp(pControl->GetClass(), DUI_CTR_BUTTON) != 0 && 
				_tcsicmp(pControl->GetClass(), DUI_CTR_OPTION) != 0 &&
				_tcsicmp(pControl->GetClass(), DUI_CTR_FADEBUTTON) != 0 &&
				_tcsicmp(pControl->GetClass(), DUI_CTR_TEXT) != 0 )
				return HTCAPTION;
	}

	bHandled = TRUE;
	return HTCLIENT;
}

LRESULT WindowImplBase::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;

	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTONEAREST), &oMonitor);
	CDuiRect rcWork = oMonitor.rcWork;
	CDuiRect rcMonitor = oMonitor.rcMonitor;
	rcWork.Offset(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);

	// 计算最大化时，正确的原点坐标
	lpMMI->ptMaxPosition.x	= rcWork.left;
	lpMMI->ptMaxPosition.y	= rcWork.top;

	lpMMI->ptMaxTrackSize.x =rcWork.GetWidth();
	lpMMI->ptMaxTrackSize.y =rcWork.GetHeight();

	lpMMI->ptMinTrackSize.x =m_PaintManager.GetMinInfo().cx;
	lpMMI->ptMinTrackSize.y =m_PaintManager.GetMinInfo().cy;

	bHandled = FALSE;
	return 0;
}

LRESULT WindowImplBase::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_PaintManager.GetRoundCorner();
#if defined(WIN32) && !defined(UNDER_CE)
	if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
		CDuiRect rcWnd;
		::GetWindowRect(*this, &rcWnd);
		rcWnd.Offset(-rcWnd.left, -rcWnd.top);
		rcWnd.right++; rcWnd.bottom++;
		HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
		::SetWindowRgn(*this, hRgn, TRUE);
		::DeleteObject(hRgn);
	}
#endif
	bHandled = FALSE;
	return 0;
}

LRESULT WindowImplBase::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == SC_CLOSE)
	{
		bHandled = TRUE;
		SendMessage(WM_CLOSE);
		return 0;
	}

	bHandled = TRUE;
	BOOL bZoomed = ::IsZoomed(*this);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if (::IsZoomed(*this) != bZoomed)
	{
		// toggle status of max and restore button
		CControlUI* pbtnMax = static_cast<CControlUI*>(m_PaintManager.FindControl(DuiWndMaxButtonName));         // max button
		if (pbtnMax != NULL)
		{
			pbtnMax->SetVisible(TRUE == bZoomed);
		}
		CControlUI* pbtnRestore = static_cast<CControlUI*>(m_PaintManager.FindControl(DuiWndRestoreButtonName)); // restore button
		if (pbtnRestore != NULL)
		{
			pbtnRestore->SetVisible(FALSE == bZoomed);
		}
	}

	if (m_PaintManager.IsLayered())
	{
		if (SC_RESTORE == (wParam & 0xfff0))
		{
			m_PaintManager.SetLayeredRestoreMark(true);
		}
	}

	return lRes;
}

LRESULT WindowImplBase::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lr = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	bHandled = TRUE;

	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	RECT rcClient;
	::GetClientRect(*this, &rcClient);
	::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
		rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

	m_PaintManager.Init(m_hWnd);
	m_PaintManager.AddPreMessageFilter(this);

	CDialogBuilder builder;
	CControlUI* pRoot=NULL;
	if (CPaintManagerUI::GetResourceType() == UILIB_RESOURCE)
	{
		STRINGorID xml(_ttoi(GetSkinFile().GetData()));
		pRoot = builder.Create(xml, _T("xml"), this, &m_PaintManager);
	}
	else
		pRoot = builder.Create(GetSkinFile().GetData(), NULL, this, &m_PaintManager);
	ASSERT(pRoot);
	if (pRoot==NULL)
	{
		MessageBox(NULL,_T("Fail to load skin resource!"),_T("Duilib"),MB_OK|MB_ICONERROR);
		ExitProcess(1);
		return 0;
	}
	m_PaintManager.AttachDialog(pRoot);
	m_PaintManager.AddNotifier(this);

    CShadowUI *pShadow = m_PaintManager.GetShadow();
    pShadow->ShowShadow(pShadow->IsShowShadow());
    pShadow->Create(&m_PaintManager);

	int iDPI = CDPI::GetMainMonitorDPI();
	m_PaintManager.AdjustWindowByDPI(iDPI);

	return lr;
}

LRESULT WindowImplBase::OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT WindowImplBase::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT WindowImplBase::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT WindowImplBase::OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT WindowImplBase::OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT WindowImplBase::OnNcRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}


LRESULT WindowImplBase::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT WindowImplBase::OnDPIChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int g_dpi = HIWORD(wParam);
    m_PaintManager.ResetDPIResource(g_dpi);

    RECT* const prcNewWindow = (RECT*)lParam;
    ::SetWindowPos(m_hWnd, NULL, prcNewWindow->left, prcNewWindow->top, prcNewWindow->right - prcNewWindow->left,
                   prcNewWindow->bottom - prcNewWindow->top, SWP_NOZORDER | SWP_NOACTIVATE);

    if (m_PaintManager.GetRoot() != NULL) 
        m_PaintManager.GetRoot()->NeedUpdate();

    ::PostMessage(m_hWnd, UIMSG_SET_DPI, 0, 0);

    bHandled = TRUE;
    return 0;
}

LRESULT WindowImplBase::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;

	switch (uMsg)
	{
	case WM_CREATE:			lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
	case WM_CLOSE:			lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
	case WM_DESTROY:		lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
	case WM_NCACTIVATE:		lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
	case WM_NCCALCSIZE:		lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
	case WM_NCPAINT:		lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
	case WM_NCHITTEST:		lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
	case WM_GETMINMAXINFO:	lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
	case WM_SIZE:			lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
	case WM_SYSCOMMAND:		lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
	case WM_KEYDOWN:		lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
	case WM_LBUTTONUP:		lRes = OnLButtonUp(uMsg, wParam, lParam, bHandled); break;
	case WM_LBUTTONDOWN:	lRes = OnLButtonDown(uMsg, wParam, lParam, bHandled); break;
	case WM_NCRBUTTONDOWN:	lRes = OnNcRButtonDown(uMsg, wParam, lParam, bHandled); break;
	case WM_MOUSEMOVE:		lRes = OnMouseMove(uMsg, wParam, lParam, bHandled); break;
    case WM_DPICHANGED:	    lRes = OnDPIChanged(uMsg, wParam, lParam, bHandled); break;
    default:				bHandled = FALSE; break;
	}
	if (bHandled) return lRes;

	if (m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes))
		return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

LONG WindowImplBase::GetStyle()
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~WS_CAPTION;

	return styleValue;
}

BOOL WindowImplBase::OnClick(TNotifyUI& msg)
{
	CDuiString sCtrlName = msg.pSender->GetName();
	if( sCtrlName == DuiWndCloseButtonName)
	{
		OnSysCloseButtonClick();
		return TRUE;
	}
	else if( sCtrlName == DuiWndMinButtonName)
	{ 
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); 
		return TRUE;
	}
	else if( sCtrlName == DuiWndMaxButtonName)
	{ 
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); 
		return TRUE;
	}
	else if( sCtrlName == DuiWndRestoreButtonName)
	{ 
		SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); 
		return TRUE;
	}

	return FALSE;
}

void WindowImplBase::Notify(TNotifyUI& msg)
{
	return CNotifyPump::NotifyPump(msg);
}

}