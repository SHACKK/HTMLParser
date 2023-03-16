#include "pch.h"
#include "HtmlParser.h"
#include "HelperFunc.h"

CHtmlParser::CHtmlParser(void)
	: m_strFileName()
	, m_strContext()
	, m_vecObjectCountStack()
	, m_stackTraverse()
	, m_bValidity(false)
	, m_bReserved()
	, m_strErrMsg()
	, m_stDummy()
	, m_stRoot()
{
}

CHtmlParser::~CHtmlParser(void)
{
}

bool CHtmlParser::Open(std::tstring strFileName)
{
	HANDLE hFile = nullptr;
	HANDLE hMap = nullptr;
	LPCBYTE pFileData = nullptr;

	try
	{
		hFile = CreateFile(strFileName.c_str(), GENERIC_READ_, OPEN_EXISTING_, FILE_ATTRIBUTE_NORMAL_);
		if (NULL == hFile)
			throw exception_format(TEXT("CreateFile(%s) failure, %d"), strFileName.c_str(), GetLastError());

		size_t tFileSize = (size_t)GetFileSize(hFile);
		hMap = core::CreateFileMapping(hFile, PAGE_READONLY_, FILE_MAP_READ_, tFileSize);
		if (NULL == hMap)
			throw exception_format("CreateFileMapping Error");

		pFileData = (BYTE*)MapViewOfFile(hMap, 0, tFileSize);
		if (NULL == pFileData)
			throw exception_format("MapViewOfFile Error");

		m_strContext = TCSFromUTF8((LPCSTR)pFileData, tFileSize);
		m_strContext = EliminateHtmlComment(m_strContext, TEXT("<!--"), TEXT("-->"));
		m_strContext = EliminateHtmlComment(m_strContext, TEXT("<!"), TEXT(">"));
		m_strContext = EliminateHtmlComment(m_strContext, TEXT("<?"), TEXT("?>"));

		UnmapViewOfFile(hMap, pFileData);
		CloseFile(hFile);
	}
	catch (const std::exception& e)
	{
		if (hMap && pFileData)
			UnmapViewOfFile(hMap, pFileData);

		if (hFile)
			CloseFile(hFile);

		Log_Error("%s", e.what());
		return false;
	}
	return true;
}

bool CHtmlParser::ParseFromMemory(std::tstring& strContext, ST_HTML_NODE& stOutRoot, std::tstring& strOutErrMsg)
{
	std::string strTempErrMsg;
	try
	{
		// 태그를 기반으로 위치를 파악한다.
		std::vector<ST_HTML_TOKENIZED_TAG> vecTags;
		if (!__ScanHtmlContext(strContext, vecTags, strTempErrMsg))
			throw exception_format("%s", strTempErrMsg.c_str());

		// 태그를 기반으로 트리를 구성한다.
		if (!__TokenizeHtmlContext(strContext, vecTags, stOutRoot, strTempErrMsg))
			throw exception_format("%s", strTempErrMsg.c_str());
	}
	catch (const std::exception& e)
	{
		strOutErrMsg = TCSFromMBS(e.what());
		return false;
	}
	return true;
}

ECODE CHtmlParser::Parse(std::tstring strFileName)
{
	m_strFileName = strFileName;
	try
	{
		if (!Open(strFileName))
			throw exception_format(TEXT("Open(%s) failure"), strFileName.c_str());

		if (!ParseFromMemory(m_strContext, m_stRoot, m_strErrMsg))
			throw exception_format(TEXT("ParseFromMemory failure : %s"), m_strErrMsg.c_str());
	}
	catch (const std::exception& e)
	{
		Log_Error("%s", e.what());
		return EC_OPEN_FAILURE;
	}
	return EC_SUCCESS;
}

ST_HTML_NODE CHtmlParser::GetDomTree(void)
{
	return m_stRoot;
}

bool CHtmlParser::GetContext(std::tstring& strOutContext)
{
	if (m_strContext.empty())
		return false;

	strOutContext = m_strContext;
	return true;
}
