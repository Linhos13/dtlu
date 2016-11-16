#include "projectlinker.hpp"
#include "symbolmap.hpp"
#include "modulecutter.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>

ModuleCutter* ProjectLinker::getOrLoadModule(std::string file_name)
{
    ModuleCutter* module = findModuleByName(file_name);
    if(!module)
    {
        module = new ModuleCutter(file_name, this);
        modules.insert(module);
    }
    return module;
}

ProjectLinker::ProjectLinker(std::string main_module, std::string project_dir, std::string llvm_linker)
{
    std::string findex_file = joinPath(std::string(project_dir), ".function_index");
    functionMap = new LinkerSupports::SymbolMap(findex_file);
    std::string gindex_file = joinPath(std::string(project_dir), ".globals_index");
    globalMap = new LinkerSupports::SymbolMap(gindex_file);
    this->llvm_linker = llvm_linker;
    mainModuleName = main_module;
    ModuleCutter* main = getOrLoadModule(main_module);
    main->addAllDefinedFunctions();
    processModules();

}

ModuleCutter* ProjectLinker::findModuleByName(std::string file_name)
{
    auto it = find_if(modules.begin(), modules.end(), [file_name] (ModuleCutter* obj) {return obj->getName() == file_name;});
    if (it != modules.end())
    {
        return *it;
    }
    return nullptr;
}

ModuleCutter* ProjectLinker::getModule(llvm::Module* module)
{
    auto it = find_if(modules.begin(), modules.end(), [module] ( ModuleCutter* obj) {return obj->getModule() == module;});
    {
        return *it;
    }
    return nullptr;
}

void ProjectLinker::processModules()
{
    bool status = false;
    while(!status)
    {
        status = true;
        for(ModuleCutter* module : modules)
        {
            bool module_status = module->hasEmptyQueues();
            status = status && module_status;
            if(!module_status)
            {
                module->processQueues();
            }
        }
    }
}

void ProjectLinker::useExternalFunction(std::string name)
{
    std::vector<std::string*>* files = functionMap->getFiles(name);
    if(!files)
    {
        notFoundFunctions.insert(name);
    }
    else
    {
        if(files->size() == 1)
        {
            ModuleCutter* mc = getOrLoadModule(*((*files)[0]));
            mc->addFunction(name);
        }
        else
        {
            //TODO
            std::cerr << "more same named functions" << std::endl;
            std::exit(2);
        }
    }
}

void ProjectLinker::useExternalGlobal(std::string name)
{
    std::vector<std::string*>* files = globalMap->getFiles(name);
    if(!files)
    {
        notFoundGlobals.insert(name);
    }
    else
    {
        if(files->size() == 1)
        {
            ModuleCutter* mc = getOrLoadModule(*((*files)[0]));
            mc->addGlobal(name);
        }
        else
        {
            //TODO
            std::cerr << "more same named globals" << std::endl;
            std::exit(2);
        }
    }
}


void ProjectLinker::linkModulesToFile(std::string& file)
{
    std::string command(llvm_linker);
    for(ModuleCutter* module : modules)
    {
        module->cutUnused();
        module->writeToBC();
        command += " " + module->getName() + ".processed";
    }
    char* temp = std::tmpnam(nullptr);
    if(temp == nullptr)
    {
        std::cerr << "could not create tmp file";
        std::exit(3);
    }
    command += " -o " + file;
    command += " 2>&1 > " + std::string(temp);
    int status = std::system(command.c_str());
    if(status != 0)
    {
        std::ifstream tempfile;
        tempfile.open(temp);
        std::cerr << llvm_linker << " not exist or end with non 0 status" << std::endl;
        char output[100];
        if (tempfile.is_open())
        {
            while (!tempfile.eof())
            {
                tempfile >> output;
                std::cerr << output;
            }
        }
        std::exit(2);
    }
}

std::string ProjectLinker::joinPath(std::string path, std::string file)
{
    if(path[path.size()-1] != '/')
    {
        path += '/';
    }
    path += file;
    return path;
}

ProjectLinker::~ProjectLinker()
{
    for(ModuleCutter* mc : modules)
    {
        delete mc;
    }
    delete functionMap;
    delete globalMap;
}

void ProjectLinker::printNotFoundFunctions()
{
    std::cerr << "Not found functions: " << std::endl;
    for(std::string nm : notFoundFunctions)
    {
        std::cerr << nm << std::endl;
    }
}

void ProjectLinker::printNotFoundGlobals()
{
    std::cerr << "Not found globals: " << std::endl;
    for(std::string nm : notFoundGlobals)
    {
        std::cerr << nm << std::endl;
    }
}

void ProjectLinker::printNotFound()
{
    printNotFoundFunctions();
    printNotFoundGlobals();
}
