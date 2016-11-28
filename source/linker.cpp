
#include <llvm/Support/CommandLine.h>
#include <string>
#include <iostream>
#include <stdexcept>
#include "projectlinker.hpp"


int main(int args, char**);
void help();


using namespace llvm;
static cl::OptionCategory LinkerCategory("Linker Options", "Options for dtlu-linker.");
static cl::opt<std::string> OutputFilename("o", cl::desc("Specify output filename"), cl::value_desc("filename"), cl::cat(LinkerCategory));
static cl::opt<std::string> InputFilename(cl::Positional, cl::Required, cl::desc("<input file>"), cl::cat(LinkerCategory));
static cl::opt<std::string> ProjectDirname("p", cl::desc("Specify project directory"), cl::value_desc("directory"), cl::Required, cl::cat(LinkerCategory));

int main(int argc, char** argv)
{
    cl::HideUnrelatedOptions(LinkerCategory);
    cl::ParseCommandLineOptions(argc, argv);

    std::string llvm_linker("llvm-link");

    std::string on;
    if(OutputFilename.empty())
    {
        on = "output.bc";
    }
    else
    {
        on = OutputFilename;
    }
    std::string mainName = InputFilename;
    std::string project_dir = ProjectDirname;
    try
    {
        ProjectLinker project(mainName, project_dir, llvm_linker);
        project.linkModulesToFile(on);
        project.printNotFound();
    }
    catch (std::invalid_argument& e)
    {
        std::cerr << e.what() << std::endl;
        return 3;
    }
}


void help()
{
    std::cout << "Use: linker file [-a name]\n";
    std::cout << "file must be llvm bytecode\n";
}
