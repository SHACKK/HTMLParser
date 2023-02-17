#include "pch.h"
#include "HTMLParser.h"

HTMLParser::HTMLParser()
	: m_hFile(NULL)
	, m_hMap(NULL)
	, m_strFileName()
	, m_strContext()
	, m_stRoot()
	, m_pszContext()
	, m_tContextSize()
{
}

HTMLParser::~HTMLParser()
{
	Close();
}

bool HTMLParser::Open(std::tstring strHtmlFile, std::tstring& strContext)
{
	try
	{
		m_hFile = CreateFile(strHtmlFile.c_str(), GENERIC_READ_, OPEN_EXISTING_, 0);
		if(NULL == m_hFile)
			throw exception_format(TEXT("CreateFile(%s) failure, %d"), strHtmlFile.c_str(), GetLastError());

		m_tContextSize = (size_t)GetFileSize(m_hFile);
		if (m_tContextSize == 0)
			throw exception_format(TEXT("File size is 0, %s"), strHtmlFile.c_str());

		m_hMap = CreateFileMapping(m_hFile, PAGE_READONLY_, FILE_MAP_READ_, m_tContextSize);
		if (NULL == m_hMap)
			throw exception_format(TEXT("CreateFileMapping(%s) failure, %d"), strHtmlFile.c_str(), GetLastError());

		m_pszContext = (LPSTR)MapViewOfFile(m_hMap, 0, m_tContextSize);
		if (IsInvalidUTF8(m_pszContext, m_tContextSize))
			strContext = TCSFromMBS(m_pszContext);
		else
			strContext = TCSFromUTF8(m_pszContext);

		if (strContext.empty())
			throw exception_format(TEXT("Context Empty"));
	}
	catch (const std::exception& e)
	{
		Close();
		return false;
	}
	return true;
}

void HTMLParser::Close()
{
	if (m_hMap)
	{
		if (m_pszContext)
			UnmapViewOfFile(m_hMap, m_pszContext);

		CloseFileMappingHandle(m_hMap);
		m_hMap = NULL;
	}

	if (m_hFile)
	{
		CloseFile(m_hFile);
		m_hFile = NULL;
	}

	m_pszContext = NULL;
}

std::tstring EliminateComment(const std::tstring strContext, const std::tstring& strStartTag, const std::tstring& strEndTag)
{
	std::tstring strRet;

	size_t tPos = 0;
	size_t tStartTagPos = 0;
	size_t tEndTagPos = 0;

	while (std::tstring::npos != (tStartTagPos = strContext.find(strStartTag, tPos)))
	{
		tEndTagPos = strContext.find(strEndTag, tStartTagPos + strStartTag.length());
		if (std::tstring::npos == tEndTagPos)
			return strRet;

		strRet += strContext.substr(tPos, tStartTagPos - tPos);
		tPos = tEndTagPos + strEndTag.length();
	}

	if (tPos < strContext.length())
		strRet += strContext.substr(tPos);

	return strRet;
}

std::tstring MakeAttributeFormally(const std::tstring& strAttribute)
{
	std::tstring strRet;
	
	return strRet;
}

ECODE HTMLParser::ParseFromMemory(std::tstring& strContext)
{
	if (strContext.empty())
		return EC_NO_DATA;

	std::tstring strCurContext = EliminateComment(strContext, TEXT("<!--"), TEXT("-->"));

	size_t tCurPos = 0;
	size_t tCompletePos = 0;

	while (std::tstring::npos != (tCurPos = strContext.find_first_not_of(TEXT("\n\r\t"), tCurPos)))
	{
		std::tstring strHead1 = strCurContext.substr(tCurPos, 1);
		std::tstring strHead2 = strCurContext.substr(tCurPos, 2);

		// tag
		if (strHead1 == TEXT("<"))
		{
			size_t tEndTagPos =	strCurContext.find(TEXT(">"), tCurPos);
			std::tstring strTag = strCurContext.substr(tCurPos, tEndTagPos - tCurPos + 1);

			// OpenTag
			if (strHead2 != TEXT("</"))
			{
				size_t tEndTagPos = strCurContext.find(TEXT(">"), tCurPos);
				std::tstring strTag = strCurContext.substr(tCurPos, tEndTagPos - tCurPos);
			}
			// EndTag
			else
			{
				dd
			}
		}
		// value
		else
		{
			// ST_HTML_NODE& stNode = FindFirstIncompleteNode();
		}
		

	}
}

ECODE HTMLParser::Parse(const std::tstring& strHtmlFile)
{
	m_strFileName = strHtmlFile;
	ECODE nRet = EC_SUCCESS;
	try
	{
		if (!Open(m_strFileName, m_strContext))
			throw exception_format(TEXT("Open(%s) failure"), strHtmlFile.c_str());

		nRet = ParseFromMemory(m_strContext);
		if (EC_SUCCESS != nRet)
			throw exception_format(TEXT("ParseFromMemory() failure, %d"), nRet);
	}
	catch (const std::exception& e)
	{
		Log_Error("%s", e.what());
		return nRet;
	}
	return EC_SUCCESS;
}

ST_HTML_NODE& HTMLParser::GetRoot()
{
    return m_stRoot;
}
