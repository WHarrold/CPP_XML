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

    //Public function
  public:
    bool XMLDocLoad(XMLDocument* doc, const std::string& path)
    {
      std::ifstream file (path);
      if(!file.is_ooen()){
        std::cerr << "Could not load file from " << path << std::endl;
        return false;
      }
      std::string buf;

      //reserve the memory in the string for the size of the file
      file.seekg(0, std::ios::end);
      buf.reserve(file.tellg());
      file.seekg(0, std::ios::beg);

      //insert the entire content of the file into the string
      buf.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

      //close file
      file.close();

      //lexical buffer, lexical index, buffer index
      std::srring lex;
      decltype(lex.size()) lexi = 0;
      decltype(buf.size()) index = 0;

      //now we parse the file
      XMLNode* curr_node = doc->root;

      while(buf[index] != buf.size()){
        if(buf[index] == '<'){
          //lex[lexi] = '\0';

          //inner text
          if(lexi > 0){
            if(!curr_node){
              std::cerr << "Text outside of document" << std::endl;
              return false;
            }
            curr_node->inner_text = lex;
            lexi = 0;
          }

          //end of node
          if(buf[index + 1] == '/'){
            index += 2;
            while(buf[index] != '>'){
              lex += buf[index ++];
              ++lexi;
            }

            if(!curr_node){
              std::cerr << "Already at root node" << std::endl;
              return false;
            }

            if(curr_node->tag.compare(lex) != 0){
              std::cerr << "Mismatched tags (" << curr_node->tag << "!=" << lex << ")" << std::endl;
              return false;
            }
            curr_node = curr_node->parent;
            ++index;
            continue;
          }

          //special node
          if(buf[index + 1] =='!'){
            while(buf[index] != ' ' && buf[index] != '>'){
              lex += buf[index++];
              ++lexi;
            }

            //commemts
            if(lex.compare("<!--") == 0){
              //pre c++20 compiler therefore i must 
              //implement my own ends_wth function
              const auto ends_wth = [](const std::string& str, const std::string& suffix){
                return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix);
              }
              while(!ends_wth(lex, "-->")){
              lex += buf[index++];
              ++lexi;
            }
            continue;
          }
        }

        //declaration tag
        if(buf[index +1] == '?'){
          while(buf[index] != ' ' && buf[index] != '>'){
            lex += buf[index++];
            ++lexi;
          }
          //lex[lexi] = '\0'
          

          //this is the XML declaration
          if(lex.compare("<?xml") == 0){
            lexi = 0;
            lex.clear();

            XMLNode* desc->newNode();

            parseAttrs(buf, &index, lex, &lexi, desc);

            doc->version = XMLNodeAttrVal(desc, "version");
            doc->encoding = XMLNodeAttrVal(desc, "encoding");
            continue;
          }
        }

        //set the current node
        curr_node->newNode(curr_node);

        //start tag
        ++index;
        if(parseAttrs(buf, &index, lex, &lexi, curr_node) == TAG_INLINE){
          curr_node = curr_node->parent;
          ++index;
          continue;
        }

        //set tag name if none
        if(!curr_node->tag)
          curr_node->tag = lex;

        //reset lexer
        lexi = 0;
        lex.clear();
        ++index;
        continue;
      }else{
        lex += buf[index++];
        ++lexi;
      }
    }
    return true;
    }

   
   
};

#endif//CPP_XML_H

