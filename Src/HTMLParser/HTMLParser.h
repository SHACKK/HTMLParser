#pragma once

class CHtmlParser
{
	std::tstring m_strFileName;
	std::tstring m_strContext;

	std::vector<sGroupingData> m_vecObjectCountStack;
	std::stack<ST_HTML_NODE> m_stackTraverse;
	bool m_bValidity;
	bool m_bReserved[7];
	std::tstring m_strErrMsg;

	ST_HTML_NODE m_stDummy;
	ST_HTML_NODE m_stRoot;

private:
	bool ParseFromMemory(std::tstring& strContext, ST_HTML_NODE& stOutRoot, std::tstring& strOutErrMsg);

public:
	CHtmlParser(void);
	~CHtmlParser(void);

	bool Open(std::tstring strFileName);
	ECODE Parse(std::tstring strFileName);

public:
	ST_HTML_NODE GetDomTree(void);
	bool GetContext(std::tstring& strOutContext);
};

