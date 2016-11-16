

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
set(CMAKE_BUILD_TYPE Debug)

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS} ${dtlu_SOURCE_DIR}/include)
add_definitions(${LLVM_DEFINITIONS})

# Now build our tools
add_executable(dtlu-indexer function_indexer.cpp)
add_executable(dtlu-linker linker.cpp)
add_library(LinkerSupports STATIC linkersupports.cpp)
add_library(SymbolMap STATIC symbolmap.cpp)
add_library(ModuleCutter STATIC modulecutter.cpp)
add_library(ProjectLinker STATIC projectlinker.cpp)


llvm_map_components_to_libnames(llvm_libs support core irreader bitwriter)

# Link against LLVM libraries
target_link_libraries(dtlu-linker ProjectLinker)
target_link_libraries(dtlu-indexer ${llvm_libs} LinkerSupports)
target_link_libraries(ModuleCutter ${llvm_libs} LinkerSupports)
target_link_libraries(ProjectLinker ModuleCutter SymbolMap)
target_link_libraries(LinkerSupports ${llvm_libs})

install(
    TARGETS dtlu-linker dtlu-indexer
    DESTINATION /usr/bin
)

install(
    FILES stcc dtlu-project-maker
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    DESTINATION /usr/bin
)
