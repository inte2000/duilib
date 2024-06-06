#include "stdafx.h"
#include "SkinManager.h"

namespace DuiLib {

    CSkinManager CSkinManager::m_SkinManager;
	
	CSkinManager* CSkinManager::GetSkinManager()
	{
		return &m_SkinManager;
	}

}