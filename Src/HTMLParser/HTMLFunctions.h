#pragma once

namespace fmt_internal
{
	//////////////////////////////////////////////////////////////////////////
	struct ST_HTML_ATTR
	{
		std::tstring strKey;
		std::tstring strValue;
	};

	//////////////////////////////////////////////////////////////////////////
	struct ST_HTML_NODE
	{
		std::tstring strKey;
		std::tstring strValue;
		std::vector<ST_HTML_ATTR> vecAttr;
		std::vector<ST_HTML_NODE> vecChild;
	};

	//////////////////////////////////////////////////////////////////////////
	bool ParseHtmlContext(const std::tstring& strContext, ST_HTML_NODE& stOutRoot, std::tstring& strOutErrMsg);
	std::tstring EncodeHtmlString(std::tstring strContext);
	std::tstring DecodeHtmlString(std::tstring strContext);
}
