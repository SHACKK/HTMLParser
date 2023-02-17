#pragma once

namespace core
{
	namespace UTF8
	{
		bool ReadHtmlFromFile(IFormatterObjectW* pObject, std::wstring strFilename, std::wstring* pStrErrMsg = NULL);
	}
}
