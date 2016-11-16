#ifndef __MODULECUTTER_H__
#define __MODULECUTTER_H__

#include <set>
#include <deque>
#include <list>
#include <string>

class ProjectLinker;
namespace llvm
{
    class Module;
    class Function;
    class GlobalVariable;
    class GlobalAlias;
    class User;
}

class ModuleCutter
{
    private:
         llvm::Module* module;
         ProjectLinker* parent;
         std::string name;
         std::set<llvm::Function*> usedFunctions;
         std::set<llvm::GlobalVariable*> usedGlobals;
         std::set<llvm::GlobalAlias*> usedAliases;
         std::deque<llvm::Function*> functionProcessQueue;
         std::deque<llvm::GlobalVariable*> globalProcessQueue;
         std::deque<llvm::GlobalAlias*> aliasProcessQueue;
         void useFunction(llvm::Function* func);
         void useGlobal(llvm::GlobalVariable* global);
         void useAlias(llvm::GlobalAlias* global);
         void useFunctions(std::list<llvm::Function*> list);
         void useGlobals(std::list<llvm::GlobalVariable*> list);
         void useAliases(std::list<llvm::GlobalAlias*> list);
         void processFunctions();
         void processGlobals();
         void processAliases();
         void checkGlobal(llvm::GlobalVariable* global);
         void checkFunction(llvm::Function* func);
         void cutFunctions();
         void cutGlobals();
         void recursiveGlobals(llvm::User* user);
         void recursiveFunctions(llvm::User* user);
    public:
        ModuleCutter(std::string name, ProjectLinker* parent);
        std::string getName() {return this->name;}
        llvm::Module* getModule() {return this->module;}
        void addAllDefinedFunctions();
        void addFunction(std::string func_name);
        void addGlobal(std::string global_name);
        void processQueues();
        bool hasEmptyQueues(){ return functionProcessQueue.empty() && globalProcessQueue.empty() && aliasProcessQueue.empty();}
        void cutUnused();
        void writeToBC(std::string fileName = "");
        virtual ~ModuleCutter();
};

#endif
