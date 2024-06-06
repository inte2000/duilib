#ifndef UI_PROMPT_INPUT_DLG_HPP
#define UI_PROMPT_INPUT_DLG_HPP

#include "UiWzDialogImp.h"

namespace DuiLib {

#define PROMPT_STYLE_NUMBER       0x00000001
#define PROMPT_STYLE_PASSWORD     0x00000002

    class DUILIB_API CPromptInputDlg : public CWzDialogImpl
    {
    public:
        CPromptInputDlg() { m_inputStyle = 0; };
        virtual ~CPromptInputDlg() {};

        LPCTSTR GetWindowClassName() const { return _T("UIPromptInputDlg"); };
        virtual CDuiString GetSkinFile() { return _T("prompt_input_dlg.xml"); };

    public:
        void SetPromptInformation(LPCTSTR lpPromptText, LPCTSTR lpCaption, UINT style);
        CDuiString GetInputText() const { return m_strInput; }
        void SetInputText(LPCTSTR lpText) { m_strInput = lpText; }

        DUI_EVENT_MAP_BEGIN()
        DUI_EVENT_MAP_END()

    protected:
        CDuiString m_strCaption;
        CDuiString m_strPrompt;
        CDuiString m_strInput;
        UINT m_inputStyle;

        virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

        void OnOkButtonClick() override;
        void OnCancelButtonClick() override;

    protected:
    };

    DUILIB_API BOOL UIGetPromptInput(HWND hWnd, LPCTSTR lpPromptText, LPCTSTR lpCaption, UINT style, CDuiString& strInput);

}

#endif // UI_PROMPT_INPUT_DLG_HPP
