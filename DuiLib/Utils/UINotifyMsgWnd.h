#ifndef UI_NOTIFY_MSG_WND_HPP
#define UI_NOTIFY_MSG_WND_HPP

class CNotifyMsgWnd : public CWindowWnd, public INotifyUI
{
public:
	CNotifyMsgWnd() : m_xml(_T("")) { };
	
    LPCTSTR GetWindowClassName() const { return _T("CNotifyMsgWnd"); }
    void OnFinalMessage(HWND /*hWnd*/) { m_pm.RemoveNotifier(this); delete this; }
	void SetMessage(LPCTSTR msg) { m_Message = msg; }
    HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT, HMENU hMenu = NULL)
    {
        dwExStyle |= WS_EX_TOOLWINDOW;
        return CWindowWnd::Create(hwndParent, pstrName, dwStyle, dwExStyle, x, y, cx, cy, hMenu);
    }

    void ShowWindow(bool bShow = true, bool bTakeFocus = true)
    {
        CWindowWnd::ShowWindow(bShow, bTakeFocus);
    }
	void LoadSkinXML(STRINGorID xml, LPCTSTR pSkinType);

protected:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	void OnTimer(TNotifyUI& msg);

    void AdjustWindowByCurrentDPI();
    void AdjustPostion();
    void Notify(TNotifyUI& msg);
    
	CDuiString m_Message;
    CPaintManagerUI m_pm;
    STRINGorID m_xml;
    CDuiString m_sType;
	int m_Process;
	int m_nMsElapse;
};

void ShowNotifyMsgWnd(LPCTSTR msg);

#endif //UI_NOTIFY_MSG_WND_HPP
