#include "pch.h"
#include <string>
#include <map>
#include <atlutil.h>
#include <wrl.h>
#include <wrl\implements.h>
#include <WebView2EnvironmentOptions.h>
#include <webview2.h>
#include "UIWebView2.h"

#define  DUI_CTR_WEBVIEW2                        (_T("WebView2"))

using namespace Microsoft::WRL;

namespace DuiLib {

	class CWebView2Impl
	{
	public:
		void AttachHost(CWebView2UI* pHostUI) { m_pHostUI = pHostUI; }
		void SendNotify(LPCTSTR pstrMessage, WPARAM wParam = 0, LPARAM lParam = 0) {
			CPaintManagerUI *pManager = m_pHostUI->GetManager();
			pManager->SendNotify(m_pHostUI, pstrMessage, wParam, lParam);
		}
		void CloseCurrentView() {
			if (!m_webController)
				return;

			m_webController->Close();
		}
		void DoOptionImplement() {
			m_settings->put_IsScriptEnabled(m_scriptEnable ? TRUE : FALSE);
			m_settings->put_AreDefaultScriptDialogsEnabled(m_defaultScriptDialogsEnabled ? TRUE : FALSE);
			m_settings->put_IsWebMessageEnabled(m_isWebMessageEnabled ? TRUE : FALSE);
			m_settings->put_IsBuiltInErrorPageEnabled(m_isBuiltInErrorPageEnabled ? TRUE : FALSE);
			
			put_AllowExternalDrop(m_allowExternalDrop ? TRUE : FALSE);
			put_ContextMenusEnabled(m_contextMenusEnabled ? TRUE : FALSE);
			put_AcceleratorKeysEnabled(m_acceleratorKeysEnabled ? TRUE : FALSE);
			put_StatusBarEnabled(m_statusBarEnabled ? TRUE : FALSE);
			put_ZoomControlEnabled(m_setZoomControlEnabled ? TRUE : FALSE);
		}
		//设置是否启用javascript
		HRESULT SetIsScriptEnabled(bool enable) {
			if (m_scriptEnable == enable)
				return S_OK;
			m_scriptEnable = enable;
			if (!m_settings)
				return E_FAIL;
			return m_settings->put_IsScriptEnabled(enable ? TRUE : FALSE);
		}
		//设置是否允许默认javascript对话框
		HRESULT SetAreDefaultScriptDialogsEnabled(bool enable) {
			if (m_defaultScriptDialogsEnabled == enable)
				return S_OK;
			m_defaultScriptDialogsEnabled = enable;
			if (!m_settings)
				return E_FAIL;
			return m_settings->put_AreDefaultScriptDialogsEnabled(enable ? TRUE : FALSE);
		}
		//设置是否允许Webview的顶层Document与Native通过Message通信
		HRESULT SetIsWebMessageEnabled(bool enable) {
			if (m_isWebMessageEnabled == enable)
				return S_OK;
			m_isWebMessageEnabled = enable;
			if (!m_settings)
				return E_FAIL;
			return m_settings->put_IsWebMessageEnabled(enable ? TRUE : FALSE);
		}
		//设置是否允许默认的错误页
		HRESULT SetIsBuiltInErrorPageEnabled(bool enable) {
			if (m_isBuiltInErrorPageEnabled == enable)
				return S_OK;
			m_isBuiltInErrorPageEnabled = enable;
			if (!m_settings)
				return E_FAIL;
			return m_settings->put_IsBuiltInErrorPageEnabled(enable ? TRUE : FALSE);
		}
		//设置将本地目录设置为虚拟的host
		HRESULT SetVirtualHostNameToFolderMapping(LPCTSTR hostName, LPCTSTR folderPath) {
			m_hfMap[hostName] = folderPath;
			if (!m_webview)
				return E_FAIL;
			//wil::com_ptr<ICoreWebView2_3> webView3 = m_pImpl->m_webview.try_query<ICoreWebView2_3>();
			ComPtr<ICoreWebView2_3> webView3;
			m_webview.As(&webView3);
			if (webView3) {
				return webView3->SetVirtualHostNameToFolderMapping(hostName, folderPath, COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY_CORS);
			}

			return E_FAIL;
		}
		//设置是否允许拖放
		HRESULT SetAllowExternalDrop(bool allow) {
			if (m_allowExternalDrop == allow)
				return S_OK;
			m_allowExternalDrop = allow;
			return put_AllowExternalDrop(allow ? TRUE : FALSE);
		}
		//设置是否允许上下文菜单
		HRESULT SetContextMenusEnabled(bool enable) {
			if (m_contextMenusEnabled == enable)
				return S_OK;
			m_contextMenusEnabled = enable;
			return put_ContextMenusEnabled(enable ? TRUE : FALSE);
		}
		//设置是否允许加速键
		HRESULT SetAcceleratorKeysEnabled(bool enable) {
			if (m_acceleratorKeysEnabled == enable)
				return S_OK;
			m_acceleratorKeysEnabled = enable;
			return put_AcceleratorKeysEnabled(enable ? TRUE : FALSE);
		}
		//设置是否显示状态栏
		HRESULT SetStatusBarEnabled(bool enable) {
			if (m_statusBarEnabled == enable)
				return S_OK;
			m_statusBarEnabled = enable;
			return put_StatusBarEnabled(enable ? TRUE : FALSE);
		}
		//设置是否可以放大缩小
		HRESULT SetZoomControlEnabled(bool enable) {
			if (m_setZoomControlEnabled == enable)
				return S_OK;
			m_setZoomControlEnabled = enable;
			return put_ZoomControlEnabled(enable ? TRUE : FALSE);
		}
		//添加一段javascript,当Document创建成功后调用
		HRESULT AddScriptToExecuteOnDocumentCreated(LPCTSTR script) {
			if (!m_webview) 
				return E_FAIL;
			return m_webview->AddScriptToExecuteOnDocumentCreated(script, NULL);
		}
		//执行javascript代码, script 是 javascript代码，handler 回调函数，可以为 NULL
		HRESULT ExecuteScript(LPCTSTR script, ICoreWebView2ExecuteScriptCompletedHandler* handler)	{
			if (!m_webview)
				return E_FAIL;
			return m_webview->ExecuteScript(script, handler);
		}
		//导航到主页
		HRESULT GotoHomePage() {
			if (!m_webview)
				return E_FAIL;
			return m_webview->Navigate(m_homepage);
		}
		//导航到url
		HRESULT Navigate(LPCTSTR url) {
			if (!m_webview)
				return E_FAIL;
			return m_webview->Navigate(url);
		}

		HRESULT put_AllowExternalDrop(BOOL allow) {
			if (!m_webController) 
				return E_FAIL;
			/*
			wil::com_ptr<ICoreWebView2Controller4> controller4 =
				m_pImpl->m_webController.try_query<ICoreWebView2Controller4>();
			*/
			ComPtr<ICoreWebView2Controller4> controller4;
			m_webController.As(&controller4);
			if (controller4) {
				return controller4->put_AllowExternalDrop(allow);
			}
			return E_FAIL;
		}
		HRESULT put_ContextMenusEnabled(BOOL enable) {
			if (!m_settings)
				return E_FAIL;
			return m_settings->put_AreDefaultContextMenusEnabled(enable);
		}
		HRESULT put_AcceleratorKeysEnabled(BOOL enable) {
			if (!m_settings)
				return E_FAIL;
			//wil::com_ptr<ICoreWebView2Settings3> settings3 = m_pImpl->m_settings.try_query<ICoreWebView2Settings3>();
			ComPtr<ICoreWebView2Settings3> settings3;
			m_settings.As(&settings3);
			if (!settings3)
				return E_FAIL;
			return settings3->put_AreBrowserAcceleratorKeysEnabled(enable);
		}
		HRESULT put_StatusBarEnabled(BOOL enable) {
			if (!m_settings)
				return E_FAIL;
			return m_settings->put_IsStatusBarEnabled(enable);
		}
		HRESULT put_ZoomControlEnabled(BOOL enable) {
			if (!m_settings)
				return E_FAIL;
			return m_settings->put_IsZoomControlEnabled(enable);
		}
		HRESULT put_IsVisible(BOOL visible) {
			if (!m_webController)
				return E_FAIL;
				
			return m_webController->put_IsVisible(visible);
		}
		HRESULT put_Bounds(const RECT& rcBound) {
			if (!m_webController)
				return E_FAIL;

			return m_webController->put_Bounds(rcBound);
		}

		HRESULT HandleTabMessageReceived(ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* eventArgs)
		{
			return S_OK;
		}
		HRESULT HandleTabHistoryUpdate(ICoreWebView2* webview)
		{
			return S_OK;
		}

		HRESULT SetMessageBroker() {
			if (!m_webview)
				return E_FAIL;

			m_messageBroker = Callback<ICoreWebView2WebMessageReceivedEventHandler>(
				[this](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* eventArgs) -> HRESULT
				{
					HandleTabMessageReceived(webview, eventArgs);

					return S_OK;
				});
			return m_webview->add_WebMessageReceived(m_messageBroker.Get(), &m_messageBrokerToken);
		}

		HRESULT RgisterHistoryChangedHandler() {
			if (!m_webview)
				return E_FAIL;

			HRESULT result = m_webview->add_HistoryChanged(Callback<ICoreWebView2HistoryChangedEventHandler>(
				[this](ICoreWebView2* webview, IUnknown* args) -> HRESULT
				{
					HandleTabHistoryUpdate(webview);

			        return S_OK;
				}).Get(), &m_historyUpdateForwarderToken);

				return result;
		}

		//取得webveiw2运行环境
		ICoreWebView2Environment* GetGetWebView2Env() {
			return m_webViewEnv.Get();
		}
		ICoreWebView2Controller* GetWebView2Controller() {
			return m_webController.Get();
		}
		//取得webview2
		ICoreWebView2* GetWebView2() {
			return m_webview.Get();
		}

	public:
		CWebView2UI* m_pHostUI;
		bool m_autoNavigate = true;
		bool m_scriptEnable = true;
		bool m_defaultScriptDialogsEnabled = true;
		bool m_isWebMessageEnabled = true;
		bool m_isBuiltInErrorPageEnabled = false;
		bool m_allowExternalDrop = false;
		bool m_contextMenusEnabled = false;
		bool m_acceleratorKeysEnabled = false;
		bool m_statusBarEnabled = false;
		bool m_setZoomControlEnabled = false;
		bool m_bMustHttps = false;
		CDuiString m_homepage;
		std::map<CDuiString, CDuiString> m_hfMap;
		HWND m_hWebViewWnd = NULL;

		ComPtr<ICoreWebView2Environment> m_webViewEnv; //m_contentEnv
		ComPtr<ICoreWebView2Controller> m_webController;
		ComPtr<ICoreWebView2> m_webview;
		ComPtr<ICoreWebView2Settings> m_settings;
		
		EventRegistrationToken m_historyUpdateForwarderToken = {};
		EventRegistrationToken m_messageBrokerToken = {};  // Message broker for browser pages loaded 
		ComPtr<ICoreWebView2WebMessageReceivedEventHandler> m_messageBroker;

	};

IMPLEMENT_DUICONTROL(CWebView2UI)

CWebView2UI::CWebView2UI() : m_pImpl(new CWebView2Impl)
{
	m_pImpl->AttachHost(this);
	m_pImpl->m_homepage = _T("about:blank");
}

CWebView2UI::~CWebView2UI()
{
	m_pImpl->CloseCurrentView();
}

LPCTSTR CWebView2UI::GetClass() const
{
	return DUI_CTR_WEBVIEW2;
}

LPVOID CWebView2UI::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, DUI_CTR_WEBVIEW2) == 0) return static_cast<CWebView2UI*>(this);
	return CControlUI::GetInterface(pstrName);
}

void CWebView2UI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("homepage")) == 0) SetHomepage(pstrValue);
	else if (_tcsicmp(pstrName, _T("autonavi")) == 0) SetAutoNavigate(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("scriptEnable")) == 0) SetIsScriptEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("defaultScriptDialogs")) == 0) SetAreDefaultScriptDialogsEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("webMessageEnabled")) == 0) SetIsWebMessageEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("builtInErrorPageEnabled")) == 0) SetIsBuiltInErrorPageEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("allowExternalDrop")) == 0) SetAllowExternalDrop(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("contextMenusEnabled")) == 0) SetContextMenusEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("acceleratorKeysEnabled")) == 0) SetAcceleratorKeysEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("statusBarEnabled")) == 0) SetStatusBarEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("zoomControlEnabled")) == 0) SetZoomControlEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("mustHttps")) == 0) SetMustHttps(_tcsicmp(pstrValue, _T("true")) == 0);
	else CControlUI::SetAttribute(pstrName, pstrValue);
}

HRESULT InitWebView2Environment(HWND hWnd, CWebView2Impl* pImpl)
{
	// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
	HRESULT hr = pImpl->m_webViewEnv->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
		[pImpl](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
			if (FAILED(result))
				return result;
			
			CWebView2UI* pHostUI = pImpl->m_pHostUI;

			pImpl->m_webController = controller;
			result = pImpl->m_webController->get_CoreWebView2(&pImpl->m_webview); // 获取ICoreWebView2实例并进行后续操作
			if (FAILED(result))
				return result;

			RECT rc = pHostUI->GetPos();
			pHostUI->SetPos(rc, true);

			// Register event handler for history change
			result = pImpl->RgisterHistoryChangedHandler();
			if (FAILED(result))
				return result;

			pImpl->m_webController->put_IsVisible(pHostUI->IsVisible());
			pImpl->m_webview->get_Settings(&pImpl->m_settings);

			auto color = pHostUI->GetBkColor();
			COREWEBVIEW2_COLOR backgroundColor = { 255, GetRValue(color), GetGValue(color), GetBValue(color) };
			//wil::com_ptr<ICoreWebView2Controller2> controller2 = m_pImpl->m_webController.query<ICoreWebView2Controller2>();
			ComPtr<ICoreWebView2Controller2> controller2;
			pImpl->m_webController.As(&controller2);
			controller2->put_DefaultBackgroundColor(backgroundColor);

			pImpl->DoOptionImplement();
			
			EventRegistrationToken token;
			pImpl->m_webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
				[pImpl](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
					if (pImpl->m_bMustHttps) {
						PWSTR uri;
						args->get_Uri(&uri);
						std::wstring source(uri);
						if (source.substr(0, 5) != L"https") {
							args->put_Cancel(true);
						}
						::CoTaskMemFree(uri);
					}
					pImpl->SendNotify(_T("NavigationStarting"), (WPARAM)webview, (LPARAM)args);
					return S_OK;
				}).Get(), &token);

			pImpl->m_webview->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
				[pImpl](ICoreWebView2* webview, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
					pImpl->SendNotify(_T("NavigationCompleted"), (WPARAM)webview, (LPARAM)args);
					return S_OK;
				}).Get(), &token);

			pImpl->m_webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
				[pImpl](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
					pImpl->SendNotify(_T("WebMessageReceived"), (WPARAM)webview, (LPARAM)args);
					return S_OK;
				}).Get(), &token);

			pImpl->m_webview->add_FrameNavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
				[pImpl](ICoreWebView2* webview, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
					pImpl->SendNotify(_T("FrameNavigationCompleted"), (WPARAM)webview, (LPARAM)args);
					return S_OK;
				}).Get(), nullptr);

			if (pImpl->m_autoNavigate) {
				result = pImpl->GotoHomePage();
			}
			pImpl->SendNotify(_T("OnWebView2Created"));
			return S_OK;
		}).Get());

	return hr;
}

void CWebView2UI::DoInit()
{
    CDuiString userDataDirectory = GetAppDataDirectory();
	userDataDirectory.Append(_T("\\User Data"));

	HWND hWnd = m_pManager->GetPaintWindow();
	//CDuiString webview2Path = CPaintManagerUI::GetInstancePath() + _T("webview2\\");
	HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataDirectory, nullptr,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[this, hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
				if (FAILED(result))
					return result;
				
				m_pImpl->m_webViewEnv = env;
				return InitWebView2Environment(hWnd, m_pImpl.get());
			}).Get());

	if (FAILED(hr))
	{
		OutputDebugString(L"Content WebViews environment creation failed\n");
	}
}

void CWebView2UI::SetPos(RECT rc, bool bNeedInvalidate)
{
	CControlUI::SetPos(rc, bNeedInvalidate);

	if (m_pImpl)
	{
		RECT rcBorderSize = GetScaleRect(m_rcBorderSize, m_nScale);
		rc.left += rcBorderSize.left;
		rc.top += rcBorderSize.top;
		rc.right -= rcBorderSize.right;
		rc.bottom -= rcBorderSize.bottom;

		m_pImpl->put_Bounds(rc);
	}
}

void CWebView2UI::SetInternVisible(bool bVisible)
{
	CControlUI::SetInternVisible(bVisible);

	m_pImpl->put_IsVisible(bVisible ? TRUE : FALSE);
}

bool CWebView2UI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl)
{
	CControlUI::PaintBorder(hDC);

	return true;
}

CDuiString CWebView2UI::GetAppDataDirectory()
{
	TCHAR path[MAX_PATH];
	CDuiString dataDirectory;
	HRESULT hr = SHGetFolderPath(nullptr, CSIDL_APPDATA, NULL, 0, path);
	if (SUCCEEDED(hr))
	{
		dataDirectory = path;
		dataDirectory.Append(_T("\\Microsoft\\"));
	}
	else
	{
		dataDirectory = _T(".\\");
	}

	dataDirectory.Append(_T("webcache")); //title
	return dataDirectory;
}

void CWebView2UI::SetVisible(bool bVisible)
{
	if (IsVisible() == bVisible) 
		return;

	CControlUI::SetVisible(bVisible);
	m_pImpl->put_IsVisible(bVisible ? TRUE : FALSE);
}

void CWebView2UI::SetHomepage(LPCTSTR url) 
{ 
	m_pImpl->m_homepage = url; 
}

LPCTSTR CWebView2UI::GetHomepage()
{ 
	return m_pImpl->m_homepage; 
}

HRESULT CWebView2UI::SetIsScriptEnabled(bool enable)
{ 
	return m_pImpl->SetIsScriptEnabled(enable); 
}

HRESULT CWebView2UI::SetAreDefaultScriptDialogsEnabled(bool enable)
{ 
	return m_pImpl->SetAreDefaultScriptDialogsEnabled(enable);
}

HRESULT CWebView2UI::SetIsWebMessageEnabled(bool enable)
{
	return m_pImpl->SetIsWebMessageEnabled(enable);
}

HRESULT CWebView2UI::SetIsBuiltInErrorPageEnabled(bool enable)
{
	return m_pImpl->SetIsBuiltInErrorPageEnabled(enable);
}

HRESULT CWebView2UI::SetVirtualHostNameToFolderMapping(LPCTSTR hostName, LPCTSTR folderPath)
{
	return m_pImpl->SetVirtualHostNameToFolderMapping(hostName, folderPath);
}

HRESULT CWebView2UI::SetAllowExternalDrop(bool allow)
{ 
	return m_pImpl->SetAllowExternalDrop(allow);
}

HRESULT CWebView2UI::SetContextMenusEnabled(bool enable)
{ 
	return m_pImpl->SetContextMenusEnabled(enable);
}

HRESULT CWebView2UI::SetAcceleratorKeysEnabled(bool enable)
{ 
	return m_pImpl->SetAcceleratorKeysEnabled(enable);
}

HRESULT CWebView2UI::SetStatusBarEnabled(bool enable)
{ 
	return m_pImpl->SetStatusBarEnabled(enable);
}

HRESULT CWebView2UI::SetZoomControlEnabled(bool enable)
{ 
	return m_pImpl->SetZoomControlEnabled(enable);
}

//设置创建成功后是否自动导航
void CWebView2UI::SetAutoNavigate(bool bAuto)
{ 
	m_pImpl->m_autoNavigate = bAuto; 
}

//设置是否只允许https
void CWebView2UI::SetMustHttps(bool must)
{ 
	m_pImpl->m_bMustHttps = must;
}

HRESULT CWebView2UI::AddScriptToExecuteOnDocumentCreated(LPCTSTR script)
{
	return m_pImpl->AddScriptToExecuteOnDocumentCreated(script);
}

HRESULT CWebView2UI::ExecuteScript(LPCTSTR script, SCRIPT_COMPLETED_CALLBACK handler)
{
	return m_pImpl->ExecuteScript(script, Callback<ICoreWebView2ExecuteScriptCompletedHandler>(handler).Get());
}

HRESULT CWebView2UI::GotoHomePage()
{ 
	return m_pImpl->GotoHomePage();
}

HRESULT CWebView2UI::Navigate(LPCTSTR url)
{ 
	return m_pImpl->Navigate(url); 
}

} 
