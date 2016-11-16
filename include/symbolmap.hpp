#ifndef __SYMBOLMAP_H__
#define __SYMBOLMAP_H__

#include <vector>
#include <map>

namespace LinkerSupports
{
  class SymbolMap
  {
      private:
        std::map<std::string, std::vector<std::string*>*> *function_index;
      public:
        SymbolMap(std::string findex_file );
        virtual ~SymbolMap();
        std::vector<std::string*> *getFiles(std::string func_name);
  };
}

#endif
