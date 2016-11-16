#ifndef __LINKERSUPPORTS_H__
#define __LINKERSUPPORTS_H__

#include <list>
#include <memory>

//forward declare
namespace llvm
{
    class Function;
    class Module;
    class GlobalAlias;
    class GlobalVariable;
    class Value;
}

namespace LinkerSupports
{
    std::list<llvm::Function*> getCalledFunctions(llvm::Function* function);
    std::list<llvm::Function*> getDefinedFunctions(llvm::Module* module);
    std::list<llvm::Function*> getNotDefinedFunctions(llvm::Module* module);
    std::list<llvm::GlobalVariable*> getDefinedGlobals(llvm::Module* module);
    std::list<llvm::GlobalVariable*> getUsedGlobals(llvm::Function *function);
    std::list<llvm::GlobalAlias*> getUsedGlobalAliases(llvm::Function *function);
    std::list<llvm::GlobalVariable*> recursiveGlobals(llvm::Value* op);
    bool isDefined(llvm::Function* function);
}
#endif
