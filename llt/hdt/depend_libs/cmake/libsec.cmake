# Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
# Description: liblog.cmake
# Create: 2022-8-15

cmake_minimum_required(VERSION 3.13)
set(SEC_DIR ${ROOT_DIR}third_party/bounds_checking_function/)

include_directories(${SEC_DIR}include)

set(LIBRARY_OUTPUT_PATH ${LIB_PATH})

set(MACRO_DEFINITION_LOG
      -D__LINUX__
      -D_INC_STRING_S
      -D_INC_WCHAR_S
      -D_SECIMP=//
      -D_STDIO_S_DEFINED
      -D_INC_STDIO_S
      -D_INC_STDLIB_S
      -D_INC_MEMORY_S)

# TEST_SOURCE_FILES
aux_source_directory(${SEC_DIR}/src SEC_SRCS)

add_library(sec STATIC ${SEC_SRCS})
target_compile_definitions(sec PRIVATE ${MACRO_DEFINITION_LOG})
set_target_properties(sec PROPERTIES COMPILE_FLAGS "-w")