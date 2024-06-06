#include "stdafx.h"
#include "UIMessageBox.h"
#include "UICommonSkin.h"

namespace DuiLib {


    const TCHAR* const kMessageTextControlName = _T("msgText");
    /*
    const TCHAR* const kOKButtonName = _T("btn_ok");
    const TCHAR* const kCancelButtonName = _T("btn_cancel");
    const TCHAR* const kYesButtonName = _T("btn_yes");
    const TCHAR* const kNoButtonName = _T("btn_no");
    const TCHAR* const kRetryButtonName = _T("btn_retry");
    const TCHAR* const kAbortButtonName = _T("btn_abort");
    const TCHAR* const kIgnoreButtonName = _T("btn_ignore");
    const TCHAR* const kYesAllButtonName = _T("btn_yes_all");
    const TCHAR* const kNoAllButtonName = _T("btn_no_all");
    */

    typedef struct tagMB_BUTTON_DEF
    {
        UINT id;
        const TCHAR* btnName;
        const TCHAR* btnText;
    }MB_BUTTON_DEF;

    typedef struct tagMB_ICON_DEF
    {
        UINT id;
        int iconIdx;
    }MB_ICON_DEF;


    static MB_BUTTON_DEF g_btnDef[] =
    {
        { PRV_BUTTON_OK,     kOKButtonName,      _T("%{OK_Button}%") },
        { PRV_BUTTON_YES,    kYesButtonName,     _T("%{Yes_Button}%") },
        { PRV_BUTTON_YESALL, kYesAllButtonName,  _T("%{AllYes_Button}%") },
        { PRV_BUTTON_NO,     kNoButtonName,      _T("%{No_Button}%") },
        { PRV_BUTTON_NOALL, kNoAllButtonName,  _T("%{AllNo_Button}%") },
        { PRV_BUTTON_RETRY,  kRetryButtonName,   _T("%{Retry}%") },
        { PRV_BUTTON_IGNORE, kIgnoreButtonName,  _T("%{Ignore}%") },
        { PRV_BUTTON_ABORT,  kAbortButtonName,   _T("%{Abandon}%") },
        { PRV_BUTTON_CANCEL, kCancelButtonName,  _T("%{Cancel_Button}%") }
    };

    static MB_ICON_DEF g_iconDef[] =
    {
        { UMB_ICONEXCLAMATION,    0 },
        { UMB_ICONWARNING,        1 },
        { UMB_ICONINFORMATION,    2 },
        { UMB_ICONASTERISK,       3 },
        { UMB_ICONQUESTION,       4 },
        { UMB_ICONSTOP,           5 },
        { UMB_ICONERROR,          6 },
        { UMB_ICONHAND,           7 }
    };

    LRESULT CMessageBox::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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

            CRichEditUI* pMsgText = static_cast<CRichEditUI*>(m_PaintManager.FindControl(kMessageTextControlName));
            if (pMsgText)
                pMsgText->SetText(m_strText);

            PrepareIconShow();
            PrepareButtonShow();
        }

        return lr;
    }

    LRESULT CMessageBox::ResponseDefaultKeyEvent(WPARAM wParam)
    {
        if (wParam == VK_RETURN)
        {
            if (m_Style & PRV_BUTTON_OK)
                Close(IDOK);
            else if ((m_Style & PRV_BUTTON_YES) || (m_Style & PRV_BUTTON_YESALL))
                Close(IDYES);
            else
                return FALSE;

            return TRUE;
        }
        else if (wParam == VK_ESCAPE)
        {
            Close(IDCANCEL);
            return TRUE;
        }

        return FALSE;
    }

    int CMessageBox::GetIconIndex(UINT style)
    {
        for (int i = 0; i < sizeof(g_iconDef) / sizeof(g_iconDef[0]); i++)
        {
            if (g_iconDef[i].id & style)
            {
                return g_iconDef[i].iconIdx;
            }
        }

        return -1;
    }

    void CMessageBox::PrepareIconShow()
    {
        int iconidx = GetIconIndex(m_Style);
        if (iconidx == -1)
        {
            CVerticalLayoutUI* pVl = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("leftVL")));
            if (pVl)
                pVl->SetVisible(false);
        }
        else
        {
            CLabelUI* pLeftIcon = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lefticon")));
            if (pLeftIcon)
            {
                CDuiString bkImgString;
                int leftpos = iconidx * 64;
                int rightpos = leftpos + 64;
                bkImgString.Format(_T("file=\'messagebox_icons.png\' source=\'%d,0,%d,64\'"), leftpos, rightpos);
                pLeftIcon->SetBkImage(bkImgString);
            }
        }
    }

    void CMessageBox::EnableButtonLayoutUI(CVerticalLayoutUI* btnVlUI, LPCTSTR btnName, LPCTSTR btnText)
    {
        btnVlUI->SetVisible(true);
        CControlUI* pControl = btnVlUI->FindSubControl(_T("btn_unknown"));
        if (pControl)
        {
            pControl->SetName(btnName);
            pControl->SetText(btnText);
        }
    }

    void CMessageBox::PrepareButtonShow()
    {
        int btnVlIdx = 0;
        CDuiString btnVlName;

        for (int i = 0; i < sizeof(g_btnDef) / sizeof(g_btnDef[0]); i++)
        {
            if (g_btnDef[i].id & m_Style)
            {
                btnVlName.Format(_T("btn_%d_pos"), btnVlIdx + 1);
                CVerticalLayoutUI* btnVlUI = dynamic_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(btnVlName));
                if (btnVlUI != NULL)
                {
                    EnableButtonLayoutUI(btnVlUI, g_btnDef[i].btnName, g_btnDef[i].btnText);
                }
                btnVlIdx++;
            }
        }
    }

    void CMessageBox::SetInformation(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
    {
        m_strCaption = lpCaption;
        m_strText = lpText;
        m_Style = uType;
    }

    void CMessageBox::OnOkButtonClick()
    {
        CWzDialogImpl::OnOkButtonClick();
    }

    void CMessageBox::OnCancelButtonClick()
    {
        CWzDialogImpl::OnCancelButtonClick();
    }

    void CMessageBox::OnYesButtonClick()
    {
        Close(IDYES);
    }

    void CMessageBox::OnNoButtonClick()
    {
        Close(IDNO);
    }

    void CMessageBox::OnAbortButtonClick()
    {
        Close(IDABORT);
    }

    void CMessageBox::OnRetryButtonClick()
    {
        Close(IDRETRY);
    }

    void CMessageBox::OnIgnoreButtonClick()
    {
        Close(IDIGNORE);
    }

    void CMessageBox::OnYesAllButtonClick()
    {
        Close(IDYESALL);
    }

    void CMessageBox::OnNoAllButtonClick()
    {
        Close(IDNOALL);
    }

    int UIMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
    {
        CMessageBox msgBox;

        msgBox.SetInformation(lpText, lpCaption, uType);
        return (int)msgBox.DoModal(hWnd);
    }

}