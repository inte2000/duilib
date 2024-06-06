#ifndef __MULTI_LANGUAGE_H__
#define __MULTI_LANGUAGE_H__

#pragma once

#include "UIMarkup.h"

namespace DuiLib 
{
	class DUILIB_API CMultiLanguage
	{
	public:
		CMultiLanguage() {}
		virtual ~CMultiLanguage() { Reset(); }

		CDuiString GeLanguagetName() const { return m_name; }
		bool LoadLanguage(LPCTSTR lanfile);
		bool TranslateLanguage(CDuiString& text);
		LPCTSTR TranslateLanguage(LPCTSTR lpText);
		LPCTSTR TranslateErrorCode(LPCTSTR lpText);
		void Reset();
	protected:
		bool ParseLanguageNode(CMarkupNode& node);
		bool ParseTranslationNode(CMarkupNode& node);
		bool ParseTextNode(CMarkupNode& node);
		bool ParseErrorInfoNode(CMarkupNode& node);
		bool ParseErrorNode(CMarkupNode& node);
		void ClearStringMap(CDuiStringPtrMap& strMap);
	private:
		//std::map<CDuiString, CDuiString> m_mapControl;
		CDuiString m_name;
		CDuiString m_desc;
		CDuiString m_author;
		CDuiStringPtrMap m_StrHash;
		CDuiStringPtrMap m_ErrorHash;
	};


}

#endif //__MULTI_LANGUAGE_H__