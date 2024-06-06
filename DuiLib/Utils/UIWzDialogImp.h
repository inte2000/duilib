#ifndef UI_WZ_DIALOG_HPP
#define UI_WZ_DIALOG_HPP


namespace DuiLib {

    //DUILIB_API extern const TCHAR* DuiDialogOkButtonName;
    #define DuiDialogOkButtonName  _T("btn_ok")
    #define DuiDialogCancelButtonName  _T("btn_cancel")

    class DUILIB_API CWzDialogImpl : public CWzWindowImpl
    {
    public:
        CWzDialogImpl() { }
        virtual ~CWzDialogImpl() {};

        UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
        UINT_PTR DoModal(HWND hWnd);
        
        DUI_EVENT_MAP_BEGIN()
            ON_CLICK_NAME_HANDLER(DuiDialogOkButtonName, OnOkButtonClick)
            ON_CLICK_NAME_HANDLER(DuiDialogCancelButtonName, OnCancelButtonClick)
        DUI_EVENT_MAP_END()

    protected:
        virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);

        virtual void OnOkButtonClick();
        virtual void OnCancelButtonClick();
    };

}
#endif // UI_WZ_DIALOG_HPP
