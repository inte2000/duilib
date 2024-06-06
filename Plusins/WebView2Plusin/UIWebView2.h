#ifndef __UIWEBVIEW2_H__
#define __UIWEBVIEW2_H__

//#include <ExDisp.h>
#include <memory>
#include "..\..\DuiLib\UIlib.h"

namespace DuiLib {

	typedef HRESULT(*SCRIPT_COMPLETED_CALLBACK)(HRESULT errorCode, LPCWSTR resultObjectAsJson);

	class CWebView2Impl;

	class DUILIB_API CWebView2UI : public CControlUI
	{
	DECLARE_DUICONTROL(CWebView2UI)
	public:
		CWebView2UI();
		virtual ~CWebView2UI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
		void DoInit() override;
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		void SetVisible(bool bVisible = true) override;
	public:
		void SetHomepage(LPCTSTR url);
		LPCTSTR GetHomepage();
		HRESULT SetIsScriptEnabled(bool enable);
		HRESULT SetAreDefaultScriptDialogsEnabled(bool enable);
		HRESULT SetIsWebMessageEnabled(bool enable);
		HRESULT SetIsBuiltInErrorPageEnabled(bool enable);
		HRESULT SetVirtualHostNameToFolderMapping(LPCTSTR hostName, LPCTSTR folderPath);
		HRESULT SetAllowExternalDrop(bool allow);
		HRESULT SetContextMenusEnabled(bool enable);
		HRESULT SetAcceleratorKeysEnabled(bool enable);
		HRESULT SetStatusBarEnabled(bool enable);
		HRESULT SetZoomControlEnabled(bool enable);
		//设置创建成功后是否自动导航
		void SetAutoNavigate(bool bAuto);
		//设置是否只允许https
		void SetMustHttps(bool must);
#if 0	
		//将Native对象注入到webview中供javascript调用
		// T 必须实现 className 成员函数, 这条函数返回的串为 javascript 调用的对象名
		template<typename T>
		void InjectObjectToScript()
		{
			if (!m_pManager || !m_pImpl->m_webview) return;
			wil::com_ptr<T> hostObj = Microsoft::WRL::Make<T>(m_pManager->GetPaintWindow());
			VARIANT remoteObjectAsVariant = {};
			hostObj.query_to<IDispatch>(&remoteObjectAsVariant.pdispVal);
			remoteObjectAsVariant.vt = VT_DISPATCH;
			m_pImpl->m_webview->AddHostObjectToScript(T::className(), &remoteObjectAsVariant);
			remoteObjectAsVariant.pdispVal->Release();
		}
#endif
		HRESULT AddScriptToExecuteOnDocumentCreated(LPCTSTR script);
		HRESULT ExecuteScript(LPCTSTR script, SCRIPT_COMPLETED_CALLBACK handler);
		HRESULT GotoHomePage();
		HRESULT Navigate(LPCTSTR url);
	protected:
		void SetInternVisible(bool bVisible = true) override;
		bool DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl) override;
		CDuiString GetAppDataDirectory();
	protected:
		std::unique_ptr<CWebView2Impl> m_pImpl;
	};


} // namespace DuiLib

#endif // __UIWEBVIEW2_H__