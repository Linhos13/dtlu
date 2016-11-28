#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/CommandLine.h>
#include <iostream>
#include <list>
#include <memory>
#include "linkersupports.hpp"


int main(int args, char**);
void help();

using namespace llvm;
static cl::OptionCategory IndexerCategory("Indexer Options", "Options for dtlu-indexer.");
static cl::opt<std::string> InputFilename(cl::Positional, cl::Required, cl::desc("<input file>"), cl::cat(IndexerCategory));
static cl::opt<bool> IndexGlobal("g", cl::desc("Get global visible global variables"), cl::cat(IndexerCategory));

int main(int argc, char** argv)
{
    cl::HideUnrelatedOptions(IndexerCategory);
    cl::ParseCommandLineOptions(argc, argv,"Print global defined function");
    LLVMContext& context = llvm::getGlobalContext();
    SMDiagnostic error;
    std::string file_name =  InputFilename;
    std::unique_ptr<Module> m = parseIRFile(file_name, error, context);
    if(!m)
    {
        std::cout << error.getMessage().str() << std::endl;
        return 2;
    }
    Module* module = m.release();
    if(IndexGlobal)
    {
        auto l = LinkerSupports::getDefinedGlobals(module);
        for(auto b = l.begin(); b != l.end(); ++b)
        {
                std::cout << (*b)->getName().str() << std::endl;
        }
    }
    else
    {
        auto l = LinkerSupports::getDefinedFunctions(module);
        for(auto b = l.begin(); b != l.end(); ++b)
        {
                std::cout << (*b)->getName().str() << std::endl;
        }
    }
    delete module;
    return 0;
}
