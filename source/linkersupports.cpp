#include "linkersupports.hpp"
#include <llvm/Support/Casting.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/GlobalValue.h>
#include <list>
#include <memory>
#include <map>
#include <fstream>

namespace LinkerSupports
{
    using namespace llvm;
    std::list<llvm::Function*> getFunctions(llvm::Module* module, bool defined);
    Function* getFunctionFromValue(Module* module, Value* value);

    std::list<llvm::Function*> getFunctions(llvm::Module* module, bool defined)
    {
        auto &fl = module->getFunctionList();
        std::list<llvm::Function*> l;
        for (auto it = fl.begin(); it != fl.end(); ++it)
        {
            auto &function = *it;
            if ((function.begin() == function.end()) != defined)
            {
                if(defined)
                {
                        if(!function.hasInternalLinkage())
                            l.push_back(&function);
                }
                else
                {
                    l.push_back(&function);
                }
            }
        }
        return l;
    }

    Function* getFunctionFromValue(Module* module, Value* value)
    {
        if(value == nullptr)
            return nullptr;
        auto strptr = value->stripPointerCasts();
        if(Function* f = dyn_cast<Function>(strptr))
        {
            if(f != nullptr)
            {
                Function* pf = module->getFunction(f->getName());
                return pf;
            }
        }
        return nullptr;
    }

    std::list<llvm::Function*> getCalledFunctions(llvm::Function *function)
    {
        Module* module = function->getParent();
        std::list<llvm::Function*> l;
        for (auto I = function->begin(), E = function->end(); I != E; ++I)
        {
            for (llvm::BasicBlock::iterator i = I->begin(), ie = I->end(); i != ie; ++i)
            {
                //call
                if (llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(&*i))
                {
                    Function* cf = callInst->getCalledFunction();
                    if(cf != nullptr)
                    {
                        l.push_back(cf);
                        for(unsigned i = 0; i < callInst->getNumArgOperands(); i++)
                        {
                            Value* v = callInst->getArgOperand(i);
                            Function* f = getFunctionFromValue(module, v);
                            if(f != nullptr)
                                l.push_back(f);
                        }
                    }
                }
                //invoke -> call
                else if (llvm::InvokeInst* invokeInst = llvm::dyn_cast<llvm::InvokeInst>(&*i))
                {
                    l.push_back(invokeInst->getCalledFunction());
                }
                else if(llvm::ReturnInst* returnInst = llvm::dyn_cast<llvm::ReturnInst>(&*i))
                {
                    Value* v = returnInst->getReturnValue();
                    Function* f = getFunctionFromValue(module, v);
                    if(f != nullptr)
                        l.push_back(f);
                }

                //store pointer to func on stack
                else if(llvm::StoreInst* storeInst = llvm::dyn_cast<llvm::StoreInst>(&*i))
                {
                    Value* v = storeInst->getPointerOperand();
                    Function* f = getFunctionFromValue(module, v);
                    if(f != nullptr)
                        l.push_back(f);
                }
                //insert ptr to array or struc
                else if(llvm::InsertValueInst* insertValueInst = llvm::dyn_cast<llvm::InsertValueInst>(&*i))
                {
                    Value* v = insertValueInst->getInsertedValueOperand();
                    Function* f = getFunctionFromValue(module, v);
                    if(f != nullptr)
                        l.push_back(f);
                }

                //ptr to other value
                else if(llvm::PtrToIntInst* ptrToIntInst = llvm::dyn_cast<llvm::PtrToIntInst>(&*i))
                {
                    Value* v = ptrToIntInst->getPointerOperand();
                    Function* f = getFunctionFromValue(module, v);
                    if(f != nullptr)
                        l.push_back(f);
                }
                //bicast ?
                else if(llvm::BitCastInst* bitCastInst = llvm::dyn_cast<llvm::BitCastInst>(&*i))
                {
                    Value* v = bitCastInst->getOperand(0);
                    Function* f = getFunctionFromValue(module, v);
                    if(f != nullptr)
                        l.push_back(f);
                }
                //get ptr of ptr
                else if(llvm::GetElementPtrInst* getElementPtrInst = llvm::dyn_cast<llvm::GetElementPtrInst>(&*i))
                {
                    Value* v = getElementPtrInst->getPointerOperand();
                    Function* f = getFunctionFromValue(module, v);
                    if(f != nullptr)
                        l.push_back(f);
                }
            }
        }
        l.unique();
        return l;
    }

    std::list<llvm::Function*> getDefinedFunctions(llvm::Module* module)
    {
        return getFunctions(module,true);
    }

    std::list<llvm::Function*> getNotDefinedFunctions(llvm::Module* module)
    {
        return getFunctions(module,false);
    }

    std::list<llvm::GlobalVariable*> getDefinedGlobals(llvm::Module* module)
    {
        auto& gl = module->getGlobalList();
        std::list<llvm::GlobalVariable*> l;
        for (auto it = gl.begin(); it != gl.end(); ++it)
        {
            auto &global = *it;
            if(isInicializedAndGlobalAccessible(&global))
                l.push_back(&global);
        }
        return l;
    }

    std::list<llvm::GlobalVariable*> getUsedGlobals(llvm::Function *function)
    {
        std::list<llvm::GlobalVariable*> l;
        for (auto I = function->begin(), E = function->end(); I != E; ++I)
        {
            for (llvm::BasicBlock::iterator i = I->begin(), ie = I->end(); i != ie; ++i)
            {
                for (llvm::Value *op : i->operands())
                {
                    auto s = recursiveGlobals(&*op);
                    l.splice(l.end(),s);
                }
            }

        }
        return l;
    }

    std::list<llvm::GlobalVariable*> recursiveGlobals(llvm::Value* op)
    {
        std::list<llvm::GlobalVariable*> l;
        if (llvm::GlobalVariable* global = llvm::dyn_cast<llvm::GlobalVariable>(&*op))
        {
            l.push_back(global);
        }
        else if (llvm::User* inst = llvm::dyn_cast<llvm::User>(&*op))
        {
            for (llvm::Value *o : inst->operands())
            {
                auto s = recursiveGlobals(&*o);
                l.splice(l.end(), s);
            }
        }
        return l;
    }

    std::list<llvm::GlobalAlias*> getUsedGlobalAliases(llvm::Function *function)
    {
        std::list<llvm::GlobalAlias*> l;
        for (auto I = function->begin(), E = function->end(); I != E; ++I)
        {
            for (llvm::BasicBlock::iterator i = I->begin(), ie = I->end(); i != ie; ++i)
            {
                for (llvm::Value *op : i->operands())
                {
                    if (llvm::GlobalAlias* global = llvm::dyn_cast<llvm::GlobalAlias>(&*op))
                    {
                        l.push_back(global);
                    }
                }
            }

        }
        return l;
    }

    bool isDefined(llvm::Function* function)
    {
        return function->begin() != function->end();
    }

    bool isInicializedAndGlobalAccessible(llvm::GlobalVariable* global)
    {
        return global->hasInitializer() && !global->hasPrivateLinkage() && !global->hasInternalLinkage();
    }
}
