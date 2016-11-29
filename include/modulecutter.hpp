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
         std::deque<llvm::Function*> functionProcessQueue;
         std::deque<llvm::GlobalVariable*> globalProcessQueue;
         void useFunction(llvm::Function* func);
         void useGlobal(llvm::GlobalVariable* global);
         void useFunctions(std::list<llvm::Function*> list);
         void useGlobals(std::list<llvm::GlobalVariable*> list);
         void processFunctions();
         void processGlobals();
         void checkGlobal(llvm::GlobalVariable* global);
         void checkFunction(llvm::Function* func);
         void cutFunctions();
         void cutGlobals();
         void recursiveGlobals(llvm::User* user);
         void recursiveFunctions(llvm::User* user);
         void removeRedefindedFunction(llvm::Function* f);
         void removeRedefindedGlobal(llvm::GlobalVariable* g);
    public:
        ModuleCutter(std::string name, ProjectLinker* parent);
        std::string getName() {return this->name;}
        llvm::Module* getModule() {return this->module;}
        void addAllDefinedFunctions();
        void addFunction(std::string func_name);
        void addGlobal(std::string global_name);
        void processQueues();
        bool hasEmptyQueues(){ return functionProcessQueue.empty() && globalProcessQueue.empty();}
        void cutUnused();
        void writeToBC(std::string fileName = "");
        bool hasFunction(std::string name);
        bool hasGlobal(std::string name);
        virtual ~ModuleCutter();
};

#endif
