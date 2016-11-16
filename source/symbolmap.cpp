#include "symbolmap.hpp"
#include <fstream>
#include <map>
#include <vector>

namespace LinkerSupports
{
  SymbolMap::SymbolMap(std::string findex_file)
  {
    std::ifstream ifs(findex_file);
    function_index = new std::map<std::string, std::vector<std::string*>*>();
    while(ifs.good())
    {
      std::string func_name;
      std::string* file_name = new std::string;
      ifs >> func_name >> *file_name;
      if((*function_index)[func_name] == nullptr)
      {
        auto vec = new std::vector<std::string*>;
        (*function_index)[func_name] = vec;
      }
      (*function_index)[func_name]->push_back(file_name);
    }
    ifs.close();
  }

  SymbolMap::~SymbolMap()
  {
    for(auto p : *function_index)
    {
      for(auto f : *(p.second))
      {
        delete f;
      }
      delete p.second;
    }
    delete function_index;
  }

  std::vector<std::string*>* SymbolMap::getFiles(std::string func_name)
  {
    auto r = function_index->find(func_name);
    return r != function_index->end() ? r->second : nullptr;
  }
}
