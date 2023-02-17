#include "pch.h"
#include "../../../cppcore/Src/300_Formatter/300_Formatter.h"
#include "../../../cppcore/Src/300_Formatter/FileReader.h"
#include "../../../cppcore/Src/300_Formatter/UTF8FileReader.h"
#include "HtmlDeserializer.h"

bool core::UTF8::ReadHtmlFromFile(IFormatterObjectW* pObject, std::wstring strFilename, std::wstring* pStrErrMsg)
{
	fmt_internal::CUTF8FileReader channel(strFilename);
	fmt_internal::CHTMLDeserializer formatter(channel);
	formatter.Synchronize(pObject);
	return formatter.CheckValidity(pStrErrMsg);
}
