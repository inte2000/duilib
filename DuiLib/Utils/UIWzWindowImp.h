#ifndef UI_WZ_WINDOW_HPP
#define UI_WZ_WINDOW_HPP


namespace DuiLib {

    DUILIB_API extern const TCHAR* DuiFrmLayoutName;
    DUILIB_API extern const TCHAR* DuiCaptionLayoutName;
    DUILIB_API extern const TCHAR* DuiFrmTitleName;

    class DUILIB_API CWzWindowImpl : public WindowImplBase
    {
    public:
        CWzWindowImpl() {
            m_dwBorderColor = 0;
            m_dwFocusBorderColor = 0;
        }
        virtual ~CWzWindowImpl() {};
        virtual void Notify(TNotifyUI& msg);
        UINT GetClassStyle() const { return CS_DBLCLKS; };

        void SetWindowTitle(LPCTSTR lpstrTitle);

        DUI_EVENT_MAP_BEGIN()
            ON_EVENT_HANDLER(DUI_EVT_MENU_CLICK, OnMenuClick)
        DUI_EVENT_MAP_END()

    protected:
        virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        virtual LRESULT OnNcActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
        virtual LRESULT OnNcRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        BOOL OnMenuClick(TNotifyUI& msg);
        void UpdateSystemMenu(CMenuWnd* pMenu);
        bool IsHasSizeButtons();
        void SysCommandClose();
        void SysCommandMaximize();
        void SysCommandMinimize();
        void SysCommandRestore();
        void SysCommandMove();
        void SysCommandSize();
    private:
        DWORD m_dwBorderColor;
        DWORD m_dwFocusBorderColor;
    };

}
#endif // UI_WZ_WINDOW_HPP
