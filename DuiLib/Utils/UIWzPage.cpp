#include "stdafx.h"
#include "UIWzPage.h"

namespace DuiLib
{
	UIWzPage::UIWzPage(CPaintManagerUI* pm, LPCTSTR pageXml) : m_pm(pm), m_bPageActive(FALSE)
	{
		m_pParent = NULL;

		CDialogBuilder builder;
		CContainerUI* pControl = static_cast<CContainerUI*>(builder.Create(pageXml, NULL, NULL, pm));
		if (pControl) {
			this->Add(pControl);
		}
		else
		{
			this->RemoveAll();
		}
	}

	void UIWzPage::PageOnCreate(CWzWindowImpl* pParent)
	{
		m_pParent = pParent;
		CDuiString virtualWndName = GetVirtualWnd();
		if (!virtualWndName.IsEmpty())
		{
			pParent->AddVirtualWnd(virtualWndName, this);
		}
	}

	void UIWzPage::PageOnClose()
	{
		if (m_pParent != NULL)
		{
			m_pParent->RemoveVirtualWnd(GetVirtualWnd());
		}
	}

}