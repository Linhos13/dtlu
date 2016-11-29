#ifndef __PROJECTLINKER_H__
#define __PROJECTLINKER_H__

#include <set>
#include <string>

class ModuleCutter;

namespace LinkerSupports
{
    class SymbolMap;
}

namespace llvm
{
    class Module;
    class Function;
    class GlobalVariable;
}

class ProjectLinker
{
    private:
        //attributes
        std::set<ModuleCutter*> modules;
        LinkerSupports::SymbolMap* functionMap;
        LinkerSupports::SymbolMap* globalMap;
        std::set<std::string> notFoundFunctions;
        std::set<std::string> notFoundGlobals;
        std::string llvm_linker;
        ModuleCutter* mainModule;
        //functions
        ModuleCutter* findModuleByName(std::string file_name);
        void printNotFoundFunctions();
        void printNotFoundGlobals();
        void processModules();
        std::string joinPath(std::string path, std::string file);
    public:
        ProjectLinker(std::string main_module, std::string project_dir, std::string llvm_linker);
        ModuleCutter* getOrLoadModule(std::string file_name);
        ModuleCutter* getModule(llvm::Module* module);
        void useExternalFunction(std::string);
        void useExternalGlobal(std::string);
        void linkModulesToFile(std::string& file);
        ModuleCutter* getMainModule(){ return mainModule; }
        void printNotFound();
        virtual ~ProjectLinker();
};

#endif
