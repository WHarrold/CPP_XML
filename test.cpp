#include"cpp_xml.h"
#include<iostream>
#include<string>
#include<fstream>


int main ()
{
  CPP_XML::XMLDocument doc;

  CPP_XML xml;
  xml.XMLDocLoad(&doc, "sample.htmle" );

  XMLDocWrite(std::cout, &doc, 0);
  std::cout << std::flush;
}
