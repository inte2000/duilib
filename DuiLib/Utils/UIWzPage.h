#ifndef UI_WZ_PAGE_H
#define UI_WZ_PAGE_H


namespace DuiLib {

	class DUILIB_API UIWzPage : public CContainerUI, public CNotifyPump
	{
	public:
		UIWzPage(CPaintManagerUI* pm, LPCTSTR pageXml);
		virtual ~UIWzPage() {};

		CWzWindowImpl* GetPageOwner() { return m_pParent; }
		CPaintManagerUI* GetPM() { return m_pm; }
		BOOL IsPageActive() const { return m_bPageActive; }
		virtual void PageOnCreate(CWzWindowImpl* pParent);
		virtual void PageOnClose();
		virtual BOOL CheckNextOK() { return TRUE; }
		virtual BOOL PageOnActive() { m_bPageActive = TRUE;  return TRUE; }
		virtual BOOL PageOnDeactive() { m_bPageActive = FALSE; return TRUE; }

	private:
		CPaintManagerUI* m_pm;
		CWzWindowImpl* m_pParent;
		BOOL m_bPageActive;
	};

}
#endif // UI_WZ_PAGE_H
