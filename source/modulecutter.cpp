#include "modulecutter.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Casting.h>
#include <llvm/IR/Constants.h>
#include "linkersupports.hpp"
#include "projectlinker.hpp"
#include <list>
#include <iostream>

using namespace llvm;

ModuleCutter::ModuleCutter(std::string name, ProjectLinker* parent) : parent(parent), name(name)
{
    SMDiagnostic error;
    std::unique_ptr<Module> m = parseIRFile(StringRef(name), error, getGlobalContext());
    if(!m)
    {
        std::cerr << name << std::endl;
        std::cerr << error.getMessage().str() << std::endl;
        std::exit(2);
    }
    module = m.release();
}

void ModuleCutter::useFunction(llvm::Function* func)
{
    auto result = usedFunctions.insert(func);
    if(result.second)
    {
        functionProcessQueue.push_back(func);
    }
}

void ModuleCutter::useGlobal(llvm::GlobalVariable* global)
{
    auto result = usedGlobals.insert(global);
    if(result.second)
    {
        globalProcessQueue.push_back(global);
    }
}


void ModuleCutter::addFunction(std::string func_name)
{
    Function* f = module->getFunction(StringRef(func_name));
    if(f)
    {
        useFunction(f);
    }
}

void ModuleCutter::addGlobal(std::string global_name)
{
    GlobalVariable* g = module->getGlobalVariable(StringRef(global_name));
    if(g)
    {
        useGlobal(g);
    }
}

void ModuleCutter::addAllDefinedFunctions()
{
    std::list<Function*> l = LinkerSupports::getDefinedFunctions(module);
    useFunctions(l);
}

void ModuleCutter::processFunctions()
{
    while(!functionProcessQueue.empty())
    {
        Function* func = functionProcessQueue.front();
        //debug
        //std::cerr <<  name << "Function " <<  func->getName().str() << std::endl;
        if(!LinkerSupports::isDefined(func))
        {
            parent->useExternalFunction(func->getName().str());
        }
        else
        {
            /*std::list<Function*> called = LinkerSupports::getCalledFunctions(func);
            useFunctions(called);
            std::list<GlobalVariable*> used = LinkerSupports::getUsedGlobals(func);
            useGlobals(used);*/
            checkFunction(func);
        }
        functionProcessQueue.pop_front();
    }
}

void ModuleCutter::processGlobals()
{
    while(!globalProcessQueue.empty())
    {
        GlobalVariable* global = globalProcessQueue.front();
        if(!global->hasInitializer() && global->hasExternalLinkage())
        {
            parent->useExternalGlobal(global->getName().str());
        }
        checkGlobal(global);
        globalProcessQueue.pop_front();
    }
}


void ModuleCutter::useFunctions(std::list<Function*> l)
{
    for(Function * f : l)
    {
        useFunction(f);
    }
}

void ModuleCutter::useGlobals(std::list<GlobalVariable*> l)
{
    for(GlobalVariable* g : l)
    {
        //std::cerr << "Global " << g->getName().str() << std::endl;
        useGlobal(g);
    }
}

void ModuleCutter::processQueues()
{
    while(!hasEmptyQueues())
    {
        processFunctions();
        processGlobals();
    }
}

void ModuleCutter::cutUnused()
{
    cutFunctions();
    cutGlobals();
}

void ModuleCutter::cutFunctions()
{
    std::list<std::string> unused;
    for(auto it = module->getFunctionList().begin(); it != module->getFunctionList().end(); ++it)
    {
        auto &f = *it;
        if(usedFunctions.find(&f) == usedFunctions.end())
        {
            unused.push_back(f.getName().str());
        }
    }
    for(auto un : unused)
    {
        Function * f = module->getFunction(un);
        if(f)
        {
            f->replaceAllUsesWith(UndefValue::get(f->getType()));
            f->removeFromParent();
            delete f;
        }
    }

}

void ModuleCutter::cutGlobals()
{
    std::list<std::string> unused;
    for(auto it = module->global_begin(); it != module->global_end(); ++it)
    {
        auto &f = *it;
        //std::cerr << name << " "  << f.getName().str() << std::endl;
        if(usedGlobals.find(&f) == usedGlobals.end())
        {
            //std::cerr << name << " not use "  << f.getName().str() << std::endl;
            unused.push_back(f.getName().str());
        }
    }
    for(auto un : unused)
    {

        module->getGlobalVariable(un, true)->eraseFromParent();
    }

}

void ModuleCutter::checkGlobal(GlobalVariable* global)
{
    recursiveGlobals(global);
    recursiveFunctions(global);
}

void ModuleCutter::checkFunction(Function* function)
{
    for (auto I = function->begin(), E = function->end(); I != E; ++I)
    {
        for (llvm::BasicBlock::iterator i = I->begin(), ie = I->end(); i != ie; ++i)
        {
            recursiveGlobals(&*i);
            recursiveFunctions(&*i);
        }
    }
}

void ModuleCutter::recursiveGlobals(User* user)
{

    for (Value *o : user->operands())
    {
        if (llvm::GlobalVariable* g = llvm::dyn_cast<llvm::GlobalVariable>(&*o))
        {
            useGlobal(g);
        }
        else if(User *u = dyn_cast<User>(&*o))
        {
            recursiveGlobals(u);
        }
    }
}

void ModuleCutter::recursiveFunctions(User* user)
{

    for (Value *o : user->operands())
    {
        if (llvm::Function* g = llvm::dyn_cast<llvm::Function>(&*o))
        {
            useFunction(g);
        }
        else if(User *u = dyn_cast<User>(&*o))
        {
            recursiveFunctions(u);
        }
    }
}

void ModuleCutter::writeToBC(std::string fileName /* = "" */)
{
    //module->dump();
    std::string outputString("");
    raw_string_ostream errorDesc(outputString);
    bool check = verifyModule(*module, &errorDesc);
    //std::cerr << module->materializeAll() << std::endl;
    if(check)
    {
        std::cerr <<"Error in module verification " << errorDesc.str() << std::endl;
        module->dump();
        std::exit(3);
    }
    if(fileName.empty())
    {
        fileName = name + ".processed";
    }
    std::error_code EC;
    raw_fd_ostream os(fileName, EC, sys::fs::OpenFlags(0));
    WriteBitcodeToFile(module, os);
    os.flush();
}

bool ModuleCutter::hasFunction(std::string name)
{
    Function* f = module->getFunction(StringRef(name));
    if(!f)
        return false;
    if(!f->hasPrivateLinkage() && !f->hasInternalLinkage())
        return LinkerSupports::isDefined(f);
}

bool ModuleCutter::hasGlobal(std::string name)
{
    GlobalVariable* g = module->getGlobalVariable(StringRef(name));
    if(!g)
        return false;
    return LinkerSupports::isInicializedAndGlobalAccessible(g);
}

ModuleCutter::~ModuleCutter()
{
    delete module;
}
