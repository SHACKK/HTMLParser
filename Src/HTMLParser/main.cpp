#include "pch.h"
#include "HTMLParser.h"

int main(void)
{
	std::tstring strTargetHTML = TEXT("test.html");
	CHtmlParser parser;
	parser.Parse(strTargetHTML);
	
	return 0;
}