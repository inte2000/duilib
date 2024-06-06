#ifndef UI_MESSAGEBOX_HPP
#define UI_MESSAGEBOX_HPP

#include "UiWzDialogImp.h"

namespace DuiLib {


#define PRV_BUTTON_OK           0x00000001
#define PRV_BUTTON_CANCEL       0x00000002
#define PRV_BUTTON_YES          0x00000004
#define PRV_BUTTON_NO           0x00000008
#define PRV_BUTTON_RETRY        0x00000010
#define PRV_BUTTON_ABORT        0x00000020
#define PRV_BUTTON_IGNORE       0x00000040
#define PRV_BUTTON_YESALL       0x00000080
#define PRV_BUTTON_NOALL        0x00000100

#define UMB_ABORTRETRYIGNORE         (PRV_BUTTON_ABORT|PRV_BUTTON_RETRY|PRV_BUTTON_IGNORE)
#define UMB_OK                       (PRV_BUTTON_OK)
#define UMB_OKCANCEL                 (PRV_BUTTON_OK|PRV_BUTTON_CANCEL)
#define UMB_RETRYCANCEL              (PRV_BUTTON_RETRY|PRV_BUTTON_CANCEL)
#define UMB_YESNO                    (PRV_BUTTON_YES|PRV_BUTTON_NO)
#define UMB_YESNOCANCEL              (PRV_BUTTON_YES|PRV_BUTTON_NO|PRV_BUTTON_CANCEL)
#define UMB_YESNOCANCEL              (PRV_BUTTON_YES|PRV_BUTTON_NO|PRV_BUTTON_CANCEL)
#define UMB_YESCONFIRM               (PRV_BUTTON_YES|PRV_BUTTON_YESALL|PRV_BUTTON_CANCEL)
#define UMB_NOCONFIRM                (PRV_BUTTON_NO|PRV_BUTTON_NOALL|PRV_BUTTON_CANCEL)
#define UMB_YESNOCONFIRM             (PRV_BUTTON_YESALL|PRV_BUTTON_NOALL|PRV_BUTTON_CANCEL)

#define UMB_ICONEXCLAMATION       0x010000
#define UMB_ICONWARNING           0x020000
#define UMB_ICONINFORMATION       0x040000
#define UMB_ICONASTERISK          0x080000
#define UMB_ICONQUESTION          0x100000
#define UMB_ICONSTOP              0x200000
#define UMB_ICONERROR             0x400000
#define UMB_ICONHAND              0x800000

#define IDYESALL            20
#define IDNOALL             21
    /*
    DUILIB_API extern const TCHAR* const kOKButtonName;
    DUILIB_API extern const TCHAR* const kCancelButtonName;
    DUILIB_API extern const TCHAR* const kYesButtonName;
    DUILIB_API extern const TCHAR* const kNoButtonName;
    DUILIB_API extern const TCHAR* const kRetryButtonName;
    DUILIB_API extern const TCHAR* const kAbortButtonName;
    DUILIB_API extern const TCHAR* const kIgnoreButtonName;
    DUILIB_API extern const TCHAR* const kYesAllButtonName;
    DUILIB_API extern const TCHAR* const kNoAllButtonName;
    */
    #define kOKButtonName              _T("btn_ok")
    #define kCancelButtonName          _T("btn_cancel")
    #define kYesButtonName             _T("btn_yes")
    #define kNoButtonName              _T("btn_no")
    #define kRetryButtonName           _T("btn_retry")
    #define kAbortButtonName           _T("btn_abort")
    #define kIgnoreButtonName          _T("btn_ignore")
    #define kYesAllButtonName          _T("btn_yes_all")
    #define kNoAllButtonName           _T("btn_no_all")

    class DUILIB_API CMessageBox : public CWzDialogImpl
    {
    public:
        CMessageBox() {};
        virtual ~CMessageBox() {};

        LPCTSTR GetWindowClassName() const { return _T("UIMessageBox"); };
        virtual CDuiString GetSkinFile() { return _T("messagebox.xml"); };
        void SetInformation(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

        DUI_EVENT_MAP_BEGIN()
            ON_CLICK_NAME_HANDLER(kOKButtonName, OnOkButtonClick)
            ON_CLICK_NAME_HANDLER(kCancelButtonName, OnCancelButtonClick)
            ON_CLICK_NAME_HANDLER(kYesButtonName, OnYesButtonClick)
            ON_CLICK_NAME_HANDLER(kNoButtonName, OnNoButtonClick)
            ON_CLICK_NAME_HANDLER(kRetryButtonName, OnRetryButtonClick)
            ON_CLICK_NAME_HANDLER(kAbortButtonName, OnAbortButtonClick)
            ON_CLICK_NAME_HANDLER(kIgnoreButtonName, OnIgnoreButtonClick)
            ON_CLICK_NAME_HANDLER(kYesAllButtonName, OnYesAllButtonClick)
            ON_CLICK_NAME_HANDLER(kNoAllButtonName, OnNoAllButtonClick)
        DUI_EVENT_MAP_END()

    protected:
        CDuiString m_strCaption;
        CDuiString m_strText;
        UINT m_Style;

        virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);

        void PrepareIconShow();
        void PrepareButtonShow();
        int GetIconIndex(UINT style);
        void EnableButtonLayoutUI(CVerticalLayoutUI* btnVlUI, LPCTSTR btnName, LPCTSTR btnText);

        void OnOkButtonClick() override;
        void OnCancelButtonClick() override;
        void OnYesButtonClick();
        void OnNoButtonClick();
        void OnAbortButtonClick();
        void OnRetryButtonClick();
        void OnIgnoreButtonClick();
        void OnYesAllButtonClick();
        void OnNoAllButtonClick();
    };


    DUILIB_API int UIMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

}

#endif // UI_MESSAGEBOX_HPP
