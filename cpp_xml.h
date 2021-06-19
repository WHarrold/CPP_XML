#ifndef CPP_XML_H
#define CPP_XML_H

#include<vector>
#include<iostream>
#include<string>
#include<sstring>
#include<sstring>
#include<fstream>
#include<algorithm>
#include<iterator>
#include<streambuf>

class CPP_XML
{
  private:
    struct XMLAttribute
    {
      std::string key;
      std::string value;
    };

    struct XMLNode
    {
      std::string tag;
      std::string inner_text;
      XMLNode* parent;
      std::vector<XMLAttribute> attributes;
      std::vector<XMLNode*> children;
    };

    struct XMLDocument
    {
      XMLNode* root;
      std::string version;
      std::string encoding;
    };

};

#endif//CPP_XML_H

