# dtlu
**Dependency Tracking Linker Utils**

##Install
```
cloned_repository$ mkdir build
cloned_repository$ cd build
cloned_repository/build$ cmake ..
cloned_repository/build$ sudo make install
```
building and indexing project - need clang
dtlu-project-maker -m path/to/Makefile -p project/build/directory

linking module - need llvm-link
input file - LLVM BC or LLVM IR, clang -c -emtt-llvm file.c -o file.bc
dtlu-linker file.bc -o output_file
