#ifndef WIN_IMPL_BASE_HPP
#define WIN_IMPL_BASE_HPP

namespace DuiLib
{
	DUILIB_API extern const TCHAR* DuiWndCloseButtonName;
	DUILIB_API extern const TCHAR* DuiWndMinButtonName;
	DUILIB_API extern const TCHAR* DuiWndMaxButtonName;
	DUILIB_API extern const TCHAR* DuiWndRestoreButtonName;

	class DUILIB_API WindowImplBase
		: public CWindowWnd
		, public CNotifyPump
		, public INotifyUI
		, public IMessageFilterUI
		, public IDialogBuilderCallback
	{
	public:
		WindowImplBase(){};
		virtual ~WindowImplBase(){};
		virtual void OnFinalMessage( HWND hWnd );
		virtual CControlUI* CreateControl(LPCTSTR pstrClass);

		DUI_EVENT_MAP_BEGIN()
			ON_EVENT_HANDLER(DUI_MSGTYPE_CLICK, OnClick)
		DUI_EVENT_MAP_END()

		
		virtual void Notify(TNotifyUI& msg);

	protected:
		virtual CDuiString GetSkinFile() = 0;
		virtual LPCTSTR GetWindowClassName(void) const = 0 ;
		virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
		virtual void OnSysCloseButtonClick();

		CPaintManagerUI m_PaintManager;

		BOOL OnClick(TNotifyUI& msg);
	public:
		virtual UINT GetClassStyle() const;
		virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/);
		virtual LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

		virtual LRESULT OnNcActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		virtual LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnNcRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
        virtual LRESULT OnDPIChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LONG GetStyle();
	};
}

#endif // WIN_IMPL_BASE_HPP
