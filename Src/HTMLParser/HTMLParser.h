#pragma once


class HTMLParser
{
	HANDLE m_hFile;
	HANDLE m_hMap;
	std::tstring m_strFileName;
	std::tstring m_strContext;

	ST_HTML_NODE m_stRoot;

	LPSTR m_pszContext;
	size_t m_tContextSize;

private:
	bool Open(std::tstring strHtmlFile, std::tstring& strContext);
	void Close();
	ECODE ParseFromMemory(std::tstring& strContext);

public:
	HTMLParser();
	~HTMLParser();

	ECODE Parse(const std::tstring& strHtmlFile);
	ST_HTML_NODE& GetRoot();
};

