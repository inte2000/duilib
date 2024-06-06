#ifndef __UIOPTION_H__
#define __UIOPTION_H__

#pragma once

namespace DuiLib
{
	class DUILIB_API COptionUI : public CButtonUI
	{
		DECLARE_DUICONTROL(COptionUI)
	public:
		COptionUI();
		~COptionUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

		bool Activate();
		void SetEnabled(bool bEnable = true);

		LPCTSTR GetSelectedImage();
		void SetSelectedImage(LPCTSTR pStrImage);

		LPCTSTR GetSelectedHotImage();
		void SetSelectedHotImage(LPCTSTR pStrImage);

		void SetSelectedTextColor(DWORD dwTextColor);
		DWORD GetSelectedTextColor();

		void SetSelectedBkColor(DWORD dwBkColor);
		DWORD GetSelectedBkColor();
		DUI_DEPRECATED DWORD GetSelectBkColor(); // deprecated, use GetSelectedBkColor instead

		LPCTSTR GetGroup() const;
		void SetGroup(LPCTSTR pStrGroupName = NULL);
		bool IsSelected() const;
		virtual void Selected(bool bSelected, bool bTriggerEvent=true);

		SIZE EstimateSize(SIZE szAvailable);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void PaintBkColor(HDC hDC);
		void PaintStatusImage(HDC hDC);
		void PaintText(HDC hDC);

	protected:
		bool			m_bSelected;
		CDuiString		m_sGroupName;

		DWORD			m_dwSelectedBkColor;
		DWORD			m_dwSelectedTextColor;

		TDrawInfo		m_diSelected;
		TDrawInfo		m_diSelectedHot;
	};

} // namespace DuiLib

#endif // __UIOPTION_H__