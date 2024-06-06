#include "stdafx.h"
#include "UIWzDialogImp.h"

namespace DuiLib
{
	//const TCHAR* DuiDialogOkButtonName = _T("btn_ok");
	//const TCHAR* DuiDialogCancelButtonName = _T("btn_cancel");

	UINT_PTR CWzDialogImpl::DoModal(HWND hWnd)
	{
		Create(hWnd, _T(""), UI_WNDSTYLE_DIALOG,  0, 0, 0, 0, 0, NULL);
		CenterWindow();

		return ShowModal();
	}

	LRESULT CWzDialogImpl::ResponseDefaultKeyEvent(WPARAM wParam)
	{
		if (wParam == VK_RETURN)
		{
			OnOkButtonClick();
			return TRUE;
		}
		else if (wParam == VK_ESCAPE)
		{
			OnCancelButtonClick();
			return TRUE;
		}

		return FALSE;
	}

	void CWzDialogImpl::OnOkButtonClick()
	{
		Close(IDOK);
	}

	void CWzDialogImpl::OnCancelButtonClick()
	{
		OnSysCloseButtonClick();
	}

}