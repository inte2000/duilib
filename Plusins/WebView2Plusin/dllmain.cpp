// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "webview2_plusin.h"
#include "UIWebView2.h"

DuiLib::CControlUI* CreateControl(LPCTSTR pstrType)
{
    if (!pstrType) return NULL;

    if (_tcsicmp(pstrType, _T("WebView2")) == 0) {
        return new DuiLib::CWebView2UI();
    }
    return NULL;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

