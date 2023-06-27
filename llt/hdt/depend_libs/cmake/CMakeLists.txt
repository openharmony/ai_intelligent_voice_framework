cmake_minimum_required(VERSION 3.13)

project(lib_cmake VERSION 1.0 LANGUAGES C CXX)

# CMAKE_WITH_ALL_COMPONENTS

# PATH
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../)
set(CLANG_DIR ${ROOT_DIR}prebuilts/clang/host/linux-x86/clang-r353983c/)
set(INTELL_VOICE_DIR ${ROOT_DIR}foundation/ai/intelligent_voice_framework/llt/hdt/)
set(LIB_PATH ${INTELL_VOICE_DIR}/build)

# COMPILE_OPTIONS
set(CMAKE_CXX_COMPILER "${CLANG_DIR}/bin/clang++")
set(CMAKE_C_COMPILER "${CLANG_DIR}/bin/clang")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
add_compile_options(-Wno-c++11-narrowing)

include("libhilog.cmake")
include("libsec.cmake")
include("libutilsbase.cmake")