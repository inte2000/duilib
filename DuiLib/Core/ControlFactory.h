#ifndef __CONTROL_FACTORY_H__
#define __CONTROL_FACTORY_H__

#pragma once

#include <map>

namespace DuiLib 
{
	typedef CControlUI* (*CreateClass)();
	//typedef std::map<CDuiString, CreateClass> MAP_DUI_CTRATECLASS;

	class DUILIB_API CControlFactory
	{
	public:
		CControlUI* CreateControl(CDuiString strClassName);
		void RegistControl(CDuiString strClassName, CreateClass pFunc);

		static CControlFactory* GetInstance();
		void Release();

	private:	
		CControlFactory();
		virtual ~CControlFactory();

	private:
		CDuiStringPtrMap m_mapControl;
	};

#define DECLARE_DUICONTROL(class_name)\
public:\
	static CControlUI* CreateControl();

#define IMPLEMENT_DUICONTROL(class_name)\
	CControlUI* class_name::CreateControl()\
	{\
		return new class_name;\
	}

#define REGIST_DUICONTROL(class_name)\
	CControlFactory::GetInstance()->RegistControl(_T(#class_name), (CreateClass)class_name::CreateControl);

}

#endif //__CONTROL_FACTORY_H__