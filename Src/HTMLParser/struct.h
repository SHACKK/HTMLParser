#pragma once

enum eHtmlTokenizedTagType
{
	HTML_TOKENIZED_TAG_OPEN_ELEMENT = 0,
	HTML_TOKENIZED_TAG_CLOSE_ELEMENT,
	HTML_TOKENIZED_TAG_SINGLE_ELEMENT,
	HTML_TOKENIZED_TAG_VALUE,
	HTML_TOKENIZED_TAG_ATTR_KEY,
	HTML_TOKENIZED_TAG_ATTR_VALUE,

	HTML_TOKENIZED_TAG_COUNT
};

struct ST_HTML_TOKENIZED_TAG
{
	eHtmlTokenizedTagType nType;
	size_t tIndex;
	size_t tLength;

	ST_HTML_TOKENIZED_TAG()
	{}

	ST_HTML_TOKENIZED_TAG(eHtmlTokenizedTagType type, size_t index, size_t len)
		: nType(type)
		, tIndex(index)
		, tLength(len)
	{}

	~ST_HTML_TOKENIZED_TAG()
	{}
};

enum E_GROUPING_TYPE { GT_ROOT, GT_DICTIONARY, GT_ARRAY, GT_OBJECT };

struct sGroupingData
{
	std::tstring strName;
	E_GROUPING_TYPE nType;
	size_t tTotalCount;
	size_t tSequence;

	sGroupingData(E_GROUPING_TYPE type, size_t tCount)
		: strName(), nType(type), tTotalCount(tCount), tSequence(0) {}

	sGroupingData(std::tstring inName, E_GROUPING_TYPE type, size_t tCount)
		: strName(inName), nType(type), tTotalCount(tCount), tSequence(0) {}
};

struct ST_HTML_ATTR
{
	std::tstring strKey;
	std::tstring strValue;

	ST_HTML_ATTR()
		: strKey()
		, strValue()
	{}

	ST_HTML_ATTR(std::tstring key, std::tstring value)
		: strKey(key)
		, strValue(value)
	{}

	~ST_HTML_ATTR()
	{}
};

struct ST_HTML_NODE
{
	std::tstring strTagName;
	std::tstring strText;
	std::vector<ST_HTML_ATTR> vecAttr;
	std::vector<ST_HTML_NODE> vecChild;

	ST_HTML_NODE()
		: strTagName()
		, strText()
		, vecAttr()
		, vecChild()
	{}
};