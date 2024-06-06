#include "StdAfx.h"
#include "ControlFactory.h"

namespace DuiLib 
{
#define INNER_REGISTER_DUICONTROL(class_name)\
	RegistControl(_T(#class_name), (CreateClass)class_name::CreateControl);

	CControlFactory::CControlFactory()
	{
		INNER_REGISTER_DUICONTROL(CControlUI); 
		INNER_REGISTER_DUICONTROL(CContainerUI); 
		INNER_REGISTER_DUICONTROL(CButtonUI); 
		INNER_REGISTER_DUICONTROL(CComboUI); 
		INNER_REGISTER_DUICONTROL(CDateTimeUI); 
		INNER_REGISTER_DUICONTROL(CEditUI); 
		INNER_REGISTER_DUICONTROL(CActiveXUI); 
		INNER_REGISTER_DUICONTROL(CFlashUI); 
		INNER_REGISTER_DUICONTROL(CGifAnimUI); 
		INNER_REGISTER_DUICONTROL(CGroupBoxUI); 
		INNER_REGISTER_DUICONTROL(CIPAddressUI); 
		INNER_REGISTER_DUICONTROL(CLabelUI); 
		INNER_REGISTER_DUICONTROL(CListUI); 
		INNER_REGISTER_DUICONTROL(CListHeaderUI); 
		INNER_REGISTER_DUICONTROL(CListHeaderItemUI); 
		INNER_REGISTER_DUICONTROL(CListLabelElementUI); 
		INNER_REGISTER_DUICONTROL(CListTextElementUI); 
		INNER_REGISTER_DUICONTROL(CListHBoxElementUI);
		INNER_REGISTER_DUICONTROL(CListContainerElementUI); 
		INNER_REGISTER_DUICONTROL(CMenuUI); 
		INNER_REGISTER_DUICONTROL(CMenuElementUI); 
		INNER_REGISTER_DUICONTROL(COptionUI); 
		INNER_REGISTER_DUICONTROL(CCheckBoxUI); 
		INNER_REGISTER_DUICONTROL(CProgressUI); 
		INNER_REGISTER_DUICONTROL(CRichEditUI); 
		INNER_REGISTER_DUICONTROL(CScrollBarUI); 
		INNER_REGISTER_DUICONTROL(CSliderUI); 
		INNER_REGISTER_DUICONTROL(CTextUI); 
		INNER_REGISTER_DUICONTROL(CTreeNodeUI); 
		INNER_REGISTER_DUICONTROL(CTreeViewUI); 
		INNER_REGISTER_DUICONTROL(CWebBrowserUI); 
		INNER_REGISTER_DUICONTROL(CAnimationTabLayoutUI); 
		INNER_REGISTER_DUICONTROL(CChildLayoutUI); 
		INNER_REGISTER_DUICONTROL(CHorizontalLayoutUI); 
		INNER_REGISTER_DUICONTROL(CTabLayoutUI); 
		INNER_REGISTER_DUICONTROL(CTileLayoutUI); 
		INNER_REGISTER_DUICONTROL(CVerticalLayoutUI); 
		INNER_REGISTER_DUICONTROL(CRollTextUI); 
		INNER_REGISTER_DUICONTROL(CHotKeyUI); 
		INNER_REGISTER_DUICONTROL(CFadeButtonUI); 
		INNER_REGISTER_DUICONTROL(CRotateUI); 
		INNER_REGISTER_DUICONTROL(CExpandButtonUI);
		INNER_REGISTER_DUICONTROL(CExpandOptionUI);
	}

	CControlFactory::~CControlFactory()
	{
	}

	CControlUI* CControlFactory::CreateControl(CDuiString strClassName)
	{
		CreateClass pCreator = static_cast<CreateClass>(m_mapControl.Find(strClassName));
		if (pCreator == NULL)
			return NULL;

		return pCreator();
	}

	void CControlFactory::RegistControl(CDuiString strClassName, CreateClass pFunc)
	{
		m_mapControl.Insert(strClassName, (LPVOID)pFunc);
	}

	CControlFactory* CControlFactory::GetInstance()  
	{
		static CControlFactory* pInstance = new CControlFactory;
		return pInstance;
	}

	void CControlFactory::Release()
	{
		delete this;
	}
}