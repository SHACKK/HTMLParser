#pragma once

std::tstring EliminateHtmlComment(std::tstring& strContext, const std::tstring& strStartTag, const std::tstring& strEndTag);
std::tstring DecodeHtmlString(std::tstring strContext);
bool __ScanHtmlAttributeContext(const std::tstring& strContext, size_t tOffset, std::vector<ST_HTML_TOKENIZED_TAG>& vecTokenizedTag, std::string& refStrErrMsg);
bool __ScanHtmlContext(const std::tstring& strContext, std::vector<ST_HTML_TOKENIZED_TAG>& vecTokenizedTag, std::string& refStrErrMsg);
bool __TokenizeHtmlContext(const std::tstring& strContext, const std::vector<ST_HTML_TOKENIZED_TAG> vecTags, ST_HTML_NODE& root, std::string& refStrErrMsg);

std::vector<ST_HTML_NODE> ExtractNodeByTagName(std::tstring strTagName, ST_HTML_NODE& stRoot);
std::vector<std::tstring> ExtractScript(ST_HTML_NODE& stRoot);
std::vector<std::tstring> ExtractResourceUrlByTagName(std::tstring strTagName, ST_HTML_NODE& stRoot);