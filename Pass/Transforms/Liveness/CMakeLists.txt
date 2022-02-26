set(CMAKE_CXX_STANDARD 14)
cmake_minimum_required(VERSION 3.9)
SET (CMAKE_CXX_FLAGS "-fno-rtti -fPIC")

find_package(LLVM REQUIRED CONFIG PATHS /usr/lib/llvm-13 NO_DEFAULT_PATH)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_library(LLVMLivenessCheckPass MODULE LivenessCheck.cpp)
set_target_properties(LLVMLivenessCheckPass PROPERTIES COMPILE_FLAGS "-D__GLIBCXX_USE_CXX11_ABI=0 ")
#SET(CMAKE_MODULE_LINKER_FLAGS "-undefined dynamic_lookup")

target_link_libraries(LLVMLivenessCheckPass)


