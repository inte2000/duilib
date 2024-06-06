#ifndef __UIMENU_H__
#define __UIMENU_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "observer_impl_base.h"
#include "..\Control\UIList.h"

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
struct ContextMenuParam
{
	// 1: remove all
	// 2: remove the sub menu
	WPARAM wParam;
	HWND hWnd;
};

enum MenuAlignment
{
	eMenuAlignment_Left = 1 << 1,
	eMenuAlignment_Top = 1 << 2,
	eMenuAlignment_Right = 1 << 3,
	eMenuAlignment_Bottom = 1 << 4,
};

enum MenuItemDefaultInfo
{
	ITEM_DEFAULT_HEIGHT = 24,		//每一个item的默认高度（只在竖状排列时自定义）
	ITEM_DEFAULT_WIDTH = 134,		//窗口的默认宽度

	ITEM_DEFAULT_ICON_WIDTH = 35,	////默认图标所占宽度
	ITEM_DEFAULT_ICON_SIZE = 16,	//默认图标的大小
	ITEM_DEFAULT_EXPLAND_ICON_WIDTH = 30,	//默认下级菜单扩展图标所占宽度
	ITEM_DEFAULT_EXPLAND_ICON_SIZE = 9,		//默认下级菜单扩展图标的大小

	DEFAULT_LINE_LEFT_WIDTH = 5,	////默认分隔线的左边距
	DEFAULT_LINE_HEIGHT = 10,		//默认分隔线所占高度
	DEFAULT_LINE_COLOR = 0xFFB5B5B5	//默认分隔线颜色

};

//typedef class ObserverImpl<BOOL, ContextMenuParam> ContextMenuObserver;
//typedef class ReceiverImpl<BOOL, ContextMenuParam> ContextMenuReceiver;

class DUILIB_API ContextMenuObserver : public ObserverImpl<BOOL, ContextMenuParam>
{
public:
	ContextMenuObserver() : m_pNotifyPm(NULL) {}
	void SetNotifyPaintManager(CPaintManagerUI* pm) { m_pNotifyPm = pm; }
	CPaintManagerUI* GetNotifyPaintManager() const { return m_pNotifyPm; }
private:
	CPaintManagerUI* m_pNotifyPm;

};

class DUILIB_API ContextMenuReceiver : public ReceiverImpl<BOOL, ContextMenuParam>
{
};

extern ContextMenuObserver s_context_menu_observer;


class CListUI;
class DUILIB_API CMenuUI : public CListUI
{
	DECLARE_DUICONTROL(CMenuUI)
public:
	CMenuUI();
	~CMenuUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

	virtual void DoEvent(TEventUI& event);

    virtual bool Add(CControlUI* pControl);
    virtual bool AddAt(CControlUI* pControl, int iIndex);

    virtual int GetItemIndex(CControlUI* pControl) const;
    virtual bool SetItemIndex(CControlUI* pControl, int iIndex);
    virtual bool Remove(CControlUI* pControl, bool bDoNotDestroy=false);

	SIZE EstimateSize(SIZE szAvailable);

	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
};

/////////////////////////////////////////////////////////////////////////////////////
//

// MenuElementUI

class CMenuElementUI;
class DUILIB_API CMenuWnd : public CWindowWnd, public ContextMenuReceiver
{
public:
	CMenuWnd();
	void LoadMenu(CMenuElementUI* pOwner, STRINGorID xml, LPCTSTR pSkinType, CPaintManagerUI *pMsgManger);
	void TrackMenu(POINT point, DWORD dwAlignment);

    LPCTSTR GetWindowClassName() const;
    void OnFinalMessage(HWND hWnd);

	CMenuUI* GetMenuUI() { return static_cast<CMenuUI*>(m_pm.GetRoot()); }

//private:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL Receive(ContextMenuParam param);
	void SetMenuItemText(LPCTSTR itemName, LPCTSTR lpstrText);
	void SetMenuItemText(UINT id, LPCTSTR lpstrText);
	void DeleteMenuItem(LPCTSTR itemName);
	void SetMenuItemEnable(LPCTSTR itemName, bool bEnable);

public:
	STRINGorID m_xml;
	CDuiString m_sType;
    CPaintManagerUI m_pm;
    CMenuElementUI* m_pOwner;
    CMenuUI* m_pLayout;
};

class CListContainerElementUI;
class DUILIB_API CMenuElementUI : public CListContainerElementUI
{
	DECLARE_DUICONTROL(CMenuElementUI)
	friend CMenuWnd;
public:
    CMenuElementUI();
	~CMenuElementUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    bool DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl);
	void DrawItemText(HDC hDC, const RECT& rcItem);
	void DrawItemIcon(HDC hDC, const RECT& rcItem);
	SIZE EstimateSize(SIZE szAvailable);
	bool Activate();
	void DoEvent(TEventUI& event);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	CMenuWnd* GetMenuWnd();
	void CreateMenuWnd();
	void SetTypeSeparator(bool bSeparator) { m_bSeparator = bSeparator; }
	void SetIcon(LPCTSTR pStrIcon);
	void SetIconSize(LONG cx, LONG cy);

protected:
	void PostMenuMessage(UINT id) const;

protected:
	CMenuWnd* m_pWindow;
	bool m_bSeparator;
	SIZE m_szIconSize;
	TDrawInfo m_Icon;
};

} // namespace DuiLib

#endif // __UIMENU_H__
