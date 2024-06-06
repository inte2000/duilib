#pragma once
//////////////BEGIN消息映射宏定义////////////////////////////////////////////////////
///

namespace DuiLib
{

#define _USEIMM					1
#define MAX_FONT_ID				30000

#define SCROLLBAR_LINESIZE      8

    // 鼠标光标定义
#define DUI_ARROW           32512
#define DUI_IBEAM           32513
#define DUI_WAIT            32514
#define DUI_CROSS           32515
#define DUI_UPARROW         32516
#define DUI_SIZE            32640
#define DUI_ICON            32641
#define DUI_SIZENWSE        32642
#define DUI_SIZENESW        32643
#define DUI_SIZEWE          32644
#define DUI_SIZENS          32645
#define DUI_SIZEALL         32646
#define DUI_NO              32648
#define DUI_HAND            32649

enum DuiSig
{
	DuiSig_end = 0, // [marks end of message map]
	DuiSig_lwl,     // LRESULT (WPARAM, LPARAM)
	DuiSig_vn,      // void (TNotifyUI)
};

class CControlUI;

// Structure for notifications to the outside world
typedef struct tagTNotifyUI 
{
	CDuiString sType;
	CDuiString sName;
    UINT uId;
    CDuiString sVirtualWnd;
	CControlUI* pSender;
	DWORD dwTimestamp;
	POINT ptMouse;
	WPARAM wParam;
	LPARAM lParam;
} TNotifyUI;

class CNotifyPump;
typedef void (CNotifyPump::*DUI_PMSG)(TNotifyUI& msg);  //指针类型

union DuiMessageMapFunctions
{
	DUI_PMSG pfn;   // generic member function pointer
	LRESULT (CNotifyPump::*pfn_Notify_lwl)(WPARAM, LPARAM);
	void (CNotifyPump::*pfn_Notify_vn)(TNotifyUI&);
};

//定义所有消息类型
//////////////////////////////////////////////////////////////////////////

#define DUI_MSGTYPE_MENU                   (_T("menu"))
#define DUI_MSGTYPE_LINK                   (_T("link"))

#define DUI_MSGTYPE_TIMER                  (_T("timer"))
#define DUI_MSGTYPE_CLICK                  (_T("click"))

#define DUI_MSGTYPE_RETURN                 (_T("return"))
#define DUI_MSGTYPE_SCROLL                 (_T("scroll"))

#define DUI_MSGTYPE_DROPDOWN               (_T("dropdown"))
#define DUI_MSGTYPE_SETFOCUS               (_T("setfocus"))

#define DUI_MSGTYPE_KILLFOCUS              (_T("killfocus"))
#define DUI_MSGTYPE_ITEMCLICK 		   	   (_T("itemclick"))
#define DUI_MSGTYPE_TABSELECT              (_T("tabselect"))

#define DUI_MSGTYPE_ITEMSELECT 		   	   (_T("itemselect"))
#define DUI_MSGTYPE_ITEMEXPAND             (_T("itemexpand"))

#define DUI_MSGTYPE_WINDOWINIT             (_T("windowinit"))
#define DUI_MSGTYPE_BUTTONDOWN 		   	   (_T("buttondown"))
#define DUI_MSGTYPE_MOUSEENTER			   (_T("mouseenter"))
#define DUI_MSGTYPE_MOUSELEAVE			   (_T("mouseleave"))

#define DUI_MSGTYPE_TEXTCHANGED            (_T("textchanged"))
#define DUI_MSGTYPE_HEADERCLICK            (_T("headerclick"))
#define DUI_MSGTYPE_ITEMDBCLICK            (_T("itemdbclick"))
#define DUI_MSGTYPE_SHOWACTIVEX            (_T("showactivex"))
#define DUI_MSGTYPE_TEXTROLLEND 		   (_T("textrollend"))

#define DUI_MSGTYPE_ITEMCOLLAPSE           (_T("itemcollapse"))
#define DUI_MSGTYPE_ITEMACTIVATE           (_T("itemactivate"))
#define DUI_MSGTYPE_VALUECHANGED           (_T("valuechanged"))

#define DUI_MSGTYPE_SELECTCHANGED 		   (_T("selectchanged"))

//ipaddress changed
#define DUI_EVT_IPADDRESSCHANGED		    (_T("ipchanged"))

//color palette changed
#define DUI_EVT_COLORCHANGED		    (_T("colorchanged"))

//menu item click
#define DUI_EVT_MENU_CLICK                 (_T("menuclick"))

//toggle changed
#define DUI_EVT_TOGGLECHANGED             (_T("togglechanged"))

//hot key changed
#define DUI_EVT_HOTKEYCHANGED		    (_T("hotkeychanged"))

//tree ctrl event
#define DUI_EVT_TC_SELCHANGING 		   (_T("TreeSelChanging"))
#define DUI_EVT_TC_SELCHANGED 		   (_T("TreeSelChanged"))
#define DUI_EVT_TC_EXPAND 		   (_T("TreeExpand"))
#define DUI_EVT_TC_CHECKSTATE 		   (_T("TreeCheckState"))
#define DUI_EVT_TC_DBCLICK 		   (_T("TreeDbClick"))
#define DUI_EVT_TC_ITEMDELETED 		   (_T("TreeItemDeleted"))
#define DUI_EVT_TC_RCLICK 		   (_T("TreeRClick"))


//////////////////////////////////////////////////////////////////////////



struct DUI_MSGMAP_ENTRY;
struct DUI_MSGMAP
{
#ifndef UILIB_STATIC
	const DUI_MSGMAP* (PASCAL* pfnGetBaseMap)();
#else
	const DUI_MSGMAP* pBaseMap;
#endif
	const DUI_MSGMAP_ENTRY* lpEntries;
};

//结构定义
struct DUI_MSGMAP_ENTRY //定义一个结构体，来存放消息信息
{
	CDuiString sMsgType;          // DUI消息类型
	CDuiString sCtrlName;         // 控件名称
	UINT       nSig;              // 标记函数指针类型
	DUI_PMSG   pfn;               // 指向函数的指针
};

#define DUI_EVENT_MAP_BEGIN()                     \
public:                                           \
    virtual BOOL _HandleNotify(TNotifyUI& msg)     \
    {                                             \
        const CDuiString& evtType = msg.sType;    \
        const CDuiString& evtName = msg.sName;    \
        UINT evtId = msg.uId;                     \

#define DUI_EVENT_MAP_END()                         \
        return __super::_HandleNotify(msg);         \
    }                                               \

#define CHAIN_DUI_EVENT_MAP(ChainClass)                 \
        if(ChainClass::_HandleEvent(msg))          \
            return TRUE;                            \

// void OnEvent(TNotifyUI& msg)
#define ON_EVENT_HANDLER(evt, func)                 \
    if(evtType.CompareNoCase(evt) == 0)              \
    {                                                \
        if(func(msg))                                \
            return TRUE;                             \
    } 

// void OnEvent(TNotifyUI& msg)
#define ON_EVENT_NAME_HANDLER(evt, name, func)      \
    if(evtType.CompareNoCase(evt) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                \
        func(msg);                              \
        return TRUE;                             \
    } 

// void OnEvent(TNotifyUI& msg)
#define ON_EVENT_ID_HANDLER(evt, id, func)              \
    if(evtType.CompareNoCase(evt) == 0 && evtId == id)   \
    {                                                   \
        func(msg)                                  \
        return TRUE;                                \
    } 

// void OnEvent(TNotifyUI& msg)
#define ON_EVENT_ID_RANGE_HANDLER(evt, ids, ide, func)              \
    if(evtType.CompareNoCase(evt) == 0 && ((evtId >= ids) && (evtId <= ide)))   \
    {                                                   \
        func(msg);                                  \
        return TRUE;                                \
    } 


// void OnWindowInit(void)
#define ON_WINDOWINIT_HANDLER(func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_WINDOWINIT) == 0)      \
    {                                                              \
        func();                                                 \
        return TRUE;                                          \
    } 

// void OnClick(void)
#define ON_CLICK_NAME_HANDLER(name, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_CLICK) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                              \
        func();                                                 \
        return TRUE;                                          \
    }

// void OnClick(void)
#define ON_CLICK_ID_HANDLER(id, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_CLICK) == 0 && evtId == id)      \
    {                                                              \
        func();                                                 \
        return TRUE;                                          \
    } 

// void OnClick(TNotifyUI& msg)
#define ON_CLICK_ID_RANGE_HANDLER(ids, ide, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_CLICK) == 0 && ((evtId >= ids) && (evtId <= ide)))      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// void OnMenuClick(void)
#define ON_MENU_CLICK_NAME_HANDLER(name, func)                      \
    if(evtType.CompareNoCase(DUI_EVT_MENU_CLICK) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                               \
        func();                                               \
        return TRUE;                                            \
    } 

// void OnMenuClick(void)
#define ON_MENU_CLICK_ID_HANDLER(id, func)                      \
    if(evtType.CompareNoCase(DUI_EVT_MENU_CLICK) == 0 && evtId == id)      \
    {                                                               \
        func();                                               \
        return TRUE;                                            \
    } 

// void OnToggleChanged(BOOL bToggled)
#define ON_TOGGLE_CHANGED_NAME_HANDLER(name, func)                      \
    if(evtType.CompareNoCase(DUI_EVT_TOGGLECHANGED) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                               \
        func((BOOL)msg.wParam);                                               \
        return TRUE;                                            \
    } 

// void OnToggleChanged(BOOL bToggled)
#define ON_TOGGLE_CHANGED_ID_HANDLER(id, func)                      \
    if(evtType.CompareNoCase(DUI_EVT_TOGGLECHANGED) == 0 && evtId == id)      \
    {                                                               \
        func((BOOL)msg.wParam);                                               \
        return TRUE;                                            \
    } 

// BOOL OnTimer(UINT_PTR id)
#define ON_TIMER_HANDLER(func)                          \
    if(evtType.CompareNoCase(DUI_MSGTYPE_TIMER) == 0)      \
    {                                                              \
        if(func(msg.wParam))                                              \
            return TRUE;                                            \
    } 

// void OnLinkClick(void)
#define ON_LINK_NAME_HANDLER(name, func)                          \
    if(evtType.CompareNoCase(DUI_MSGTYPE_LINK) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                     \
        func();                                              \
        return TRUE;                                            \
    }

// void OnLinkClick(void)
#define ON_LINK_ID_HANDLER(id, func)                          \
    if(evtType.CompareNoCase(DUI_MSGTYPE_LINK) == 0 && evtId == id)      \
    {                                                     \
        func();                                              \
        return TRUE;                                            \
    } 

// void OnItemClick(void)
#define ON_ITEMCLICK_NAME_HANDLER(name, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_ITEMCLICK) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                              \
        func();                                                 \
        return TRUE;                                          \
    } 

// void OnItemClick(void)
#define ON_ITEMCLICK_ID_HANDLER(id, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_ITEMCLICK) == 0 && evtId == id)      \
    {                                                              \
        func();                                                 \
        return TRUE;                                          \
    } 

// void OnItemSelect(TNotifyUI& msg)
#define ON_ITEMSELECT_NAME_HANDLER(name, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_ITEMSELECT) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// void OnItemSelect(TNotifyUI& msg)
#define ON_ITEMSELECT_ID_HANDLER(id, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_ITEMSELECT) == 0 && evtId == id)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// void OnTextChanged(TNotifyUI& msg)
#define ON_TEXTCHANGED_NAME_HANDLER(name, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_TEXTCHANGED) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// void OnTextChanged(TNotifyUI& msg)
#define ON_TEXTCHANGED_ID_HANDLER(id, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_TEXTCHANGED) == 0 && evtId == id)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// void OnTextChanged(TNotifyUI& msg)
#define ON_TEXTCHANGED_ID_RANGE_HANDLER(ids, ide, func)              \
    if(evtType.CompareNoCase(DUI_MSGTYPE_TEXTCHANGED) == 0 && ((evtId >= ids) && (evtId <= ide)))   \
    {                                                   \
        func(msg);                                  \
        return TRUE;                                \
    } 

// void OnColorChanged(TNotifyUI& msg)
#define ON_COLORCHANGED_NAME_HANDLER(name, func)                         \
    if(evtType.CompareNoCase(DUI_EVT_COLORCHANGED) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// void OnColorChanged(TNotifyUI& msg)
#define ON_COLORCHANGED_ID_HANDLER(id, func)                         \
    if(evtType.CompareNoCase(DUI_EVT_COLORCHANGED) == 0 && evtId == id)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// BOOL OnColorChanged(TNotifyUI& msg)
#define ON_COLORCHANGED_HANDLER(func)                         \
    if(evtType.CompareNoCase(DUI_EVT_COLORCHANGED) == 0)      \
    {                                                              \
        if(func(msg))                                                 \
            return TRUE;                                          \
    } 

// void OnIpChanged(TNotifyUI& msg)
#define ON_IPCHANGED_NAME_HANDLER(name, func)                         \
    if(evtType.CompareNoCase(DUI_EVT_IPADDRESSCHANGED) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// void OnIpChanged(TNotifyUI& msg)
#define ON_IPCHANGED_ID_HANDLER(id, func)                         \
    if(evtType.CompareNoCase(DUI_EVT_IPADDRESSCHANGED) == 0 && evtId == id)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// BOOL OnIpChanged(TNotifyUI& msg)
#define ON_IPCHANGED_HANDLER(func)                         \
    if(evtType.CompareNoCase(DUI_EVT_IPADDRESSCHANGED) == 0)      \
    {                                                              \
        if(func(msg))                                                 \
            return TRUE;                                          \
    } 

// BOOL OnSelectChanged(TNotifyUI& msg)
#define ON_SELECTCHANGED_HANDLER(func)                          \
    if(evtType.CompareNoCase(DUI_MSGTYPE_SELECTCHANGED) == 0)      \
    {                                                              \
        if(func(msg))                                              \
            return TRUE;                                            \
    } 

// void OnSelectChanged(TNotifyUI& msg)
#define ON_SELECTCHANGED_NAME_HANDLER(name, func)                          \
    if(evtType.CompareNoCase(DUI_MSGTYPE_SELECTCHANGED) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                            \
        func(msg);                                              \
        return TRUE;                                            \
    } 

// void OnSelectChanged(TNotifyUI& msg)
#define ON_SELECTCHANGED_ID_HANDLER(id, func)                          \
    if(evtType.CompareNoCase(DUI_MSGTYPE_SELECTCHANGED) == 0 && evtId == id)      \
    {                                                            \
        func(msg);                                              \
        return TRUE;                                            \
    } 

// void OnTextRollEnd(void)
#define ON_TEXTROLLEND_NAME_HANDLER(name, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_TEXTROLLEND) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                              \
        func();                                                 \
        return TRUE;                                          \
    } 

// void OnTextRollEnd(void)
#define ON_TEXTROLLEND_ID_HANDLER(id, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_TEXTROLLEND) == 0 && evtId == id)      \
    {                                                              \
        func();                                                 \
        return TRUE;                                          \
    } 

// void OnValueChanged(TNotifyUI& msg)
#define ON_VALUECHANGED_NAME_HANDLER(name, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_VALUECHANGED) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// void OnValueChanged(TNotifyUI& msg)
#define ON_VALUECHANGED_ID_HANDLER(id, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_VALUECHANGED) == 0 && evtId == id)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// void OnValueChanged(TNotifyUI& msg)
#define ON_VALUECHANGED_ID_RANGE_HANDLER(ids, ide, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_VALUECHANGED) == 0 && ((evtId >= ids) && (evtId <= ide)))      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// BOOL OnValueChanged(TNotifyUI& msg)
#define ON_VALUECHANGED_HANDLER(func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_VALUECHANGED) == 0)      \
    {                                                              \
        if(func(msg))                                                 \
            return TRUE;                                          \
    } 

// void OnHotKeyChanged(TNotifyUI& msg)
#define ON_HOTKEYCHANGED_NAME_HANDLER(name, func)                         \
    if(evtType.CompareNoCase(DUI_EVT_HOTKEYCHANGED) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// void OnHotKeyChanged(TNotifyUI& msg)
#define ON_HOTKEYCHANGED_ID_HANDLER(id, func)                         \
    if(evtType.CompareNoCase(DUI_EVT_HOTKEYCHANGED) == 0 && evtId == id)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// BOOL OnHotKeyChanged(TNotifyUI& msg)
#define ON_HOTKEYCHANGED_HANDLER(func)                         \
    if(evtType.CompareNoCase(DUI_EVT_HOTKEYCHANGED) == 0)      \
    {                                                              \
        if(func(msg))                                                 \
            return TRUE;                                          \
    } 

// void OnTabSelect(TNotifyUI& msg)
#define ON_TABSELECT_NAME_HANDLER(name, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_TABSELECT) == 0 && evtName.CompareNoCase(name) == 0)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 

// void OnTabSelect(TNotifyUI& msg)
#define ON_TABSELECT_ID_HANDLER(id, func)                         \
    if(evtType.CompareNoCase(DUI_MSGTYPE_TABSELECT) == 0 && evtId == id)      \
    {                                                              \
        func(msg);                                                 \
        return TRUE;                                          \
    } 


// Mark method as deprecated.
// example: DUI_DEPRECATED void func();
#if defined(_MSC_VER)
#  define DUI_DEPRECATED __declspec(deprecated)
#elif defined(__GNUC__)
#  define DUI_DEPRECATED __attribute__ ((deprecated))
#else
#  pragma message("WARNING: You need to implement DUI_DEPRECATED for this compiler")
#  define DUI_DEPRECATED
#endif


///
//////////////END消息映射宏定义////////////////////////////////////////////////////


//////////////BEGIN控件名称宏定义//////////////////////////////////////////////////
///

#define  DUI_CTR_EDIT                            (_T("Edit"))
#define  DUI_CTR_LIST                            (_T("List"))
#define  DUI_CTR_TEXT                            (_T("Text"))
#define  DUI_CTR_MENU                            (_T("Menu"))

#define  DUI_CTR_ILIST                           (_T("IList"))
#define  DUI_CTR_LABEL                           (_T("Label"))
#define  DUI_CTR_FLASH							 (_T("Flash"))

#define  DUI_CTR_BUTTON                          (_T("Button"))
#define  DUI_CTR_OPTION                          (_T("Option"))
#define  DUI_CTR_SLIDER                          (_T("Slider"))
#define  DUI_CTR_TOGGLE                          (_T("Toggle"))
#define  DUI_CTR_ROTATE                          (_T("Rotate"))
#define  DUI_CTR_HOTKEY                          (_T("HotKey"))

#define  DUI_CTR_CONTROL                         (_T("Control"))
#define  DUI_CTR_ACTIVEX                         (_T("ActiveX"))
#define  DUI_CTR_GIFANIM                         (_T("GifAnim"))

#define  DUI_CTR_PROGRESS                        (_T("Progress"))
#define  DUI_CTR_RICHEDIT                        (_T("RichEdit"))
#define  DUI_CTR_CHECKBOX                        (_T("CheckBox"))
#define  DUI_CTR_COMBO                           (_T("Combo"))
#define  DUI_CTR_DATETIME                        (_T("DateTime"))
#define  DUI_CTR_TREEVIEW                        (_T("TreeView"))
#define  DUI_CTR_TREENODE                        (_T("TreeNode"))
#define  DUI_CTR_GROUPBOX                        (_T("GroupBox"))
#define  DUI_CTR_ROLLTEXT                        (_T("RollText"))
#define  DUI_CTR_EMBEDWND                        (_T("EmbedWnd"))

#define  DUI_CTR_ILISTITEM                       (_T("IListItem"))
#define  DUI_CTR_CONTAINER                       (_T("Container"))
#define  DUI_CTR_TABLAYOUT                       (_T("TabLayout"))
#define  DUI_CTR_SCROLLBAR                       (_T("ScrollBar"))
#define  DUI_CTR_IPADDRESS                       (_T("IPAddress"))

#define  DUI_CTR_ICONTAINER                      (_T("IContainer"))
#define  DUI_CTR_ILISTOWNER                      (_T("IListOwner"))
#define  DUI_CTR_LISTHEADER                      (_T("ListHeader"))
#define  DUI_CTR_TILELAYOUT                      (_T("TileLayout"))
#define  DUI_CTR_WEBBROWSER                      (_T("WebBrowser"))
#define  DUI_CTR_FADEBUTTON                      (_T("FadeButton"))
#define  DUI_CTR_CUSTOMDRAW                      (_T("CustomDraw"))

#define  DUI_CTR_CHILDLAYOUT                     (_T("ChildLayout"))
#define  DUI_CTR_LISTELEMENT                     (_T("ListElement"))
#define  DUI_CTR_VIRTUALLIST                     (_T("VirtualList"))
#define  DUI_CTR_MENUELEMENT                     (_T("MenuElement"))

#define  DUI_CTR_COLORPALETTE                    (_T("ColorPalette"))
#define  DUI_CTR_EXPANDBUTTON                    (_T("ExpandButton"))
#define  DUI_CTR_EXPANDOPTION                    (_T("ExpandOption"))
#define  DUI_CTR_CUSTOMDRAW_EX                   (_T("CustomDrawEx"))

#define  DUI_CTR_ANIMATELAYOUT                   (_T("AnimateLayout"))

#define  DUI_CTR_VERTICALLAYOUT                  (_T("VerticalLayout"))
#define  DUI_CTR_LISTHEADERITEM                  (_T("ListHeaderItem"))
#define  DUI_CTR_ANIMATELOADING                  (_T("AnimateLoading"))

#define  DUI_CTR_LISTHBOXELEMENT                 (_T("ListHBoxElement"))
#define  DUI_CTR_LISTTEXTELEMENT                 (_T("ListTextElement"))

#define  DUI_CTR_HORIZONTALLAYOUT                (_T("HorizontalLayout"))
#define  DUI_CTR_LISTLABELELEMENT                (_T("ListLabelElement"))
#define  DUI_CTR_ANIMATETABLAYOUT                (_T("AnimateTabLayout"))

#define  DUI_CTR_LISTCONTAINERELEMENT            (_T("ListContainerElement"))

///
//////////////END控件名称宏定义//////////////////////////////////////////////////


}// namespace DuiLib

