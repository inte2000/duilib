#include "framework.h"
#include <windows.h>
#include <shellapi.h>
#include <time.h>
#include "resource.h"
#include "CWzClockUI.h"
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

CControlUI* MainFrame::CreateControl(LPCTSTR pstrClass)
{
    if (!pstrClass) return NULL;

    if (_tcsicmp(pstrClass, DUI_CTR_WZCLOCK) == 0)
    {
        return new DuiLib::CWzClockUI();
    }

    return NULL;
}

void MainFrame::OnWindowInit() 
{
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
