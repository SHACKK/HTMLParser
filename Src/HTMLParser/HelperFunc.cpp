#include "pch.h"
#include "HelperFunc.h"

static LPCTSTR s_pszHtmlWhitespace = TEXT(" \r\n\t");

template<typename T>
T __Min(const T& a, const T& b)
{
	if (a < b)
		return a;
	return b;
}

std::tstring EliminateHtmlComment(std::tstring& strContext, const std::tstring& strStartTag, const std::tstring& strEndTag)
{
	std::tstring strRet;

	size_t tPos = 0;
	size_t tIndex = 0;
	while (std::tstring::npos != (tIndex = strContext.find(strStartTag, tPos)))
	{
		size_t tEnd = strContext.find(strEndTag, tIndex + strStartTag.length());
		if (std::tstring::npos == tEnd)
			return strRet;

		strRet += strContext.substr(tPos, tIndex - tPos);
		tPos = tEnd + strEndTag.length();
	}

	if (tPos < strContext.length())
		strRet += strContext.substr(tPos);
	return strRet;
}

bool __ScanHtmlAttributeContext(const std::tstring& strContext, size_t tOffset, std::vector<ST_HTML_TOKENIZED_TAG>& vecTokenizedTag, std::string& refStrErrMsg)
{
	size_t tPos = 0;

	try
	{
		size_t tKeyBeginIndex = 0;
		while (std::tstring::npos != (tKeyBeginIndex = strContext.find_first_not_of(s_pszHtmlWhitespace, tPos)))
		{
			size_t tSepIndex = strContext.find(TEXT("="), tKeyBeginIndex);
			if (std::tstring::npos == tSepIndex)
			{
				if (TEXT("/") == strContext.substr(tKeyBeginIndex))
					break;
#ifdef _DEBUG
				Log_Debug(TEXT("Causing Syntax Error : %s"), strContext.c_str());
#endif
				break;
			}

			size_t tValueBeginIndex = strContext.find_first_not_of(s_pszHtmlWhitespace, tSepIndex + 1);
			if (std::tstring::npos == tValueBeginIndex)
				throw exception_format("HTML attribute parsing failure, seperator after value is not found");

			size_t tValueEndIndex = strContext.find_first_of(s_pszHtmlWhitespace, tValueBeginIndex);
			if (strContext.at(tValueBeginIndex) == TEXT('\''))
				tValueEndIndex = strContext.find(TEXT('\''), tValueBeginIndex + 1) + 1;
			if (strContext.at(tValueBeginIndex) == TEXT('\"'))
				tValueEndIndex = strContext.find(TEXT('\"'), tValueBeginIndex + 1) + 1;
			if (0 == tValueBeginIndex)
				throw exception_format("HTML attribute value close quotaion is not found!");

			tValueEndIndex = __Min(tValueEndIndex, strContext.length());

			ST_HTML_TOKENIZED_TAG stTag;
			stTag.nType = HTML_TOKENIZED_TAG_ATTR_KEY;
			stTag.tIndex = tKeyBeginIndex + tOffset;
			stTag.tLength = tSepIndex + tOffset - stTag.tIndex;
			vecTokenizedTag.push_back(stTag);

			stTag.nType = HTML_TOKENIZED_TAG_ATTR_VALUE;
			stTag.tIndex = tValueBeginIndex + tOffset;
			stTag.tLength = tValueEndIndex + tOffset - stTag.tIndex;
			vecTokenizedTag.push_back(stTag);

			tPos = tValueEndIndex;
		}
	}
	catch (std::exception& e)
	{
		std::string strErrTag = MBSFromTCS(strContext.substr(tPos, 10));
		refStrErrMsg = std::string(e.what()) + "(" + strErrTag + "...)";
		return false;
	}
	return true;
}

bool __ScanHtmlContext(const std::tstring& strContext, std::vector<ST_HTML_TOKENIZED_TAG>& vecTokenizedTag, std::string& refStrErrMsg)
{
	size_t tPos = 0;
	try
	{
		while (std::tstring::npos != (tPos = strContext.find_first_not_of(s_pszHtmlWhitespace, tPos)))
		{
			std::tstring strHead1 = strContext.substr(tPos, 1);
			std::tstring strHead2 = strContext.substr(tPos, 2);

			// tag
			if (TEXT("<") == strHead1)
			{
				size_t tEnd = strContext.find(TEXT(">"), tPos);
				std::tstring strTag = strContext.substr(tPos, tEnd - tPos + 1);

				size_t tTagPos = 0;
				while (std::tstring::npos != (tTagPos = strTag.find_first_not_of(s_pszHtmlWhitespace, tTagPos)))
				{
					// 따옴표가 짝수개면 패스
					int nDoubleQuotationCount = (int)std::count(strTag.begin(), strTag.end(), TEXT('\"'));
					if (!(nDoubleQuotationCount % 2))
						break;

					size_t tAttrStartPos = strTag.find_first_of(s_pszHtmlWhitespace, tTagPos);
					size_t tEqualSignPos = strTag.find(TEXT("="), tAttrStartPos);
					if (std::tstring::npos == tAttrStartPos || std::tstring::npos == tEqualSignPos)
						break;

#ifdef _DEBUG
					std::tstring strKey = strTag.substr(tAttrStartPos, tEqualSignPos - tAttrStartPos);
#endif
					size_t tValueStartPos = strTag.find_first_of(TEXT("\""), tEqualSignPos + 1);
					size_t tValueEndPos = strTag.find_first_of(TEXT("\""), tValueStartPos + 1);

					// value가 닫히지 않았음
					if (std::tstring::npos != tValueStartPos && std::tstring::npos == tValueEndPos)
					{
						tEnd = strContext.find(TEXT(">"), tEnd + 1);
						if (std::tstring::npos == tEnd)
							throw exception_format("HTML tokenizer failure, unrecognized tag has found.");
						tTagPos = 0;
						strTag = strContext.substr(tPos, tEnd - tPos + 1);
						continue;
					}
					tTagPos = tValueEndPos + 1;
				}

				if (std::tstring::npos == tEnd)
					throw exception_format("HTML tokenizer failure, unrecognized tag has found.");

				std::tstring strTagToken = strContext.substr(tPos, tEnd - tPos);
				size_t tWhiteSpaceIndex = strTagToken.find_first_of(s_pszHtmlWhitespace);

				BOOL bSingleTag = (tEnd >= tPos + 1) ? strTagToken.at(tEnd - tPos - 1) == TEXT('/') : false;

				// open tag
				if (TEXT("</") != strHead2)
				{
					if (std::tstring::npos == tWhiteSpaceIndex)
					{
						ST_HTML_TOKENIZED_TAG stTag;
						stTag.nType = bSingleTag ? HTML_TOKENIZED_TAG_SINGLE_ELEMENT : HTML_TOKENIZED_TAG_OPEN_ELEMENT;
						stTag.tIndex = tPos + 1;
						stTag.tLength = tEnd - stTag.tIndex;
						vecTokenizedTag.push_back(stTag);
					}
					else
					{
						ST_HTML_TOKENIZED_TAG stTag;
						stTag.nType = HTML_TOKENIZED_TAG_OPEN_ELEMENT;
						stTag.tIndex = tPos + 1;
						stTag.tLength = tWhiteSpaceIndex + tPos - stTag.tIndex;
						vecTokenizedTag.push_back(stTag);

						//bool bIsSingleElement = false;	// ex) <Element value="123" />
						std::string strErrMsg;
						if (!__ScanHtmlAttributeContext(strTagToken.substr(tWhiteSpaceIndex), tWhiteSpaceIndex + tPos, vecTokenizedTag, strErrMsg))
							throw exception_format("%s", strErrMsg.c_str());

						if (bSingleTag)
						{
							stTag.nType = HTML_TOKENIZED_TAG_CLOSE_ELEMENT;
							vecTokenizedTag.push_back(stTag);
						}

						// script 인 경우 바로 </script>를 찾아준다.
						std::tstring strToken = Trim(strTagToken.c_str());
						std::tstring strTagName = strToken.substr(strToken.find_first_not_of(s_pszHtmlWhitespace), strToken.find_first_of(s_pszHtmlWhitespace));
						if (MakeLower(strTagName.c_str()) == TEXT("<script"))
						{
							tEnd = strContext.find(TEXT("</script>"), tEnd);
							if (std::tstring::npos == tEnd)
								throw exception_format("HTML tokenizer failure, End of script is not found");

							ST_HTML_TOKENIZED_TAG stEndScriptTag;
							stEndScriptTag.nType = HTML_TOKENIZED_TAG_VALUE;
							stEndScriptTag.tIndex = stTag.tIndex + strToken.size();
							stEndScriptTag.tLength = tEnd - stEndScriptTag.tIndex;
							vecTokenizedTag.push_back(stEndScriptTag);
							tEnd -= 1;
						}
					}
				}
				// close tag
				else
				{
					//if (std::tstring::npos != tWhiteSpaceIndex)
					//	throw exception_format("HTML tokenizer failure, close tag cannot contain whitespace.");

					ST_HTML_TOKENIZED_TAG stTag;
					stTag.nType = HTML_TOKENIZED_TAG_CLOSE_ELEMENT;
					stTag.tIndex = tPos + 2;
					stTag.tLength = tEnd - stTag.tIndex;
					vecTokenizedTag.push_back(stTag);
				}

				tPos = tEnd + 1;
			}
			// value
			else
			{
				size_t tEnd = strContext.find(TEXT("</"), tPos);
				if (std::tstring::npos == tEnd)
					throw exception_format("HTML tokenizer failure, end of value is not found");

				ST_HTML_TOKENIZED_TAG stTag;
				stTag.nType = HTML_TOKENIZED_TAG_VALUE;
				stTag.tIndex = tPos;
				stTag.tLength = tEnd - stTag.tIndex;
				vecTokenizedTag.push_back(stTag);
				tPos = tEnd;
			}
		}
	}
	catch (std::exception& e)
	{
		std::string strErrTag = MBSFromTCS(strContext.substr(tPos, 10));
		refStrErrMsg = std::string(e.what()) + "(" + strErrTag + "...)";
		return false;
	}

	return true;
}

std::tstring DecodeHtmlString(std::tstring strContext)
{
	strContext = Replace(strContext, TEXT("&lt;"), TEXT("<"));
	strContext = Replace(strContext, TEXT("&gt;"), TEXT(">"));
	strContext = Replace(strContext, TEXT("&apos;"), TEXT("\'"));
	strContext = Replace(strContext, TEXT("&quot;"), TEXT("\""));
	strContext = Replace(strContext, TEXT("&amp;"), TEXT("&"));
	return strContext;
}

bool __TokenizeHtmlContext(const std::tstring& strContext, const std::vector<ST_HTML_TOKENIZED_TAG> vecTags, ST_HTML_NODE& root, std::string& refStrErrMsg)
{
	const ST_HTML_TOKENIZED_TAG* pCurTag = NULL;
	std::stack<ST_HTML_NODE*> stackTags;
	try
	{
		size_t i;
		for (i = 0; i < vecTags.size(); i++)
		{
			pCurTag = &vecTags[i];
			std::tstring strToken = Trim(strContext.substr(pCurTag->tIndex, pCurTag->tLength).c_str());
			switch (pCurTag->nType)
			{
			case HTML_TOKENIZED_TAG_SINGLE_ELEMENT:
				break;
			case HTML_TOKENIZED_TAG_OPEN_ELEMENT:
				if (stackTags.empty())
					stackTags.push(&root);
				else
				{
					stackTags.top()->vecChild.push_back(ST_HTML_NODE());
					stackTags.push(&stackTags.top()->vecChild.back());
				}
				stackTags.top()->strTagName = strToken;
				break;
			case HTML_TOKENIZED_TAG_CLOSE_ELEMENT:
				if (stackTags.empty())
					throw exception_format("Unmatched close tag has found!");
				stackTags.pop();
				break;
			case HTML_TOKENIZED_TAG_VALUE:
				if (stackTags.empty())
					throw exception_format("No tag value has found!");
				stackTags.top()->strText = DecodeHtmlString(strToken);
				break;
			case HTML_TOKENIZED_TAG_ATTR_KEY:
				if (stackTags.empty())
					throw exception_format("No tag attribute key has found!");
				stackTags.top()->vecAttr.push_back(ST_HTML_ATTR());
				stackTags.top()->vecAttr.back().strKey = strToken;
				break;
			case HTML_TOKENIZED_TAG_ATTR_VALUE:
				if (stackTags.empty())
					throw exception_format("No tag attribute value has found!");
				if (stackTags.top()->vecAttr.empty())
					throw exception_format("No key attribute value has found!");
				stackTags.top()->vecAttr.back().strValue = DecodeHtmlString(StripQuotation(strToken));
				break;

			default:
				throw exception_format("Unrecognized tag type(%d) has found", pCurTag->nType);
			}
		}

		//if (!stackTags.empty())
		//{
		//	pCurTag = NULL;
		//	std::string strOpenTag = MBSFromTCS(stackTags.top()->strTagName);
		//	throw exception_format("Unmatched open tag has found!(%s)", strOpenTag.c_str());
		//}
	}
	catch (std::exception& e)
	{
		if (pCurTag)
		{
			std::string strErrTag = MBSFromTCS(strContext.substr(pCurTag->tIndex, pCurTag->tLength));
			refStrErrMsg = std::string(e.what()) + "(" + strErrTag + "...)";
		}
		else
			refStrErrMsg = e.what();
		return false;
	}
	return true;
}

std::vector<ST_HTML_NODE> ExtractNodeByTagName(std::tstring strTagName, ST_HTML_NODE& stRoot)
{
	std::vector<ST_HTML_NODE> vecRet;
	if (MakeLower(stRoot.strTagName) == MakeLower(strTagName))
		vecRet.push_back(stRoot);

	if (stRoot.vecChild.empty())
		return vecRet;

	for (ST_HTML_NODE& stChildNode : stRoot.vecChild)
	{
		// 재귀 호출
		std::vector<ST_HTML_NODE> vecChildNode = ExtractNodeByTagName(strTagName, stChildNode);
		if (vecChildNode.empty())
			continue;
		for (ST_HTML_NODE& stChildNode : vecChildNode)
			vecRet.push_back(stChildNode);
	}
	return vecRet;
}

std::vector<std::tstring> ExtractScript(ST_HTML_NODE& stRoot)
{
	std::vector<std::tstring> vecRet;
	std::vector<ST_HTML_NODE> vecNodes = ExtractNodeByTagName(TEXT("script"), stRoot);
	for (ST_HTML_NODE& Node : vecNodes)
	{
		if (Node.strText.empty())
			continue;
		vecRet.push_back(Node.strText);
	}
	return vecRet;
}
