#include "StdAfx.h"
#include "UIMenu.h"
#include "UICommonSkin.h"


namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
ContextMenuObserver s_context_menu_observer;

// MenuUI
IMPLEMENT_DUICONTROL(CMenuUI)

CMenuUI::CMenuUI()
{
	if (GetHeader() != NULL)
		GetHeader()->SetVisible(false);
}

CMenuUI::~CMenuUI()
{}

LPCTSTR CMenuUI::GetClass() const
{
    return DUI_CTR_MENU;
}

LPVOID CMenuUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcscmp(pstrName, DUI_CTR_MENU) == 0 ) return static_cast<CMenuUI*>(this);
    return CListUI::GetInterface(pstrName);
}

void CMenuUI::DoEvent(TEventUI& event)
{
	return __super::DoEvent(event);
}

bool CMenuUI::Add(CControlUI* pControl)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(DUI_CTR_MENUELEMENT));
	if (pMenuItem == NULL)
		return false;

	for (int i = 0; i < pMenuItem->GetCount(); ++i)
	{
		if (pMenuItem->GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT) != NULL)
		{
			(static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT)))->SetInternVisible(false);
		}
	}
	return CListUI::Add(pControl);
}

bool CMenuUI::AddAt(CControlUI* pControl, int iIndex)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(DUI_CTR_MENUELEMENT));
	if (pMenuItem == NULL)
		return false;

	for (int i = 0; i < pMenuItem->GetCount(); ++i)
	{
		if (pMenuItem->GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT) != NULL)
		{
			(static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT)))->SetInternVisible(false);
		}
	}
	return CListUI::AddAt(pControl, iIndex);
}

int CMenuUI::GetItemIndex(CControlUI* pControl) const
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(DUI_CTR_MENUELEMENT));
	if (pMenuItem == NULL)
		return -1;

	return __super::GetItemIndex(pControl);
}

bool CMenuUI::SetItemIndex(CControlUI* pControl, int iIndex)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(DUI_CTR_MENUELEMENT));
	if (pMenuItem == NULL)
		return false;

	return __super::SetItemIndex(pControl, iIndex);
}

bool CMenuUI::Remove(CControlUI* pControl, bool bDoNotDestroy)
{
	CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(DUI_CTR_MENUELEMENT));
	if (pMenuItem == NULL)
		return false;

	return __super::Remove(pControl, bDoNotDestroy);
}

SIZE CMenuUI::EstimateSize(SIZE szAvailable)
{
	int cxFixed = 0;
    int cyFixed = 0;
    for( int it = 0; it < GetCount(); it++ ) {
        CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
        if( !pControl->IsVisible() ) continue;
        SIZE sz = pControl->EstimateSize(szAvailable);
        cyFixed += sz.cy;
		if( cxFixed < sz.cx )
			cxFixed = sz.cx;
    }
    return CDuiSize(cxFixed, cyFixed);
}

void CMenuUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	CListUI::SetAttribute(pstrName, pstrValue);
}

/////////////////////////////////////////////////////////////////////////////////////
//

CMenuWnd::CMenuWnd():
m_pOwner(NULL),
m_pLayout(NULL),
m_xml(_T(""))
{}

BOOL CMenuWnd::Receive(ContextMenuParam param)
{
	switch (param.wParam)
	{
	case 1:
		Close();
		break;
	case 2:
		{
			HWND hParent = GetParent(m_hWnd);
			while (hParent != NULL)
			{
				if (hParent == param.hWnd)
				{
					Close();
					break;
				}
				hParent = GetParent(hParent);
			}
		}
		break;
	default:
		break;
	}

	return TRUE;
}
/*
void SetMenuItemEnable(LPCTSTR itemName, bool bEnable) {
	CListContainerElementUI* elemUI = dynamic_cast<CListContainerElementUI*>(m_pm.FindControl(itemName));
	if (elemUI != NULL) {
		elemUI->SetEnabled(bEnable);
		CControlUI* pControl = elemUI->FindSubControl(_T("menu_text"));
		if (pControl) {
			pControl->SetEnabled(bEnable);
		}
	}
}
*/
void CMenuWnd::SetMenuItemText(LPCTSTR itemName, LPCTSTR lpstrText) 
{
	for (int i = 0; i < m_pLayout->GetCount(); i++)
	{
		CMenuElementUI* itemUI = static_cast<CMenuElementUI*>(m_pLayout->GetItemAt(i));
		if (itemUI->GetName().Compare(itemName) == 0)
		{
			itemUI->SetText(lpstrText);
			break;
		}

	}
}

void CMenuWnd::SetMenuItemText(UINT id, LPCTSTR lpstrText)
{
	for (int i = 0; i < m_pLayout->GetCount(); i++)
	{
		CMenuElementUI* itemUI = static_cast<CMenuElementUI*>(m_pLayout->GetItemAt(i));
		if (itemUI->GetID() == id)
		{
			itemUI->SetText(lpstrText);
			break;
		}
	}
}

void CMenuWnd::DeleteMenuItem(LPCTSTR itemName) 
{
	for (int i = 0; i < m_pLayout->GetCount(); i++)
	{
		CMenuElementUI* itemUI = static_cast<CMenuElementUI*>(m_pLayout->GetItemAt(i));
		if (itemUI->GetName().Compare(itemName) == 0)
		{
			int itemHeight = itemUI->GetFixedHeight();
			m_pLayout->Remove(itemUI);

			CDuiRect rcWnd;
			GetWindowRect(m_hWnd, &rcWnd);
			rcWnd.bottom -= itemHeight;
			::SetWindowPos(m_hWnd, NULL, rcWnd.left, rcWnd.top, rcWnd.GetWidth(), rcWnd.GetHeight(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

			break;
		}
	}
}

void CMenuWnd::SetMenuItemEnable(LPCTSTR itemName, bool bEnable)
{
	for (int i = 0; i < m_pLayout->GetCount(); i++)
	{
		CMenuElementUI* itemUI = static_cast<CMenuElementUI*>(m_pLayout->GetItemAt(i));
		if (itemUI->GetName().Compare(itemName) == 0)
		{
			itemUI->SetEnabled(bEnable);
			break;
		}
	}
}

void CMenuWnd::LoadMenu(CMenuElementUI* pOwner, STRINGorID xml, LPCTSTR pSkinType, CPaintManagerUI* pMsgManger)
{
	m_pOwner = pOwner;
	m_pLayout = NULL;

	if (pSkinType != NULL)
		m_sType = pSkinType;

	m_xml = xml;
	if (pOwner == NULL)
	{
		s_context_menu_observer.SetNotifyPaintManager(pMsgManger);
	}
	s_context_menu_observer.AddReceiver(this);

	Create((m_pOwner == NULL) ? pMsgManger->GetPaintWindow() : m_pOwner->GetManager()->GetPaintWindow(), NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST, CDuiRect());
	// HACK: Don't deselect the parent's caption
	HWND hWndParent = m_hWnd;
	while (::GetParent(hWndParent) != NULL) hWndParent = ::GetParent(hWndParent);
	::ShowWindow(m_hWnd, SW_SHOW);
#if defined(WIN32) && !defined(UNDER_CE)
	::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
#endif	
}

void CMenuWnd::TrackMenu(POINT point, DWORD dwAlignment)
{
	if (m_pOwner != NULL)
	{
		//多级菜单在 OnCreate 中处理位置
	}
	else
	{
#if defined(WIN32) && !defined(UNDER_CE)
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CDuiRect rcWork = oMonitor.rcWork;
#else
		CDuiRect rcWork;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWork);
#endif
		CDuiRect rcWnd;
		GetWindowRect(m_hWnd, &rcWnd);

		CDuiRect rc;
		rc.left = point.x; //默认是 eMenuAlignment_Left
		rc.top = point.y; //默认是 eMenuAlignment_Top
		rc.right = rc.left + rcWnd.GetWidth();
		rc.bottom = rc.top + rcWnd.GetHeight();

		if (dwAlignment & eMenuAlignment_Right)
		{
			rc.right = point.x;
			rc.left = rc.right - rcWnd.GetWidth();
		}
		if (dwAlignment & eMenuAlignment_Bottom)
		{
			rc.bottom = point.y;
			rc.top = rc.bottom - rcWnd.GetHeight();
		}
		//避免超出屏幕边界
		if (rc.bottom > rcWork.bottom) {
			if (rcWnd.GetHeight() >= rcWork.GetHeight()) {
				rc.top = 0;
				rc.bottom = rcWnd.GetHeight();
			}
			else {
				rc.bottom = rcWork.bottom;
				rc.top = rc.bottom - rcWnd.GetHeight();
			}
		}
		if (rc.right > rcWork.right) {
			if (rcWnd.GetWidth() >= rcWork.GetWidth()) {
				rc.left = 0;
				rc.right = rcWnd.GetWidth();
			}
			else {
				rc.right = rcWork.right;
				rc.left = rc.right - rcWnd.GetWidth();
			}
		}
		if (rc.left < 0) {
			rc.left = 0;
			rc.right = rc.left + rcWnd.GetWidth();
		}
		if (rc.top < 0) {
			rc.top = 0;
			rc.bottom = rc.top + rcWnd.GetHeight();
		}

		SetForegroundWindow(m_hWnd);
		MoveWindow(m_hWnd, rc.left, rc.top, rc.GetWidth(), rc.GetHeight(), FALSE);
		::SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top, rc.GetWidth(), rc.GetHeight(), SWP_SHOWWINDOW);
	}
}

LPCTSTR CMenuWnd::GetWindowClassName() const
{
    return _T("MenuWnd");
}

void CMenuWnd::OnFinalMessage(HWND hWnd)
{
	RemoveObserver();
	if( m_pOwner != NULL ) {
		for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
			if( static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT)) != NULL ) {
				(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetOwner(m_pOwner->GetParent());
				(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetVisible(false);
				(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT)))->SetInternVisible(false);
			}
		}
		m_pOwner->m_pWindow = NULL;
		m_pOwner->m_uButtonState &= ~ UISTATE_PUSHED;
		m_pOwner->Invalidate();
	}
    delete this;
}

LRESULT CMenuWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_pOwner != NULL)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		RECT rcClient;
		::GetClientRect(*this, &rcClient);
		::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
			rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

		m_pm.Init(m_hWnd);
		// The trick is to add the items to the new container. Their owner gets
		// reassigned by this operation - which is why it is important to reassign
		// the items back to the righfull owner/manager when the window closes.
		m_pLayout = new CMenuUI();
		m_pLayout->SetManager(&m_pm, NULL, true);
		LPCTSTR pDefaultAttributes = m_pOwner->GetManager()->GetDefaultAttributeList(DUI_CTR_MENU);
		if (pDefaultAttributes) 
		{
			m_pLayout->SetAttributeList(pDefaultAttributes);
		}
		m_pLayout->SetBkColor(0xFFFFFFFF);
		m_pLayout->SetBorderColor(0xFF85E4FF);
		m_pLayout->SetBorderSize(0);
		m_pLayout->SetAutoDestroy(false);
		m_pLayout->EnableScrollBar();
		for (int i = 0; i < m_pOwner->GetCount(); i++) 
		{
			if (m_pOwner->GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT) != NULL)
			{
				(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetOwner(m_pLayout);
				m_pLayout->Add(static_cast<CControlUI*>(m_pOwner->GetItemAt(i)));
			}
		}
		m_pm.AttachDialog(m_pLayout);

		// Position the popup window in absolute space
		RECT rcOwner = m_pOwner->GetPos();
		RECT rc = rcOwner;

		int cxFixed = 0;
		int cyFixed = 0;

#if defined(WIN32) && !defined(UNDER_CE)
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CDuiRect rcWork = oMonitor.rcWork;
#else
		CDuiRect rcWork;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWork);
#endif
		SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };

		for (int it = 0; it < m_pOwner->GetCount(); it++)
		{
			if (m_pOwner->GetItemAt(it)->GetInterface(DUI_CTR_MENUELEMENT) != NULL)
			{
				CControlUI* pControl = static_cast<CControlUI*>(m_pOwner->GetItemAt(it));
				SIZE sz = pControl->EstimateSize(szAvailable);
				cyFixed += sz.cy;

				if (cxFixed < sz.cx)
					cxFixed = sz.cx;
			}
		}
		cyFixed += 4;
		cxFixed += 4;

		RECT rcWindow;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWindow);

		rc.top = rcOwner.top;
		rc.bottom = rc.top + cyFixed;
		::MapWindowRect(m_pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
		rc.left = rcWindow.right;
		rc.right = rc.left + cxFixed;
		rc.right += 2;

		bool bReachBottom = false;
		bool bReachRight = false;
		LONG chRightAlgin = 0;
		LONG chBottomAlgin = 0;

		RECT rcPreWindow = { 0 };
		ContextMenuObserver::Iterator<BOOL, ContextMenuParam> iterator(s_context_menu_observer);
		ReceiverImplBase<BOOL, ContextMenuParam>* pReceiver = iterator.next();
		while (pReceiver != NULL)
		{
			CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
			if (pContextMenu != NULL)
			{
				GetWindowRect(pContextMenu->GetHWND(), &rcPreWindow);

				bReachRight = rcPreWindow.left >= rcWindow.right;
				bReachBottom = rcPreWindow.top >= rcWindow.bottom;
				if (pContextMenu->GetHWND() == m_pOwner->GetManager()->GetPaintWindow()
					|| bReachBottom || bReachRight)
					break;
			}
			pReceiver = iterator.next();
		}

		if (bReachBottom)
		{
			rc.bottom = rcWindow.top;
			rc.top = rc.bottom - cyFixed;
		}

		if (bReachRight)
		{
			rc.right = rcWindow.left;
			rc.left = rc.right - cxFixed;
		}

		if (rc.bottom > rcWork.bottom)
		{
			rc.bottom = rc.top;
			rc.top = rc.bottom - cyFixed;
		}

		if (rc.right > rcWork.right)
		{
			rc.right = rcWindow.left;
			rc.left = rc.right - cxFixed;

			rc.top = rcWindow.bottom;
			rc.bottom = rc.top + cyFixed;
		}

		if (rc.top < rcWork.top)
		{
			rc.top = rcOwner.top;
			rc.bottom = rc.top + cyFixed;
		}

		if (rc.left < rcWork.left)
		{
			rc.left = rcWindow.right;
			rc.right = rc.left + cxFixed;
		}

		MoveWindow(m_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);
	}
	else 
	{
		m_pm.Init(m_hWnd);

		CDialogBuilder builder;

		CControlUI* pRoot = builder.Create(m_xml, m_sType.GetData(), NULL, &m_pm); 
		m_pm.AttachDialog(pRoot);

		m_pLayout = dynamic_cast<CMenuUI*>(m_pm.FindSubControlByClass(pRoot, DUI_CTR_MENU));
	}

	CShadowUI* pShadow = m_pm.GetShadow();
	pShadow->ShowShadow(pShadow->IsShowShadow());
	pShadow->Create(&m_pm);

	CDuiString skImg = GetCommonBkImage();
	DWORD bkColor = GetCommonBkColor();
	if (!skImg.IsEmpty() || (bkColor != 0))
	{
		m_pm.SetPaintWindowBackground(skImg, bkColor, GetCommonOpacity());
	}

	//int iDPI = CDPI::GetMainMonitorDPI();
	//m_pm.AdjustWindowByDPI(iDPI);

	return 0;
}

LRESULT CMenuWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_pm.GetRoundCorner();
	if (!::IsIconic(*this)) {
		CDuiRect rcWnd;
		::GetWindowRect(*this, &rcWnd);
		rcWnd.Offset(-rcWnd.left, -rcWnd.top);
		rcWnd.right++; rcWnd.bottom++;
		HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
		::SetWindowRgn(*this, hRgn, TRUE);
		::DeleteObject(hRgn);
	}
	bHandled = FALSE;
	return 0;
}

LRESULT CMenuWnd::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt;
	GetCursorPos(&pt);
	HWND hCurWnd = WindowFromPoint(pt);

	ContextMenuObserver::Iterator<BOOL, ContextMenuParam> iterator(s_context_menu_observer);
	ReceiverImplBase<BOOL, ContextMenuParam>* pReceiver = iterator.next();
	while (pReceiver != NULL)
	{
		CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
		if (pContextMenu != NULL)
		{
			if (hCurWnd == pContextMenu->GetHWND())
			{
				::SetCapture(pContextMenu->GetHWND());
				break;
			}
		}
		pReceiver = iterator.next();
	}

	return 0;
}

LRESULT CMenuWnd::OnButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL bInMenuWindowList = FALSE;
	ContextMenuParam param;
	param.hWnd = GetHWND();

	ContextMenuObserver::Iterator<BOOL, ContextMenuParam> iterator(s_context_menu_observer);
	ReceiverImplBase<BOOL, ContextMenuParam>* pReceiver = iterator.next();
	while (pReceiver != NULL)
	{
		CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
		if (pContextMenu != NULL)
		{
			POINT pt;
			GetCursorPos(&pt);
			if (WindowFromPoint(pt) == pContextMenu->GetHWND())
			{
				bInMenuWindowList = TRUE;
				break;
			}
		}
		pReceiver = iterator.next();
	}

	if (!bInMenuWindowList) {
		param.wParam = 1;
		s_context_menu_observer.RBroadcast(param);

		return 0;
	}
	return 0;
}

LRESULT CMenuWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch (uMsg)
	{
	case WM_CREATE:
		lRes = OnCreate(uMsg, wParam, lParam, bHandled);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE || wParam == VK_LEFT)
			Close();
		break;
	case WM_SIZE:
		lRes = OnSize(uMsg, wParam, lParam, bHandled);
		break;
	case WM_CLOSE:
		if (m_pOwner != NULL)
		{
			m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
			m_pOwner->SetPos(m_pOwner->GetPos());
			m_pOwner->SetFocus();
		}
		break;
	case WM_MOUSEMOVE:
		lRes = OnMouseMove(uMsg, wParam, lParam, bHandled);
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		lRes = OnButtonDown(uMsg, wParam, lParam, bHandled);
		break;
	case WM_CONTEXTMENU:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		return 0L;
		break;
	default:
		bHandled = FALSE;
		break;
	}

	if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

/*
LRESULT CMenuWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if( uMsg == WM_CREATE ) {

    }
    else if( uMsg == WM_CLOSE ) {
		if( m_pOwner != NULL )
		{
			m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
			m_pOwner->SetPos(m_pOwner->GetPos(), false);
			m_pOwner->SetFocus();
		}
	}
	else if( uMsg == WM_RBUTTONDOWN || uMsg == WM_CONTEXTMENU || uMsg == WM_RBUTTONUP || uMsg == WM_RBUTTONDBLCLK )
	{
		return 0L;
	}
	else if( uMsg == WM_KILLFOCUS )
	{
		HWND hFocusWnd = (HWND)wParam;

		BOOL bInMenuWindowList = FALSE;
		ContextMenuParam param;
		param.hWnd = GetHWND();

		ContextMenuObserver::Iterator<BOOL, ContextMenuParam> iterator(s_context_menu_observer);
		ReceiverImplBase<BOOL, ContextMenuParam>* pReceiver = iterator.next();
		while( pReceiver != NULL ) {
			CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
			if( pContextMenu != NULL && pContextMenu->GetHWND() ==  hFocusWnd ) {
				bInMenuWindowList = TRUE;
				break;
			}
			pReceiver = iterator.next();
		}

		if( !bInMenuWindowList ) {
			param.wParam = 1;
			s_context_menu_observer.RBroadcast(param);

			return 0;
		}
	}
	else if( uMsg == WM_KEYDOWN)
	{
		if( wParam == VK_ESCAPE)
		{
			Close();
		}
	}

    LRESULT lRes = 0;
    if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
    return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}
*/
/////////////////////////////////////////////////////////////////////////////////////
//

// MenuElementUI
IMPLEMENT_DUICONTROL(CMenuElementUI)

CMenuElementUI::CMenuElementUI(): m_pWindow(NULL)
{
	m_cxyFixed.cy = 25;
	m_bMouseChildEnabled = true;
	m_bSeparator = false;
	m_szIconSize = { 16, 16 };
	//SetMouseChildEnabled(false);
}

CMenuElementUI::~CMenuElementUI()
{}

LPCTSTR CMenuElementUI::GetClass() const
{
	return DUI_CTR_MENUELEMENT;
}

LPVOID CMenuElementUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, DUI_CTR_MENUELEMENT) == 0 ) return static_cast<CMenuElementUI*>(this);
    return CListContainerElementUI::GetInterface(pstrName);
}

void CMenuElementUI::SetIconSize(LONG cx, LONG cy)
{
	m_szIconSize.cx = cx;
	m_szIconSize.cy = cy;
}

void CMenuElementUI::SetIcon(LPCTSTR pStrIcon)
{
	if (m_Icon.sDrawString == pStrIcon && m_Icon.pImageInfo != NULL) return;
	m_Icon.Clear();
	m_Icon.sDrawString = pStrIcon;
	ParseDrawInfo(m_pManager, m_Icon, m_nScale);
	Invalidate();
}

bool CMenuElementUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl)
{
    RECT rcTemp = { 0 };
    if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return true;

    CRenderClip clip;
    CRenderClip::GenerateClip(hDC, rcTemp, clip);

	if (m_bSeparator)
	{
		RECT rcLine = { m_rcItem.left + DEFAULT_LINE_LEFT_WIDTH, m_rcItem.bottom - 5, m_rcItem.right - DEFAULT_LINE_LEFT_WIDTH, m_rcItem.bottom - 5 };
		CRenderEngine::DrawLine(hDC, rcLine, 1, GetBkColor());
	}
	else
	{
		DrawItemBk(hDC, m_rcItem);
		DrawItemText(hDC, m_rcItem);
		DrawItemIcon(hDC, m_rcItem);
		for (int i = 0; i < GetCount(); ++i)
		{
			if (GetItemAt(i)->GetInterface(_T("MenuElement")) == NULL)
				GetItemAt(i)->DoPaint(hDC, rcPaint, pStopControl);
		}
	}

    return true;
}

void CMenuElementUI::DrawItemText(HDC hDC, const RECT& rcItem)
{
	CDuiString sText = GetText();
	if (sText.IsEmpty()) return;

    if( m_pOwner == NULL ) return;
    TListInfoUI* pInfo = m_pOwner->GetListInfo();
    DWORD iTextColor = pInfo->dwTextColor;
    if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        iTextColor = pInfo->dwHotTextColor;
    }
    if( IsSelected() ) {
        iTextColor = pInfo->dwSelectedTextColor;
    }
    if( !IsEnabled() ) {
        iTextColor = pInfo->dwDisabledTextColor;
    }
    int nLinks = 0;
    RECT rcText = rcItem;
	RECT rcTextPadding = GetScaleRect(pInfo->rcTextPadding, m_nScale);
    rcText.left += rcTextPadding.left;
    rcText.right -= rcTextPadding.right;
    rcText.top += rcTextPadding.top;
    rcText.bottom -= rcTextPadding.bottom;

    if( pInfo->bShowHtml )
        CRenderEngine::DrawHtmlText(hDC, m_pManager, rcText, sText, iTextColor, \
        NULL, NULL, nLinks, pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
    else
        CRenderEngine::DrawText(hDC, m_pManager, rcText, sText, iTextColor, \
        pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
}

void CMenuElementUI::DrawItemIcon(HDC hDC, const RECT& rcItem)
{
	if (m_Icon.pImageInfo != NULL)
	{
		m_Icon.rcDestOffset.left = (ITEM_DEFAULT_ICON_WIDTH - m_szIconSize.cx) / 2;
		m_Icon.rcDestOffset.top = (m_cxyFixed.cy - m_szIconSize.cy) / 2;
		m_Icon.rcDestOffset.right = (ITEM_DEFAULT_ICON_WIDTH - m_szIconSize.cx) / 2 + m_szIconSize.cx;
		m_Icon.rcDestOffset.bottom = (m_cxyFixed.cy - m_szIconSize.cy) / 2 + m_szIconSize.cy;

		DrawImage(hDC, m_Icon);
	}
}

SIZE CMenuElementUI::EstimateSize(SIZE szAvailable)
{
	SIZE cXY = {0};
	for( int it = 0; it < GetCount(); it++ ) {
		CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
		if( !pControl->IsVisible() ) continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		cXY.cy += sz.cy;
		if( cXY.cx < sz.cx )
			cXY.cx = sz.cx;
	}
	if(cXY.cy == 0) {
		TListInfoUI* pInfo = m_pOwner->GetListInfo();

		DWORD iTextColor = pInfo->dwTextColor;
		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			iTextColor = pInfo->dwHotTextColor;
		}
		if( IsSelected() ) {
			iTextColor = pInfo->dwSelectedTextColor;
		}
		if( !IsEnabled() ) {
			iTextColor = pInfo->dwDisabledTextColor;
		}
		CDuiString sText = GetText();
		RECT rcTextPadding = GetScaleRect(pInfo->rcTextPadding, m_nScale);
		RECT rcText = { 0, 0, MAX(szAvailable.cx, GetScaleInt(m_cxyFixed.cx, m_nScale)), 9999 };
		rcText.left += rcTextPadding.left;
		rcText.right -= rcTextPadding.right;
		if( pInfo->bShowHtml ) {   
			int nLinks = 0;
			CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), m_pManager, rcText, sText, iTextColor, NULL, NULL, nLinks, pInfo->nFont, DT_CALCRECT | pInfo->uTextStyle & ~DT_RIGHT & ~DT_CENTER);
		}
		else {
			CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, sText, iTextColor, pInfo->nFont, DT_CALCRECT | pInfo->uTextStyle & ~DT_RIGHT & ~DT_CENTER);
		}
		cXY.cx = rcText.right - rcText.left + rcTextPadding.left + rcTextPadding.right + 20;
		cXY.cy = rcText.bottom - rcText.top + rcTextPadding.top + rcTextPadding.bottom;
	}

	if( m_cxyFixed.cy != 0 ) cXY.cy = GetScaleInt(m_cxyFixed.cy, m_nScale);
	return cXY;
}

void CMenuElementUI::DoEvent(TEventUI& event)
{
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		CListContainerElementUI::DoEvent(event);
		if( m_pWindow ) return;
		bool hasSubMenu = false;
		for( int i = 0; i < GetCount(); ++i )
		{
			if( GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT) != NULL )
			{
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT)))->SetVisible(true);
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT)))->SetInternVisible(true);

				hasSubMenu = true;
			}
		}
		if( hasSubMenu )
		{
			m_pOwner->SelectItem(GetIndex(), true);
			CreateMenuWnd();
		}
		else
		{
			ContextMenuParam param;
			param.hWnd = m_pManager->GetPaintWindow();
			param.wParam = 2;
			s_context_menu_observer.RBroadcast(param);
			m_pOwner->SelectItem(GetIndex(), true);
		}
	}

	if( event.Type == UIEVENT_BUTTONUP ) //UIEVENT_BUTTONDOWN
	{
		if( IsEnabled() ){
			CListContainerElementUI::DoEvent(event);

			if( m_pWindow ) return;

			bool hasSubMenu = false;
			for( int i = 0; i < GetCount(); ++i ) {
				if( GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT) != NULL ) {
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT)))->SetVisible(true);
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT)))->SetInternVisible(true);

					hasSubMenu = true;
				}
			}
			if( hasSubMenu )
			{
				CreateMenuWnd();
			}
			else
			{
				PostMenuMessage(GetID());
				ContextMenuParam param;
				param.hWnd = m_pManager->GetPaintWindow();
				param.wParam = 1;
				s_context_menu_observer.RBroadcast(param);
			}
        }
        return;
    }

	if (event.Type == UIEVENT_KEYDOWN && event.chKey == VK_RIGHT)
	{
		if (m_pWindow) return;
		bool hasSubMenu = false;
		for (int i = 0; i < GetCount(); ++i)
		{
			if (GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL)
			{
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetVisible(true);
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(true);

				hasSubMenu = true;
			}
		}
		if (hasSubMenu)
		{
			m_pOwner->SelectItem(GetIndex(), true);
			CreateMenuWnd();
		}
		else
		{
			ContextMenuParam param;
			param.hWnd = m_pManager->GetPaintWindow();
			param.wParam = 2;
			s_context_menu_observer.RBroadcast(param);
			m_pOwner->SelectItem(GetIndex(), true);
		}

		return;
	}

    CListContainerElementUI::DoEvent(event);
}

void CMenuElementUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("icon")) == 0) {
		SetIcon(pstrValue);
	}
	else if (_tcscmp(pstrName, _T("iconsize")) == 0) {
		LPTSTR pstr = NULL;
		LONG cx = 0, cy = 0;
		cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
		cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
		SetIconSize(cx, cy);
	}
	else if (_tcscmp(pstrName, _T("type")) == 0) {
		if (_tcscmp(pstrValue, _T("separator")) == 0)
			SetTypeSeparator(true);
	}
	else if (_tcscmp(pstrName, _T("height")) == 0) {
		SetFixedHeight(_ttoi(pstrValue));
	}
	else
		CListContainerElementUI::SetAttribute(pstrName, pstrValue);
}

bool CMenuElementUI::Activate()
{
	if (CListContainerElementUI::Activate() && m_bSelected)
	{
		if( m_pWindow ) return true;
		bool hasSubMenu = false;
		for (int i = 0; i < GetCount(); ++i)
		{
			if (GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT) != NULL)
			{
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT)))->SetVisible(true);
				(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(DUI_CTR_MENUELEMENT)))->SetInternVisible(true);

				hasSubMenu = true;
			}
		}
		if (hasSubMenu)
		{
			CreateMenuWnd();
		}
		else
		{
			ContextMenuParam param;
			param.hWnd = m_pManager->GetPaintWindow();
			param.wParam = 1;
			s_context_menu_observer.RBroadcast(param);
		}

		return true;
	}
	return false;
}

CMenuWnd* CMenuElementUI::GetMenuWnd()
{
	return m_pWindow;
}

void CMenuElementUI::CreateMenuWnd()
{
	if( m_pWindow ) return;

	m_pWindow = new CMenuWnd();
	ASSERT(m_pWindow);

	ContextMenuParam param;
	param.hWnd = m_pManager->GetPaintWindow();
	param.wParam = 2;
	s_context_menu_observer.RBroadcast(param);

	m_pWindow->LoadMenu(static_cast<CMenuElementUI*>(this), _T(""), _T(""), NULL);
}


void CMenuElementUI::PostMenuMessage(UINT id) const
{
	CPaintManagerUI* pNotifyPm = s_context_menu_observer.GetNotifyPaintManager();
	if (pNotifyPm != NULL)
	{
		pNotifyPm->SendNotify(id, GetName(), GetVirtualWnd(), DUI_EVT_MENU_CLICK);
	}
}

} // namespace DuiLib
