# HTML Parser
```
HTML 파일을 Parsing 하여 DOM-TREE를 생성하는 코드입니다.
```

# Usage
```
CHtmlParser parser;
parser.parse("HTML File Path");
ST_HTML_NODE stRoot = parser.GetDomTree();
```

# dependency
```
cppcore : https://github.com/profrog-jeon/cppcore
```