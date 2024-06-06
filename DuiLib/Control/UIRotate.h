#ifndef __UIROTATE_H__
#define __UIROTATE_H__

#pragma once

namespace DuiLib
{
	class CRotateUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CRotateUI)
	public:
		CRotateUI();
		virtual ~CRotateUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetBkImage(LPCTSTR pStrImage);	
		virtual void DoEvent(TEventUI& event);
		virtual void PaintBkImage(HDC hDC);	
	private:
		enum
		{ 
			RING_TIMERID = 100,
		};
		void InitImage(LPCTSTR pStrImage);
		void DeleteImage();

	public:
		CDuiString m_strImgName;
		float m_fCurAngle;
		Gdiplus::Image* m_pBkimage;
	};
}

#endif // __UIROTATE_H__