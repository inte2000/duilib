#ifndef __UIACTIVEX_H__
#define __UIACTIVEX_H__

#pragma once

struct IOleObject;


namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

class CActiveXCtrl;

template< class T >
class CSafeRelease
{
public:
    CSafeRelease(T* p) : m_p(p) { };
    ~CSafeRelease() { if( m_p != NULL ) m_p->Release(); };
    T* Detach() { T* t = m_p; m_p = NULL; return t; };
    T* m_p;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class DUILIB_API CActiveXUI : public CControlUI, public IMessageFilterUI
{
    DECLARE_DUICONTROL(CActiveXUI)
    friend class CActiveXCtrl;
public:
    CActiveXUI();
    virtual ~CActiveXUI();
    /*
    * 20240325 增加 DoInit，在这里调用 DoCreateControl()，解决原方案在 SetPos 中调用可能
    * 出现的顺序错误问题，比如当 ActiveXUI 是某个 tab 页的时候，只有 tab 页被激活时才会触发
    * SetPos，但是在这之前，用户可能同构 FindControl 得到 ActiveXUI 对象，并访问它的方法
    */
    void DoInit() { if (!m_bCreated) DoCreateControl(); }
    LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);
	UINT GetControlFlags() const;
	HWND GetNativeWindow() const;

    bool IsDelayCreate() const;
    void SetDelayCreate(bool bDelayCreate = true);

    bool CreateControl(const CLSID clsid);
    bool CreateControl(LPCTSTR pstrCLSID);
    HRESULT GetControl(const IID iid, LPVOID* ppRet);
	CLSID GetClisd() const;
    CDuiString GetModuleName() const;
    void SetModuleName(LPCTSTR pstrText);

    void SetVisible(bool bVisible = true);
    void SetInternVisible(bool bVisible = true);
	void SetPos(RECT rc, bool bNeedInvalidate = true);
	void Move(SIZE szOffset, bool bNeedInvalidate = true);
    bool DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl);

    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

    LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

protected:
    virtual void ReleaseControl();
    virtual bool DoCreateControl();

protected:
    CLSID m_clsid;
    CDuiString m_sModuleName;
    bool m_bCreated;
    bool m_bDelayCreate;
    IOleObject* m_pUnk;
    CActiveXCtrl* m_pControl;
    HWND m_hwndHost;
};

} // namespace DuiLib

#endif // __UIACTIVEX_H__
