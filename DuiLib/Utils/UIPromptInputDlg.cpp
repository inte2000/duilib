#include "stdafx.h"
#include "UIPromptInputDlg.h"
#include "UICommonSkin.h"

namespace DuiLib
{

    LPCTSTR kPromptLabelName = _T("prompt_text");
    LPCTSTR kPromptEditName = _T("input_text");


    LRESULT CPromptInputDlg::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LRESULT lr = CWzDialogImpl::OnCreate(uMsg, wParam, lParam, bHandled);
        if (lr == 0)
        {
            bHandled = TRUE;
            SetWindowTitle(m_strCaption);

            CDuiString skImg = GetCommonBkImage();
            DWORD bkColor = GetCommonBkColor();
            if (!skImg.IsEmpty() || (bkColor != 0))
            {
                m_PaintManager.SetPaintWindowBackground(skImg, bkColor, GetCommonOpacity());
            }

            CLabelUI* pPrompt = static_cast<CLabelUI*>(m_PaintManager.FindControl(kPromptLabelName));
            if (pPrompt)
                pPrompt->SetText(m_strPrompt);

            CEditUI* pInput = static_cast<CEditUI*>(m_PaintManager.FindControl(kPromptEditName));
            if (pInput)
            {
                if ((m_inputStyle & PROMPT_STYLE_NUMBER) == PROMPT_STYLE_NUMBER)
                {
                    pInput->SetNumberOnly(true);
                }
                if ((m_inputStyle & PROMPT_STYLE_PASSWORD) == PROMPT_STYLE_PASSWORD)
                {
                    pInput->SetPasswordChar(_T('#'));
                    pInput->SetPasswordMode(true);
                }
                if (!m_strInput.IsEmpty())
                {
                    pInput->SetText(m_strInput);
                }
            }
        }

        return lr;
    }

    void CPromptInputDlg::SetPromptInformation(LPCTSTR lpPromptText, LPCTSTR lpCaption, UINT style)
    {
        m_strCaption = lpCaption;
        m_strPrompt = lpPromptText;
        m_inputStyle = style;
    }

    void CPromptInputDlg::OnOkButtonClick()
    {
        CEditUI* pInput = static_cast<CEditUI*>(m_PaintManager.FindControl(kPromptEditName));
        if (pInput)
        {
            m_strInput = pInput->GetText();
        }

        CWzDialogImpl::OnOkButtonClick();
    }

    void CPromptInputDlg::OnCancelButtonClick()
    {
        CWzDialogImpl::OnCancelButtonClick();
    }

    BOOL UIGetPromptInput(HWND hWnd, LPCTSTR lpPromptText, LPCTSTR lpCaption, UINT style, CDuiString& strInput)
    {
        CPromptInputDlg inputDlg;

        inputDlg.SetPromptInformation(lpPromptText, lpCaption, style);
        inputDlg.SetInputText(strInput);
        if (inputDlg.DoModal(hWnd) == IDOK)
        {
            strInput = inputDlg.GetInputText();
            return TRUE;
        }

        return FALSE;
    }

}