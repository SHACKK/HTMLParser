#include "pch.h"
#include "HtmlDeserializer.h"
#include "HelperFunc.h"

namespace fmt_internal
{
	//////////////////////////////////////////////////////////////////////////
	CHTMLDeserializer::CHTMLDeserializer(core::IChannel& channel)
		: IFormatter(channel)
		, m_vecObjectCountStack()
		, m_stackTraverse()
		, m_bValidity(false)
		, m_strErrMsg()
		, m_stRoot()
	{
		if (!(m_bValidity = channel.CheckValidity(m_strErrMsg)))
			return;

		std::tstring strContext;
		while (1)
		{
			const size_t tTokenSize = 512;
			TCHAR szBuff[tTokenSize + 1] = { 0, };
			size_t tReadSize = channel.Access(szBuff, sizeof(TCHAR) * tTokenSize);
			if (0 == tReadSize)
				break;

			strContext += szBuff;
		}

		m_bValidity = fmt_internal::ParseHtmlContext(strContext, m_stRoot, m_strErrMsg);
	}

	//////////////////////////////////////////////////////////////////////////
	CHTMLDeserializer::~CHTMLDeserializer(void)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CHTMLDeserializer::BeginDictionary(std::tstring& strKey, const size_t tSize, bool bAllowMultiKey)
	{
		ST_HTML_NODE* pNode = &m_stDummy;

		if (m_stackTraverse.top()->strKey == strKey)
		{
			pNode = m_stackTraverse.top();
		}
		else
		{
			size_t i;
			for (i = 0; i < m_stackTraverse.top()->vecChild.size(); i++)
			{
				if (m_stackTraverse.top()->vecChild[i].strKey != strKey)
					continue;

				pNode = &m_stackTraverse.top()->vecChild[i];
				break;
			}
		}

		sGroupingData stGroupingData(GT_DICTIONARY, pNode->vecChild.size());
		m_vecObjectCountStack.push_back(stGroupingData);
		m_stackTraverse.push(pNode);
		return pNode->vecChild.size();
	}

	//////////////////////////////////////////////////////////////////////////
	void CHTMLDeserializer::EndDictionary()
	{
		m_vecObjectCountStack.pop_back();
		m_stackTraverse.pop();
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CHTMLDeserializer::BeginArray(std::tstring& strKey, const size_t tSize)
	{
		size_t tCount = 0;
		size_t i;
		for (i = 0; i < m_stackTraverse.top()->vecChild.size(); i++)
		{
			if (m_stackTraverse.top()->vecChild[i].strKey != strKey)
				continue;
			tCount++;
		}

		sGroupingData stGroupingData(GT_ARRAY, tCount);
		m_vecObjectCountStack.push_back(stGroupingData);
		return tCount;
	}

	//////////////////////////////////////////////////////////////////////////
	void CHTMLDeserializer::EndArray()
	{
		m_vecObjectCountStack.pop_back();
	}

	//////////////////////////////////////////////////////////////////////////
	void CHTMLDeserializer::BeginObject(std::tstring& strKey)
	{
		sGroupingData stGroupingData(GT_OBJECT, 0xFFFFFFFF);
		m_vecObjectCountStack.push_back(stGroupingData);

		sGroupingData& refParentGroupData = m_vecObjectCountStack[m_vecObjectCountStack.size() - 2];

		ST_HTML_NODE* pCurNode = m_stackTraverse.top();

		size_t tFoundIndex = 0;
		size_t i;
		for (i = 0; i < pCurNode->vecChild.size(); i++)
		{
			if ((GT_DICTIONARY == refParentGroupData.nType))
			{
				if (tFoundIndex < refParentGroupData.tSequence)
				{
					tFoundIndex++;
					continue;
				}

				strKey = pCurNode->vecChild[i].strKey;
			}
			else
			{
				if (pCurNode->vecChild[i].strKey != strKey)
					continue;

				if ((GT_ARRAY == refParentGroupData.nType) && (tFoundIndex < refParentGroupData.tSequence))
				{
					tFoundIndex++;
					continue;
				}
			}

			refParentGroupData.tSequence++;
			m_stackTraverse.push(&pCurNode->vecChild[i]);
			return;
		}

		m_stackTraverse.push(&m_stDummy);
	}

	//////////////////////////////////////////////////////////////////////////
	void CHTMLDeserializer::EndObject()
	{
		m_vecObjectCountStack.pop_back();
		m_stackTraverse.pop();
	}

	//////////////////////////////////////////////////////////////////////////
	void CHTMLDeserializer::BeginRoot()
	{
		sGroupingData stGroupingData(GT_ROOT, 0xFFFFFFFF);
		m_vecObjectCountStack.push_back(stGroupingData);
		m_stackTraverse.push(&m_stRoot);
	}

	//////////////////////////////////////////////////////////////////////////
	void CHTMLDeserializer::EndRoot()
	{
		m_vecObjectCountStack.pop_back();
		m_stackTraverse.pop();
	}

	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	static inline void __HTMLDeserializerSync(const std::stack<ST_HTML_NODE*>& stackTraverse, std::vector<sGroupingData>& vecGroupingData, std::tstring& strKey, T* pValue)
	{
		sGroupingData& refGroupData = vecGroupingData.back();
		if (refGroupData.tSequence >= refGroupData.tTotalCount)
			return;

		size_t tIndex = refGroupData.tSequence++;
		size_t tFoundIndex = 0;

		if (GT_DICTIONARY == refGroupData.nType)
		{
			const std::vector<ST_HTML_NODE>& vecChildren = stackTraverse.top()->vecChild;
			strKey = vecChildren[tIndex].strKey;
			*pValue = ValueFrom<T>(vecChildren[tIndex].strValue);
			return;
		}

		{	// Search on children
			const std::vector<ST_HTML_NODE>& vecChildren = stackTraverse.top()->vecChild;

			size_t i;
			for (i = 0; i < vecChildren.size(); i++)
			{
				if (vecChildren[i].strKey != strKey)
					continue;

				if (GT_ARRAY == refGroupData.nType && tIndex > tFoundIndex++)
					continue;

				*pValue = ValueFrom<T>(vecChildren[i].strValue);
				return;
			}
		}

		{	// Search on attributes
			const std::vector<ST_HTML_ATTR>& vecAttrs = stackTraverse.top()->vecAttr;

			size_t i;
			for (i = 0; i < vecAttrs.size(); i++)
			{
				if (vecAttrs[i].strKey != strKey)
					continue;

				if (GT_ARRAY == refGroupData.nType && tIndex > tFoundIndex++)
					continue;

				*pValue = ValueFrom<T>(vecAttrs[i].strValue);
				return;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, std::tstring* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, std::ntstring* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, bool* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, char* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, short* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, int32_t* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, int64_t* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, BYTE* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, WORD* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, DWORD* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, QWORD* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, float* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, double* pValue)
	{
		__HTMLDeserializerSync(m_stackTraverse, m_vecObjectCountStack, strKey, pValue);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	core::IFormatterT& CHTMLDeserializer::Sync(std::tstring& strKey, std::vector<BYTE>* pvecData)
	{
		// Ignore
		return *this;
	}
}