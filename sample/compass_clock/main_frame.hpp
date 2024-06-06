#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP


class MainFrame : public CWzWindowImpl
{
public:
	MainFrame();
	virtual ~MainFrame();

    LPCTSTR GetWindowClassName() const { return _T("CDuiTestMainFrameWnd"); };
    virtual CDuiString GetSkinFile() { return _T("testdui_mainfrm.xml"); };
    virtual void OnFinalMessage(HWND hWnd);
    CControlUI* CreateControl(LPCTSTR pstrClass) override;
public:
     
    DUI_EVENT_MAP_BEGIN()
        ON_WINDOWINIT_HANDLER(OnWindowInit)
    DUI_EVENT_MAP_END()

protected:	
    void OnWindowInit();

    virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

};

#endif // MAINFRAME_HPP
