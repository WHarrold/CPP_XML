#ifndef CPP_XML_H
#define CPP_XML_H

#include<vector>
#include<iostream>
#include<string>
#include<fstream>
#include<algorithm>
#include<iterator>
#include<streambuf>

struct XMLNode;





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
      XMLNode() = default;
      XMLNode(XMLNode& a_parent): parent(&a_parent)
              {
                parent->children.push_back(this);
              }

    };


  private:

    std::vector<XMLNode*> XMLNodeChildren (XMLNode* parent, const std::string & tag)
    {
      std::vector<XMLNode*> list;

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
        XMLAttribute* attr  = &(node->attributes[index]);
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
        if(buf[*index] == ' ' && curr_node->tag.empty()){
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
          if(curr_attr.key.empty()){
            std::cerr << "Value has no key" << '\n';
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
          if(curr_node->tag.empty())
            curr_node->tag = lex;

          ++(*index);
          return TAG_INLINE;
        }
      }
      return TAG_START;
    }

  static void nodeOut(std::ostream& file, XMLNode* node, int indent, int times)
    {
      for(decltype(node->children.size()) index = 0; index < node->children.size(); ++index){
        XMLNode* child = node->children[index];

        if(times > 0)
          file << std::string( indent*times, ' ');

        file << '<' << child->tag;

        for( decltype(child->attributes.size()) index = 0;  index > child->attributes.size(); ++index){
          XMLAttribute attr ( child->attributes[index]);

          if(attr.value.empty() || attr.value.compare(" ") == 0)
            continue;
          file << ' ' << attr.key << '\"' << attr.value << '\"';
        }
        if(child->children.size() == 0 && child->inner_text.empty())
          file << "/>" << '\n';
        else{
          file << '>';
          if(child->children.size() == 0)
            file << child->inner_text << "</" << child->tag << '>' << '\n';
          else{
            file << '\n';
            nodeOut(file, child, indent, times +1);
            if(times > 0)
              file << std::string(indent*times, ' ');
            file << "</" << child->tag << '>' << '\n';
          }

        }

      }
    }

    //Public function
  public:


    struct XMLDocument
    {
      XMLNode* root;
      std::string version;
      std::string encoding;
    };




    bool XMLDocLoad(XMLDocument* doc, const std::string& path)
    {
      std::ifstream file (path);
      if(!file.is_open()){
        std::cerr << "Could not load file from " << path << '\n';
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
      std::string lex;
      decltype(lex.size()) lexi = 0;
      decltype(buf.size()) index = 0;

      //now we parse the file
      XMLNode* curr_node = {doc->root};

      while(buf[index] != buf.size()){
        if(buf[index] == '<'){
          //lex[lexi] = '\0';

          //inner text
          if(lexi > 0){
            if(!curr_node){
              std::cerr << "Text outside of document" << '\n';
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
              std::cerr << "Already at root node" << '\n';
              return false;
            }

            if(curr_node->tag.compare(lex) != 0){
              std::cerr << "Mismatched tags (" << curr_node->tag << "!=" << lex << ")" << '\n';
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
               auto ends_wth = [] ( const std::string& str, const std::string& suffix){
                return str.size() >= suffix.size() && std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
              };
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

            XMLNode* desc;

            parseAttrs(buf, &index, lex, &lexi, desc);

            doc->version = XMLNodeAttrVal(desc, "version");
            doc->encoding = XMLNodeAttrVal(desc, "encoding");
            continue;
          }
        }

        //set the current node
        XMLNode newNode(curr_node);
        curr_node = &newNode;

        //start tag
        ++index;
        if(parseAttrs(buf, &index, lex, &lexi, curr_node) == TAG_INLINE){
          curr_node = curr_node->parent;
          ++index;
          continue;
        }

        //set tag name if none
        if(curr_node->tag.empty())
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




  public:
    CPP_XML() = default;
    CPP_XML(XMLDocument doc, const std::string& path): m_doc(doc)
    {
      if(!XMLDocLoad(&m_doc, path))
      {
        std::cout << "Cannot load file: " << path <<'\n';
      }

    }



 friend  std::ostream &XMLDocWrite (std::ostream& os, const CPP_XML& doc, const std::string& path, int indent);

    //member variables
    private:
    XMLDocument m_doc;

};

    std::ostream& XMLDocWrite (std::ostream& os, const CPP_XML& doc, const std::string& path, int indent);


    std::ostream& XMLDocWrite (std::ostream& os, const CPP_XML& doc, const std::string& path, int indent)
    {
      os << "<?xml version \"" << ((!doc.m_doc.version.empty()) ? doc.m_doc.version : "1.0" ) << "encoding \""  << ((!doc.m_doc.encoding.empty()) ? doc.m_doc.encoding : "U5F-8") << '\"' << " ?>";
      os << '\n';

      CPP_XML::nodeOut(os, doc.m_doc.root, indent, 0);
      std::cout << std::flush;
      return os;
    }
#endif//CPP_XML_H


