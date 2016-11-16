#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Casting.h>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <unistd.h>
#include "linkersupports.hpp"


int main(int args, char**);
void help();

using namespace llvm;
int main(int args, char** argv)
{
    if(args < 2)
    {
        help();
        return 1;
    }
    LLVMContext& context = llvm::getGlobalContext();
    SMDiagnostic error;
    std::unique_ptr<Module> m = parseIRFile(argv[1], error, context);
    if(!m)
    {
        std::cout << error.getMessage().str() << std::endl;
        help();
        return 2;
    }
    Module* module = m.release();
    int opt;
    bool undefined = false;
    bool globals = false;
    char* func_name = NULL;
    while((opt = getopt(args, argv, "ugf:")) != -1)
    {
        switch (opt)
        {
            case 'u':
              undefined = true;
              break;
            case 'g':
                globals = true;
            case 'f':
                func_name = optarg;
                break;
            case '?':
                if (optopt == 'f')
                    std::cout << "Option -" << (char)optopt << "requires an argument.\n";
                else
                    std::cout << "Unknown option '-" << (char)optopt << "'.\n";
                return 1;
            default:
                help();
                return 1;
                break;
        }
    }

    if(!func_name)
    {
        if(!globals)
        {
            std::list<Function*> l;
            if(!undefined)
                l = LinkerSupports::getDefinedFunctions(module);
            else
                l = LinkerSupports::getNotDefinedFunctions(module);
            for(auto b = l.begin(); b != l.end(); ++b)
            {
                std::cout << (*b)->getName().str() << std::endl;
            }
        }
        else
        {
            std::list<GlobalVariable*> l = LinkerSupports::getDefinedGlobals(module);
            for(auto b = l.begin(); b != l.end(); ++b)
            {
                std::cout << (*b)->getName().str() << std::endl;
            }

        }
    }
    else
    {
        auto func = module->getFunction(StringRef(func_name));
        if(func)
        {
            std::list<Function*> l = LinkerSupports::getCalledFunctions(func);
            for(auto b = l.begin(); b != l.end(); ++b)
            {
                std::cout << (*b)->getName().str() << std::endl;
            }
        }
        else
        std::cerr << "function not found" << std::endl;
    }
    delete module;
    return 0;
}


void help()
{
    std::cout << "Use: function_indexer file [-f name]\n";
    std::cout << "file must be llvm bytecode\n";
}
