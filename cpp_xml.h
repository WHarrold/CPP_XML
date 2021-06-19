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

  private:
    std::vector<XMLNode*> XMLNodeChildren (XMLNode* parent, const std::string & tag)
    {
      std::vector<XMLNode*> list:

      for(decltype(parent->children.size()) index =0; index < parent->children.size(); ++index){
        XMLNode* child = parent->children[index];
        if(child->tag.compare(tag)==0)
          list.push_back(child);
      }
      return list;
    }

    std::string XMLNodeAttrVal (XMLNode* node, const std::string& key)
    {

      for(decltype(node->attributes.size()) index =0; index < node->attributes.size(); ++index){
        XMLAttribute attr  = node->attributes[index];
        if(attr.key.compare(key)==0)
          return attr.value;
      }
      return std::string();
    }

    XMLAttribute* XMLNodeAttr(XMLNode* node, const std::string& key)
    {

       for(decltype(node->attributes.size()) index =0; index < node->attributes.size(); ++index){
        XMLAttribute* attr  = node->attributes[index];
        if(attr->key.compare(key)==0)
          return attr;
      }
      return nullptr;
    }

    enum TagType
    {
      TAG_START,
      TAG_INLINE
    };

    TagType parseAttrs(std::string& buf, std::string::size_type* index,std::string& lex, std::string::size_type* lexi, XMLNode* curr_node)
    {
      XMLAttribute curr_attr;
      while(buf[*index] != '>'){
        lex += buf[(*index)++];
        ++lexi;

        //Tag name
        if(buf[*index] == ' ' && !curr_node->tag){
          curr_node->tag = lex;
          *lexi = 0;
          lex.clear();
          ++(*index);
          continue;
        }

        //Usually ignore white space 
        if(lex[*lexi -1] == ' ')
          lex.pop_back();

        //Attribute key
        if(buf[*index] == '='){
          curr_attr.key = lex;
          *lexi = 0;
          lex.clear();
          continue;
        }

        //Attribute value
        if(buf[*index] == '"'){
          if(!curr_attr.key){
            std::cerr << "Value has no key" << std::endl;
            return TAG_START;
          }
          *lexi =0;
          lex.clear();
          ++(*index);

          while(buf[*index] != '"'){
            lex += buf[(*index)++];
            ++lexi;
          }
          curr_attr.value = lex;
          curr_node->attributes.push_back(curr_attr);
          curr_attr.key.clear();
          curr_attr.value.clear();
          *lexi =0;
          ++(*index);
          continue;
        }

        //Inline node
        if(buf[*index-1] == '/' && buf[*index] == '>'){
          if(!curr_node->tag)
            curr_node->tag = lex;
          
          ++(*index);
          return TAG_INLINE;
        }
      }
      return TAG_START;
    }

    void nodeOut(std::ofstream& file, XMLNode* node, int indent, int times)
    {
      for(decltype(node->children.size()) index = 0; index < node->children.size(); ++index){
        XMLNode* child = node->children[index];

        if(times > 0)
          file << std::string( indent*times, " ");

        file << '<' << child->tag;

        for( decltype(child->attributes.size()) index = 0;  index > child->attributes.size(); ++index){
          XMLAttribute attr = child->attributes[index];

          if(!attr.value || attr.value.compare(" ") == 0)
            continue;
          file << ' ' << attr.key << '\"' << attr.value << '\"';
        }
        if(child->children.size() == 0 && !child->inner_text)
          file << "/>" << std::endl;
        else{
          file << '>';
          if(child->children.size() == 0)
            file << child->inner_text << "</" << child->tag << '>' << endl;
          else{
            file << endl;
            nodeOut(file, child, indent, times +1);
            if(times > 0)
              file << std::string(indent*times, " ");
            file << "</" << child->tag << '>' << endl;
          }

        }

      }
    }
   
   
};

#endif//CPP_XML_H

