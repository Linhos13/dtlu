
#include <llvm/Pass.h>
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
#include <fstream>
#include <iostream>
#include <list>
#include <deque>
#include <memory>
#include <map>
#include <set>
#include <unistd.h>
#include "projectlinker.hpp"
#include "symbolmap.hpp"


int main(int args, char**);
bool isDefined(llvm::Function* function);
void help();
static llvm::Module* getOrLoadModule(std::map<std::string, llvm::Module*>& map, std::string* fileName);
llvm::Module* getModuleFromFunction(std::map<llvm::Module*,std::set<llvm::Function*>>& map, llvm::Function* function);
std::string joinPath(std::string path, std::string file);
static llvm::LLVMContext& context = llvm::getGlobalContext();

using namespace llvm;
int main(int args, char** argv)
{
    std::string llvm_linker("llvm-link");
    if(args < 2)
    {
        help();
        return 1;
    }
    int opt;
    char *project_dir = NULL;
    char *output_file = NULL;
    std::string mainName(argv[1]);
    while((opt = getopt(args, argv, "p:o:")) != -1)
    {
        switch (opt)
        {
            case 'p':
                project_dir = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case '?':
                if (optopt == 'p' || optopt == 'o')
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
    if(!project_dir)
    {
        std::cout << "Missing project directory -p argument.\n";
        return 1;
    }
    ProjectLinker project(mainName, project_dir, llvm_linker);

    std::string name;
    if(!output_file)
    {
        name = std::string("output");
    }
    else
    {
        name = std::string(output_file);
    }
    project.linkModulesToFile(name);
    project.printNotFound();
}


void help()
{
    std::cout << "Use: linker file [-a name]\n";
    std::cout << "file must be llvm bytecode\n";
}
