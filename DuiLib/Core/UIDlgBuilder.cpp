#include "StdAfx.h"

namespace DuiLib {

CDialogBuilder::CDialogBuilder() : m_pCallback(NULL), m_pstrtype(NULL)
{

}

CControlUI* CDialogBuilder::Create(STRINGorID xml, LPCTSTR type, IDialogBuilderCallback* pCallback, 
                                   CPaintManagerUI* pManager, CControlUI* pParent)
{
	//资源ID为0-65535，两个字节；字符串指针为4个字节
	//字符串以<开头认为是XML字符串，否则认为是XML文件

    if( HIWORD(xml.m_lpstr) != NULL ) {
        if( *(xml.m_lpstr) == _T('<') ) {
            if( !m_xml.Load(xml.m_lpstr) ) return NULL;
        }
        else {
            if( !m_xml.LoadFromFile(xml.m_lpstr) ) return NULL;
        }
    }
    else {
        HRSRC hResource = ::FindResource(CPaintManagerUI::GetResourceDll(), xml.m_lpstr, type);
        if( hResource == NULL ) return NULL;
        HGLOBAL hGlobal = ::LoadResource(CPaintManagerUI::GetResourceDll(), hResource);
        if( hGlobal == NULL ) {
            FreeResource(hResource);
            return NULL;
        }

        m_pCallback = pCallback;
        if( !m_xml.LoadFromMem((BYTE*)::LockResource(hGlobal), ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource) )) return NULL;
        ::FreeResource(hResource);
        m_pstrtype = type;
    }

    return Create(pCallback, pManager, pParent);
}

CControlUI* CDialogBuilder::Create(IDialogBuilderCallback* pCallback, CPaintManagerUI* pManager, CControlUI* pParent)
{
    m_pCallback = pCallback;
    CMarkupNode root = m_xml.GetRoot();
    if( !root.IsValid() ) return NULL;

    if( pManager ) {
        LPCTSTR pstrClass = NULL;
        int nAttributes = 0;
        LPCTSTR pstrName = NULL;
        LPCTSTR pstrValue = NULL;
        LPTSTR pstr = NULL;
        for( CMarkupNode node = root.GetChild() ; node.IsValid(); node = node.GetSibling() ) {
            pstrClass = node.GetName();
            if( _tcsicmp(pstrClass, _T("Image")) == 0 ) {
                nAttributes = node.GetAttributeCount();
                LPCTSTR pImageName = NULL;
                LPCTSTR pImageResType = NULL;
                DWORD mask = 0;
				bool shared = false;
                for( int i = 0; i < nAttributes; i++ ) {
                    pstrName = node.GetAttributeName(i);
                    pstrValue = node.GetAttributeValue(i);
                    if( _tcsicmp(pstrName, _T("name")) == 0 ) {
                        pImageName = pstrValue;
                    }
                    else if( _tcsicmp(pstrName, _T("restype")) == 0 ) {
                        pImageResType = pstrValue;
                    }
                    else if( _tcsicmp(pstrName, _T("mask")) == 0 ) {
                        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
                        mask = _tcstoul(pstrValue, &pstr, 16);
                    }
					else if( _tcsicmp(pstrName, _T("shared")) == 0 ) {
						shared = (_tcsicmp(pstrValue, _T("true")) == 0);
					}
                }
                if( pImageName ) pManager->AddImage(pImageName, pImageResType, mask, shared);
            }
            else if( _tcsicmp(pstrClass, _T("Font")) == 0 ) {
                nAttributes = node.GetAttributeCount();
				int id = -1;
                LPCTSTR pFontName = NULL;
                int size = 12;
                bool bold = false;
                bool underline = false;
                bool italic = false;
                bool defaultfont = false;
				bool shared = false;
                for( int i = 0; i < nAttributes; i++ ) {
                    pstrName = node.GetAttributeName(i);
                    pstrValue = node.GetAttributeValue(i);
					if( _tcsicmp(pstrName, _T("id")) == 0 ) {
						id = _tcstol(pstrValue, &pstr, 10);
					}
                    else if( _tcsicmp(pstrName, _T("name")) == 0 ) {
                        pFontName = pstrValue;
                    }
                    else if( _tcsicmp(pstrName, _T("size")) == 0 ) {
                        size = _tcstol(pstrValue, &pstr, 10);
                    }
                    else if( _tcsicmp(pstrName, _T("bold")) == 0 ) {
                        bold = (_tcsicmp(pstrValue, _T("true")) == 0);
                    }
                    else if( _tcsicmp(pstrName, _T("underline")) == 0 ) {
                        underline = (_tcsicmp(pstrValue, _T("true")) == 0);
                    }
                    else if( _tcsicmp(pstrName, _T("italic")) == 0 ) {
                        italic = (_tcsicmp(pstrValue, _T("true")) == 0);
                    }
                    else if( _tcsicmp(pstrName, _T("default")) == 0 ) {
                        defaultfont = (_tcsicmp(pstrValue, _T("true")) == 0);
                    }
					else if( _tcsicmp(pstrName, _T("shared")) == 0 ) {
						shared = (_tcsicmp(pstrValue, _T("true")) == 0);
					}
                }
                if( id >= 0 && pFontName ) {
                    pManager->AddFont(id, pFontName, size, bold, underline, italic, shared);
                    if( defaultfont ) pManager->SetDefaultFont(pFontName, size, bold, underline, italic, shared);
                }
            }
            else if( _tcsicmp(pstrClass, _T("Default")) == 0 ) {
                nAttributes = node.GetAttributeCount();
                LPCTSTR pControlName = NULL;
                LPCTSTR pControlValue = NULL;
				bool shared = false;
                for( int i = 0; i < nAttributes; i++ ) {
                    pstrName = node.GetAttributeName(i);
                    pstrValue = node.GetAttributeValue(i);
                    if( _tcsicmp(pstrName, _T("name")) == 0 ) {
                        pControlName = pstrValue;
                    }
                    else if( _tcsicmp(pstrName, _T("value")) == 0 ) {
                        pControlValue = pstrValue;
                    }
					else if( _tcsicmp(pstrName, _T("shared")) == 0 ) {
						shared = (_tcsicmp(pstrValue, _T("true")) == 0);
					}
                }
                if( pControlName ) {
                    pManager->AddDefaultAttributeList(pControlName, pControlValue, shared);
                }
            }
        }

        pstrClass = root.GetName();
        if( _tcsicmp(pstrClass, _T("Window")) == 0 ) {
            if( pManager->GetPaintWindow() ) {
                int nAttributes = root.GetAttributeCount();
                for( int i = 0; i < nAttributes; i++ ) {
                    pstrName = root.GetAttributeName(i);
                    pstrValue = root.GetAttributeValue(i);
                    pManager->SetWindowAttribute(pstrName, pstrValue);
                }
            }
        }
    }
    return _Parse(&root, pParent, pManager);
}

CMarkup* CDialogBuilder::GetMarkup()
{
    return &m_xml;
}

void CDialogBuilder::GetLastErrorMessage(LPTSTR pstrMessage, SIZE_T cchMax) const
{
    return m_xml.GetLastErrorMessage(pstrMessage, cchMax);
}

void CDialogBuilder::GetLastErrorLocation(LPTSTR pstrSource, SIZE_T cchMax) const
{
    return m_xml.GetLastErrorLocation(pstrSource, cchMax);
}

CControlUI* CDialogBuilder::_Parse(CMarkupNode* pRoot, CControlUI* pParent, CPaintManagerUI* pManager)
{
    IContainerUI* pContainer = NULL;
    CControlUI* pReturn = NULL;
    for( CMarkupNode node = pRoot->GetChild() ; node.IsValid(); node = node.GetSibling() ) {
        LPCTSTR pstrClass = node.GetName();
        if( _tcsicmp(pstrClass, _T("Image")) == 0 || _tcsicmp(pstrClass, _T("Font")) == 0 \
            || _tcsicmp(pstrClass, _T("Default")) == 0  ) continue;

        CControlUI* pControl = NULL;
        if( _tcsicmp(pstrClass, _T("Include")) == 0 ) {
            if( !node.HasAttributes() ) continue;
            int count = 1;
            LPTSTR pstr = NULL;
            TCHAR szValue[500] = { 0 };
            SIZE_T cchLen = lengthof(szValue) - 1;
            if ( node.GetAttributeValue(_T("count"), szValue, cchLen) )
                count = _tcstol(szValue, &pstr, 10);
            cchLen = lengthof(szValue) - 1;
            if ( !node.GetAttributeValue(_T("source"), szValue, cchLen) ) continue;
			CDialogBuilder builder;
			for ( int i = 0; i < count; i++ ) {
				if (!builder.GetMarkup()->IsValid())
				{
					if( m_pstrtype != NULL ) { // 使用资源dll，从资源中读取
						WORD id = (WORD)_tcstol(szValue, &pstr, 10); 
						pControl = builder.Create((UINT)id, m_pstrtype, m_pCallback, pManager, pParent);
					}
					else 
						pControl = builder.Create((LPCTSTR)szValue, NULL, m_pCallback, pManager, pParent);
				}
				else
					pControl = builder.Create(m_pCallback, pManager, pParent);
			}
            continue;
        }
		//树控件XML解析
		else if( _tcsicmp(pstrClass, _T("TreeNode")) == 0 ) {
			CTreeNodeUI* pParentNode	= static_cast<CTreeNodeUI*>(pParent->GetInterface(_T("TreeNode")));
			CTreeNodeUI* pNode			= new CTreeNodeUI();
			if(pParentNode){
				if(!pParentNode->Add(pNode)){
					delete pNode;
					continue;
				}
			}

            TCHAR szValue[500] = { 0 };
            SIZE_T cchLen = lengthof(szValue) - 1;
			// 若有控件默认配置先初始化默认属性
			if( pManager ) {
				pNode->SetManager(pManager, NULL, false);
                //orbit 20230309 添加 style 支持
                // 如果有 style ，优先选择 style 指定的样式，没有的话默认使用 class 指定的样式
                if (node.GetAttributeValue(_T("style"), szValue, cchLen)) {
                    LPCTSTR pStyleAttributes = pManager->GetDefaultAttributeList(szValue);
                    if (pStyleAttributes) {
                        pControl->SetAttributeList(pStyleAttributes);
                    }
                }
                else {
                    LPCTSTR pDefaultAttributes = pManager->GetDefaultAttributeList(pstrClass);
                    if (pDefaultAttributes) {
                        pControl->SetAttributeList(pDefaultAttributes);
                    }
                }
            }

			// 解析所有属性并覆盖默认属性
			if( node.HasAttributes() ) {
				// Set ordinary attributes
				int nAttributes = node.GetAttributeCount();
				for( int i = 0; i < nAttributes; i++ ) {
					pNode->SetAttribute(node.GetAttributeName(i), node.GetAttributeValue(i));
				}
			}

			//检索子节点及附加控件
			if(node.HasChildren()){
				CControlUI* pSubControl = _Parse(&node,pNode,pManager);
				if(pSubControl && _tcsicmp(pSubControl->GetClass(),_T("TreeNodeUI")) != 0)
				{
					// 					pSubControl->SetFixedWidth(30);
					// 					CHorizontalLayoutUI* pHorz = pNode->GetTreeNodeHoriznotal();
					// 					pHorz->Add(new CEditUI());
					// 					continue;
				}
			}

			if(!pParentNode){
				CTreeViewUI* pTreeView = static_cast<CTreeViewUI*>(pParent->GetInterface(_T("TreeView")));
				ASSERT(pTreeView);
				if( pTreeView == NULL ) return NULL;
				if( !pTreeView->Add(pNode) ) {
					delete pNode;
					continue;
				}
			}
			continue;
		}
        else {
#ifdef _DEBUG
			DUITRACE(_T("Create Control: %s"), pstrClass);
#endif
            CDuiString strClass;
            strClass.Format(_T("C%sUI"), pstrClass);
            pControl = dynamic_cast<CControlUI*>(CControlFactory::GetInstance()->CreateControl(strClass));

            // User-supplied control factory
            if( pControl == NULL ) {
                CDuiPtrArray* pPlugins = CPaintManagerUI::GetPlugins();
                LPCREATECONTROL lpCreateControl = NULL;
                for( int i = 0; i < pPlugins->GetSize(); ++i ) {
                    lpCreateControl = (LPCREATECONTROL)pPlugins->GetAt(i);
                    if( lpCreateControl != NULL ) {
                        pControl = lpCreateControl(pstrClass);
                        if( pControl != NULL ) break;
                    }
                }
            }
            //通过回调创建
            if( pControl == NULL && m_pCallback != NULL ) {
                pControl = m_pCallback->CreateControl(pstrClass);
            }
        }

#ifndef _DEBUG
        ASSERT(pControl);
#endif // _DEBUG
			if( pControl == NULL )
			{
#ifdef _DEBUG
				DUITRACE(_T("Unknow Control:%s"),pstrClass);
#endif
				continue;
			}

        // Add children
        if( node.HasChildren() ) {
            _Parse(&node, pControl, pManager);
        }
        TCHAR szValue[256] = { 0 };
        int cchLen = lengthof(szValue) - 1;
        // Attach to parent
        // 因为某些属性和父窗口相关，比如selected，必须先Add到父窗口
		if( pParent != NULL ) {
            LPCTSTR lpValue = szValue;
            if( node.GetAttributeValue(_T("cover"), szValue, cchLen) && _tcscmp(lpValue, _T("true")) == 0 ) {
                pParent->SetCover(pControl);
            }
            else {
                CTreeNodeUI* pContainerNode = static_cast<CTreeNodeUI*>(pParent->GetInterface(DUI_CTR_TREENODE));
                if(pContainerNode)
                    pContainerNode->GetTreeNodeHoriznotal()->Add(pControl);
                else
                {
                    if( pContainer == NULL ) pContainer = static_cast<IContainerUI*>(pParent->GetInterface(DUI_CTR_ICONTAINER));
                    ASSERT(pContainer);
                    if( pContainer == NULL ) return NULL;
                    if( !pContainer->Add(pControl) ) {
                        pControl->Delete();
                        continue;
                    }
                }
            }
		}
        // Init default attributes
        if( pManager ) {
            pControl->SetManager(pManager, NULL, false);
            //orbit 20230309 添加 style 支持
            // 如果有 style ，优先选择 style 指定的样式，没有的话默认使用 class 指定的样式
            if (node.GetAttributeValue(_T("style"), szValue, cchLen))  {
                LPCTSTR pStyleAttributes = pManager->GetDefaultAttributeList(szValue);
                    if (pStyleAttributes) {
                        pControl->SetAttributeList(pStyleAttributes);
                    }
            }
            else  {
                LPCTSTR pDefaultAttributes = pManager->GetDefaultAttributeList(pstrClass);
                if( pDefaultAttributes ) {
                    pControl->SetAttributeList(pDefaultAttributes);
                }
            }
        }
        // Process attributes
        if( node.HasAttributes() ) {
            // Set ordinary attributes
            int nAttributes = node.GetAttributeCount();
            for( int i = 0; i < nAttributes; i++ ) {
                pControl->SetAttribute(node.GetAttributeName(i), node.GetAttributeValue(i));
            }
        }
        if( pManager ) {
            pControl->SetManager(NULL, NULL, false);
        }
        // Return first item
        if( pReturn == NULL ) pReturn = pControl;
    }
    return pReturn;
}

} // namespace DuiLib
