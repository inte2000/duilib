// compass_clock.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "compass_clock.h"
#include "ComEnv.h"
#include "main_frame.hpp"

#include <atlbase.h>
CComModule _Module;
#include <atlwin.h>


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	ComEnv com_init; 
	CGdiPlusEnv gdiplus_init;

	CPaintManagerUI::SetInstance(hInstance);
#ifdef _DEBUG
	CPaintManagerUI::SetResourceType(UILIB_FILE);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin\\compass_clock"));
#else
	CPaintManagerUI::SetResourceType(UILIB_ZIP);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin"));
	CPaintManagerUI::SetResourceZip(_T("compass_clock.zip"), true, nullptr); 
#endif

	_Module.Init(0, hInstance);

	MainFrame* pFrame = new MainFrame();
	if (pFrame == NULL) return 0;
	pFrame->Create(NULL, _T(""), UI_WNDSTYLE_FRAME, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 600, 480);
	pFrame->CenterWindow();
	::ShowWindow(*pFrame, SW_SHOW);

	CPaintManagerUI::MessageLoop();
	CPaintManagerUI::Term();

	_Module.Term();

    return 0;
}

