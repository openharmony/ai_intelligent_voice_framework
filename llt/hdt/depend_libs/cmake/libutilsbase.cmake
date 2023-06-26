# Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
# Description: liblog.cmake
# Create: 2022-8-15

cmake_minimum_required(VERSION 3.13)
set(UTILSBASE_DIR ${ROOT_DIR}commonlibrary/c_utils/base/)

include_directories(${UTILSBASE_DIR}include)
include_directories(${UTILSBASE_DIR}src)
include_directories(${ROOT_DIR}kernel/linux-5.10-lts/make_output/usr/include/)

set(LIBRARY_OUTPUT_PATH ${LIB_PATH})

# TEST_SOURCE_FILES
aux_source_directory(${UTILSBASE_DIR}/src UTILSBASE_DIR_SRCS)

file(GLOB SRC_BLACK_LIST
    ${UTILSBASE_DIR}/src/file_ex.cpp)

list(REMOVE_ITEM UTILSBASE_DIR_SRCS ${SRC_BLACK_LIST})

add_library(utilsbase STATIC ${UTILSBASE_DIR_SRCS})
target_compile_options(utilsbase PRIVATE -include functional -includelimits.h)
set_target_properties(utilsbase PROPERTIES COMPILE_FLAGS "-w")