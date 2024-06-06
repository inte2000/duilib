#ifndef __SKIN_MANAGER_H__
#define __SKIN_MANAGER_H__

#include "..\Utils\observer_impl_base.h"

namespace DuiLib {

	const UINT SKIN_NOTIFY_LANG = 0x00000001;
	const UINT SKIN_NOTIFY_SKIN = 0x00000002;
	const UINT SKIN_NOTIFY_BKCOLOR = 0x00000004;
	const UINT SKIN_NOTIFY_BKIMAGE = 0x00000008;
	const UINT SKIN_NOTIFY_OPACITY = 0x00000010;

	struct DUILIB_API SkinChangedParam
	{
		UINT type;
		DWORD bkcolor;
		CDuiString skinname;
		CDuiString bgimage;
		CDuiString langfile;
		BYTE opacity;
	};

	class DUILIB_API CSkinChangedObserver : public ObserverImpl<BOOL, SkinChangedParam>
	{
	};

	class DUILIB_API CSkinChangedReceiver : public ReceiverImpl<BOOL, SkinChangedParam>
	{
	};

	class DUILIB_API CSkinManager
	{
		friend class CPaintManagerUI;
	public:
		static CSkinManager* GetSkinManager();

	public:
		void NotifyLanguageChanged(LPCTSTR lpFileName)
		{
			SkinChangedParam param;
			param.type = SKIN_NOTIFY_LANG;
			param.langfile = lpFileName;
			
			Broadcast(param);
		}
		void NotifyBkColorChanged(DWORD dwColor)
		{
			SkinChangedParam param;
			param.type = SKIN_NOTIFY_BKCOLOR;
			param.bkcolor = dwColor;

			Broadcast(param);
		}
		void NotifyBkImageChanged(LPCTSTR lpImgName)
		{
			SkinChangedParam param;
			param.type = SKIN_NOTIFY_BKIMAGE;
			param.bgimage = lpImgName;

			Broadcast(param);
		}
		void NotifyOpacityChanged(BYTE opacity)
		{
			SkinChangedParam param;
			param.type = SKIN_NOTIFY_OPACITY;
			param.opacity = opacity;

			Broadcast(param);
		}
		void NotifySkinChanged(LPCTSTR lpSkinName)
		{
			SkinChangedParam param;
			param.type = SKIN_NOTIFY_SKIN;
			param.skinname = lpSkinName;

			Broadcast(param);
		}

		void Broadcast(SkinChangedParam param)
		{
			m_SkinChangeObserver.Broadcast(param);
		}
		/*
		void Notify(SkinChangedParam param)
		{
			m_SkinChangeObserver.Notify(param);
		}
		*/
	private:
		CSkinManager(){}
		void AddReceiver(CSkinChangedReceiver* receiver)
		{
			m_SkinChangeObserver.AddReceiver(receiver);
		}

		void RemoveReceiver(CSkinChangedReceiver* receiver)
		{
			m_SkinChangeObserver.RemoveReceiver(receiver);
		}

	private:
		CSkinChangedObserver m_SkinChangeObserver;
		static CSkinManager m_SkinManager;
	};

} // namespace DuiLib

#endif // __SKIN_MANAGER_H__