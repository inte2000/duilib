#ifndef __UILABEL_H__
#define __UILABEL_H__

#pragma once

#ifdef _USE_GDIPLUS
//class DUILIB_API Gdiplus::RectF;
//struct DUILIB_API Gdiplus::GdiplusStartupInput;

using Gdiplus::RectF;
//using Gdiplus::GdiplusStartupInput;
#endif


namespace DuiLib
{
	class DUILIB_API CLabelUI : public CControlUI
	{
		DECLARE_DUICONTROL(CLabelUI)
	public:
		CLabelUI();
		~CLabelUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

        void SetFixedWidth(int cx);
        void SetFixedHeight(int cy);
		void SetText(LPCTSTR pstrText);

		void SetTextStyle(UINT uStyle);
		UINT GetTextStyle() const;
		bool IsMultiLine();
		void SetMultiLine(bool bMultiLine = true);
		void SetTextColor(DWORD dwTextColor);
		DWORD GetTextColor() const;
		void SetDisabledTextColor(DWORD dwTextColor);
		DWORD GetDisabledTextColor() const;
		void SetFont(int index);
		int GetFont() const;
		RECT GetTextPadding() const;
		void SetTextPadding(RECT rc);
		bool IsShowHtml();
		void SetShowHtml(bool bShowHtml = true);

		SIZE EstimateSize(SIZE szAvailable);
		void DoEvent(TEventUI& event);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void PaintText(HDC hDC);

#ifdef _USE_GDIPLUS
		void		SetEnabledEffect(bool _EnabledEffect);
		bool		GetEnabledEffect();
		void		SetEnabledLuminous(bool bEnableLuminous);
		bool		GetEnabledLuminous();
		void		SetLuminousFuzzy(float fFuzzy);
		float		GetLuminousFuzzy();
		void		SetGradientLength(int _GradientLength);
		int			GetGradientLength();
		void		SetShadowOffset(int _offset,int _angle);
		RectF	    GetShadowOffset();
		void		SetTextColor1(DWORD _TextColor1);
		DWORD		GetTextColor1();
		void		SetTextShadowColorA(DWORD _TextShadowColorA);
		DWORD		GetTextShadowColorA();
		void		SetTextShadowColorB(DWORD _TextShadowColorB);
		DWORD		GetTextShadowColorB();
		void		SetStrokeColor(DWORD _StrokeColor);
		DWORD		GetStrokeColor();
		void		SetGradientAngle(int _SetGradientAngle);
		int			GetGradientAngle();
		void		SetEnabledStroke(bool _EnabledStroke);
		bool		GetEnabledStroke();
		void		SetEnabledShadow(bool _EnabledShadowe);
		bool		GetEnabledShadow();
		void		SetTextRenderingAlias(int nTextRenderingAlias);
		Gdiplus::TextRenderingHint GetTextRenderingAlias();
#endif
	
	protected:
		DWORD	m_dwTextColor;
		DWORD	m_dwDisabledTextColor;
		int		m_iFont;
		UINT	m_uTextStyle;
		RECT	m_rcTextPadding;
		bool	m_bShowHtml;
        SIZE    m_szAvailableLast;
        SIZE    m_cxyFixedLast;
        bool    m_bNeedEstimateSize;

#ifdef _USE_GDIPLUS
		float					m_fLuminousFuzzy;
		int						m_GradientLength;
		int						m_GradientAngle;
		bool					m_EnableEffect;
		bool					m_bEnableLuminous;
		bool					m_EnabledStroke;
		bool					m_EnabledShadow;
		DWORD					m_dwTextColor1;
		DWORD					m_dwTextShadowColorA;
		DWORD					m_dwTextShadowColorB;
		DWORD					m_dwStrokeColor;
		RectF					m_ShadowOffset;
		Gdiplus::TextRenderingHint		m_TextRenderingAlias;	//���ֿ���ݼ���
#endif
	};
}

#endif // __UILABEL_H__