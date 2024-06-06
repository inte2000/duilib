#include "stdafx.h"
#include "UIWzWindowImp.h"

namespace DuiLib
{

    const TCHAR* DuiFrmLayoutName = _T("wnd_frame");
    const TCHAR* DuiCaptionLayoutName = _T("wnd_caption");
    const TCHAR* DuiFrmTitleName = _T("frmtitle");

    void CWzWindowImpl::SetWindowTitle(LPCTSTR lpstrTitle)
    {
        CLabelUI* pFrmTitle = static_cast<CLabelUI*>(m_PaintManager.FindControl(DuiFrmTitleName));
        if (pFrmTitle)
            pFrmTitle->SetText(lpstrTitle);

        ::SetWindowText(m_hWnd, lpstrTitle);
    }

    LRESULT CWzWindowImpl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LRESULT lr = WindowImplBase::OnCreate(uMsg, wParam, lParam, bHandled);
        if (lr == 0)
        {
            CContainerUI* frameUI = static_cast<CContainerUI *>(m_PaintManager.FindControl(DuiFrmLayoutName));
            if (frameUI != NULL)
            {
                m_dwBorderColor = frameUI->GetBorderColor();
                m_dwFocusBorderColor = frameUI->GetFocusBorderColor();
                if (m_dwFocusBorderColor != 0)
                {
                    frameUI->SetBorderColor(m_dwFocusBorderColor);

                    CControlUI* pCaption = frameUI->FindSubControl(DuiCaptionLayoutName);
                    if (pCaption != NULL)
                    {
                        DWORD bkColor = m_dwFocusBorderColor & 0x00FFFFFF; 
                        bkColor |= 0xAA000000; //激活时设置标题栏用 85 透明度的背景色
                        pCaption->SetBkColor(bkColor);
                    }
                }
            } 

            //将资源文件中指定的窗口标题设置给实际的窗口句柄（GetText() 得到经过翻译的问题）
            CLabelUI* pFrmTitle = static_cast<CLabelUI*>(m_PaintManager.FindControl(DuiFrmTitleName));
            if (pFrmTitle)
                ::SetWindowText(m_hWnd, pFrmTitle->GetText());
        }

        bHandled = FALSE;
        return lr;
    }

    LRESULT CWzWindowImpl::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LRESULT lr = WindowImplBase::OnNcActivate(uMsg, wParam, lParam, bHandled);

        if (m_PaintManager.GetRoot() != NULL)
        {
            bool bActive = (wParam != 0) ? true : false;
            CContainerUI* frameUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(DuiFrmLayoutName));
            if (frameUI != NULL)
            {
                DWORD actColor = bActive ? m_dwFocusBorderColor : m_dwBorderColor;
                if (actColor != 0)
                {
                    frameUI->SetBorderColor(actColor);

                    CControlUI* pCaption = frameUI->FindSubControl(DuiCaptionLayoutName);
                    if (pCaption != NULL)
                    {
                        DWORD bkColor = actColor & 0x00FFFFFF;
                        if(bActive) 
                            bkColor |= 0xAA000000; //激活时设置标题栏用 85 透明度的背景色
                        pCaption->SetBkColor(bkColor);
                    }
                }
            }
        }

        bHandled = FALSE;
        //DUITRACE(_T("OnNcActivate(%x) = %d"), m_hWnd, wParam);
        return lr;
    }

    LRESULT CWzWindowImpl::OnNcRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LRESULT lr = WindowImplBase::OnNcRButtonDown(uMsg, wParam, lParam, bHandled);

        POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        //::ScreenToClient(m_hWnd, &point);

        CDuiString sysmenuname = m_PaintManager.GetWindowCustomAttribute(_T("sys_menu"));
        if (!sysmenuname.IsEmpty())
        {
#if 0
            CSingleMenuWnd* pMenu = new CSingleMenuWnd();
            ::SetForegroundWindow(m_hWnd);//it is important that ...
            pMenu->LoadMenu(&m_PaintManager, STRINGorID(sysmenuname), 0);
            UpdateSystemMenu(pMenu);
            pMenu->TrackMenu(point, ptLeftTop);
            ::PostMessage(m_hWnd, NULL, 0, 0);
#endif
            CMenuWnd* pMenu = new CMenuWnd();
            pMenu->LoadMenu(NULL, STRINGorID(sysmenuname), _T("xml"), &m_PaintManager);
            UpdateSystemMenu(pMenu);
            pMenu->TrackMenu(point, eMenuAlignment_Left | eMenuAlignment_Top);
            ::PostMessage(m_hWnd, NULL, 0, 0);
        }

        //bHandled = FALSE;
        return lr;
    }

    void CWzWindowImpl::UpdateSystemMenu(CMenuWnd* pMenu)
    {
        bool bHasSizeButtons = IsHasSizeButtons();

        if (!bHasSizeButtons)
        {
            pMenu->DeleteMenuItem(_T("menu_wnd_restore"));
            pMenu->DeleteMenuItem(_T("menu_wnd_size"));
            pMenu->DeleteMenuItem(_T("menu_wnd_maximize"));
            pMenu->DeleteMenuItem(_T("menu_wnd_minimize"));
        }

        if (::IsIconic(m_hWnd))
        {
            if (bHasSizeButtons)
            {
                pMenu->SetMenuItemEnable(_T("menu_wnd_restore"), true);
            }
        }
        else
        {
            if (::IsZoomed(m_hWnd))
            {
                if (bHasSizeButtons)
                {
                    pMenu->SetMenuItemEnable(_T("menu_wnd_restore"), true);
                }
            }
            else
            {
                if (bHasSizeButtons)
                {
                    pMenu->SetMenuItemEnable(_T("menu_wnd_restore"), false);
                }
            }
        }

        if (::IsZoomed(m_hWnd))
        {
            pMenu->SetMenuItemEnable(_T("menu_wnd_move"), false);
            if (bHasSizeButtons)
            {
                pMenu->SetMenuItemEnable(_T("menu_wnd_size"), false);
                pMenu->SetMenuItemEnable(_T("menu_wnd_maximize"), false);
            }
        }
        else
        {
            pMenu->SetMenuItemEnable(_T("menu_wnd_move"), true);
            if (bHasSizeButtons)
            {
                pMenu->SetMenuItemEnable(_T("menu_wnd_size"), true);
                pMenu->SetMenuItemEnable(_T("menu_wnd_maximize"), true);
            }
        }
    }

    BOOL CWzWindowImpl::OnMenuClick(TNotifyUI& msg)
    {
        switch (msg.uId)
        {
        case 51001:
            SysCommandRestore();
            break;
        case 51002:
            SysCommandMove();
            break;
        case 51003:
            SysCommandSize();
            break;
        case 51004:
            SysCommandMinimize();
            break;
        case 51005:
            SysCommandMaximize();
            break;
        case 51006:
            SysCommandClose();
            break;
        default:
            break;
        }

        return FALSE;
    }

    void CWzWindowImpl::Notify(TNotifyUI& msg)
    {
        WindowImplBase::Notify(msg);
    }

    bool CWzWindowImpl::IsHasSizeButtons()
    {
        CControlUI* pMinBtn = static_cast<CControlUI*>(m_PaintManager.FindControl(DuiLib::DuiWndMinButtonName));
        if (pMinBtn == NULL)
            return false;

        CControlUI* pMaxBtn = static_cast<CControlUI*>(m_PaintManager.FindControl(DuiLib::DuiWndMaxButtonName));
        if (pMaxBtn == NULL)
            return false;

        return true;
    }

    void CWzWindowImpl::SysCommandClose()
    {
        OnSysCloseButtonClick();
        //Close(IDCANCEL);
    }

    void CWzWindowImpl::SysCommandMaximize()
    {
#if defined(UNDER_CE)
        ::ShowWindow(m_hWnd, SW_MAXIMIZE);
        CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(DuiWndMaxButtonName));
        if (pControl) pControl->SetVisible(false);
        pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(DuiWndRestoreButtonName));
        if (pControl) pControl->SetVisible(true);
#else
        PostMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
#endif
    }

    void CWzWindowImpl::SysCommandMinimize()
    {
#if defined(UNDER_CE)
        ::ShowWindow(m_hWnd, SW_MINIMIZE);
#else
        PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
#endif
    }

    void CWzWindowImpl::SysCommandRestore()
    {
#if defined(UNDER_CE)
        ::ShowWindow(m_hWnd, SW_RESTORE);
        CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(DuiWndMaxButtonName));
        if (pControl) pControl->SetVisible(true);
        pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(DuiWndRestoreButtonName));
        if (pControl) pControl->SetVisible(false);
#else
        PostMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
#endif
    }

    void CWzWindowImpl::SysCommandMove()
    {
        PostMessage(WM_SYSCOMMAND, SC_MOVE, 0);
    }

    void CWzWindowImpl::SysCommandSize()
    {
        PostMessage(WM_SYSCOMMAND, SC_SIZE, 0);
    }

}