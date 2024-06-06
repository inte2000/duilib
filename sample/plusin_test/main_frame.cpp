#include "framework.h"
#include <windows.h>
#include <shellapi.h>
#include <time.h>
#include "resource.h"
#include "main_frame.hpp"


 
MainFrame::MainFrame() 
{
}

MainFrame::~MainFrame()
{
}
 
void MainFrame::OnFinalMessage(HWND hWnd)
{
	CWzWindowImpl::OnFinalMessage(hWnd);
	delete this;
}
 
void MainFrame::OnWindowInit() 
{
}

void MainFrame::OnWebViewCreated(TNotifyUI& msg)
{
    CWebView2UI* pWeb = static_cast<CWebView2UI*>(m_PaintManager.FindControl(_T("web_browser")));
    if (pWeb != NULL)
    {
        HRESULT hr = pWeb->Navigate(_T("file://D:\\workspace\\dui_test\\clock\\index.html"));
        if (!SUCCEEDED(hr))
        {
            //œ‘ æ¥ÌŒÛ–≈œ¢
        }
    }
}

 
LRESULT MainFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lr = CWzWindowImpl::OnCreate(uMsg, wParam, lParam, bHandled);
    if (lr == 0)
    {
        bHandled = TRUE;
    }

    return lr;
}

LRESULT MainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    bHandled = FALSE;
    return __super::OnClose(uMsg, wParam, lParam, bHandled);
}
 
LRESULT MainFrame::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ::PostQuitMessage(0L);

    bHandled = FALSE;
    return __super::OnDestroy(uMsg, wParam, lParam, bHandled);
}
