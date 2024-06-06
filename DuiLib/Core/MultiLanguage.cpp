#include "StdAfx.h"
#include "MultiLanguage.h"

namespace DuiLib 
{
	bool CMultiLanguage::ParseLanguageNode(CMarkupNode& node)
	{
		LPCTSTR lpName = node.GetAttributeValue(_T("name"));
		if (lpName == NULL)
			return false;
		
		m_name = lpName;

		LPCTSTR lpDesc = node.GetAttributeValue(_T("desc"));
		if (lpDesc == NULL)
			return false;

		m_desc = lpDesc;

		LPCTSTR lpAuthor = node.GetAttributeValue(_T("author"));
		if (lpAuthor == NULL)
			return false;

		m_author = lpAuthor;

		return true;
	}

	bool CMultiLanguage::ParseTranslationNode(CMarkupNode& node)
	{
		for (CMarkupNode subnode = node.GetChild(); subnode.IsValid(); subnode = subnode.GetSibling())
		{
			LPCTSTR pstrClass = subnode.GetName();
			if (_tcsicmp(pstrClass, _T("Text")) == 0)
			{
				if (!ParseTextNode(subnode))
					return false;
			}
		}

		return (m_StrHash.GetSize() > 0);
	}

	bool CMultiLanguage::ParseTextNode(CMarkupNode& node)
	{
		LPCTSTR lpSrc = node.GetAttributeValue(_T("src"));
		if (lpSrc == NULL)
			return false;

		LPCTSTR lpValue = node.GetAttributeValue(_T("value"));
		if (lpValue == NULL)
			return false;

		CDuiString* pMultiLanguage = new CDuiString(lpValue);
		if (pMultiLanguage != NULL)
		{
			CDuiString* pOldMultiLanguage = static_cast<CDuiString*>(m_StrHash.Set(lpSrc, (LPVOID)pMultiLanguage));
			if (pOldMultiLanguage) 
				delete pOldMultiLanguage;

			return true;
		}

		return false;
	}

	bool CMultiLanguage::ParseErrorInfoNode(CMarkupNode& node)
	{
		for (CMarkupNode subnode = node.GetChild(); subnode.IsValid(); subnode = subnode.GetSibling())
		{
			LPCTSTR pstrClass = subnode.GetName();
			if (_tcsicmp(pstrClass, _T("Error")) == 0)
			{
				if (!ParseErrorNode(subnode))
					return false;
			}
		}

		return (m_ErrorHash.GetSize() > 0);
	}

	bool CMultiLanguage::ParseErrorNode(CMarkupNode& node)
	{
		LPCTSTR lpCode = node.GetAttributeValue(_T("code"));
		if (lpCode == NULL)
			return false;

		LPCTSTR lpValue = node.GetAttributeValue(_T("value"));
		if (lpValue == NULL)
			return false;

		CDuiString* pErrorMsg = new CDuiString(lpValue);
		if (pErrorMsg != NULL)
		{
			CDuiString* pOldErrorMsg = static_cast<CDuiString*>(m_ErrorHash.Set(lpCode, (LPVOID)pErrorMsg));
			if (pOldErrorMsg)
				delete pOldErrorMsg;

			return true;
		}

		return false;
	}

	void CMultiLanguage::ClearStringMap(CDuiStringPtrMap& strMap)
	{
		for (int i = 0; i < strMap.GetSize(); i++)
		{
			if (LPCTSTR key = strMap.GetAt(i))
			{
				CDuiString* pString = static_cast<CDuiString*>(strMap.Find(key));
				if (pString)
					delete pString;
			}
		}
		strMap.RemoveAll();
	}

	bool CMultiLanguage::LoadLanguage(LPCTSTR lanfile)
	{
		CMarkup xml;
		if (xml.LoadFileOnlyForParse(lanfile))
		{
			CMarkupNode root = xml.GetRoot();
			if (!root.IsValid() || _tcscmp(root.GetName(), _T("Language")) != 0)
				return false;

			if (!ParseLanguageNode(root))
				return false;
			
			for (CMarkupNode node = root.GetChild(); node.IsValid(); node = node.GetSibling())
			{
				LPCTSTR pstrClass = node.GetName();
				if (_tcsicmp(pstrClass, _T("Translation")) == 0)
				{
					if (!ParseTranslationNode(node))
						return false;
				}
				else if (_tcsicmp(pstrClass, _T("ErrorInfo")) == 0)
				{
					if (!ParseErrorInfoNode(node))
						return false;
				}
			}

			return true;
		}

		return false;
	}

	void CMultiLanguage::Reset()
	{
		ClearStringMap(m_StrHash);
		ClearStringMap(m_ErrorHash);
	}

	bool CMultiLanguage::TranslateLanguage(CDuiString& text)
	{
		CDuiString* pMultiLanguage = static_cast<CDuiString*>(m_StrHash.Find(text.GetData()));
		if (pMultiLanguage)
		{
			text = *pMultiLanguage;
			return true;
		}

		return false;
	}

	LPCTSTR CMultiLanguage::TranslateLanguage(LPCTSTR lpText)
	{
		CDuiString* pMultiLanguage = static_cast<CDuiString*>(m_StrHash.Find(lpText));
		if (pMultiLanguage)
		{
			return pMultiLanguage->GetData();
		}

		return NULL;
	}

	LPCTSTR CMultiLanguage::TranslateErrorCode(LPCTSTR lpText)
	{
		CDuiString* pErrorMsg = static_cast<CDuiString*>(m_ErrorHash.Find(lpText));
		if (pErrorMsg)
		{
			return pErrorMsg->GetData();
		}

		return NULL;
	}

}