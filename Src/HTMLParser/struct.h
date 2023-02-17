#pragma once

struct ST_HTML_ATTRIBUTE
{
	std::tstring strKey;
	std::tstring strValue;

	ST_HTML_ATTRIBUTE()
		: strKey()
		, strValue()
	{}
};

struct ST_HTML_NODE
{
	std::tstring strTag;
	std::tstring strValue;
	std::vector<ST_HTML_ATTRIBUTE> vecAttr;
	std::vector<ST_HTML_NODE> vecChild;

	ST_HTML_NODE()
		: strTag()
		, strValue()
		, vecAttr()
		, vecChild()
	{}
};
