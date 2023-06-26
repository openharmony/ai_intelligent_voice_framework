# Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
# Description: liblog.cmake
# Create: 2022-8-15

cmake_minimum_required(VERSION 3.13)
set(HILOG_DIR ${ROOT_DIR}base/hiviewdfx/hilog/)
set(HILOG_SRC_DIR ${ROOT_DIR}base/hiviewdfx/hilog/frameworks/libhilog/)

include_directories(${ROOT_DIR}third_party/bounds_checking_function/include)
include_directories(${HILOG_DIR}frameworks/include/)
include_directories(${HILOG_DIR}frameworks/libhilog/include/)
include_directories(${HILOG_DIR}frameworks/libhilog/socket/include/)
include_directories(${HILOG_DIR}frameworks/libhilog/param/include/)
include_directories(${HILOG_DIR}frameworks/libhilog/utils/include/)
include_directories(${HILOG_DIR}frameworks/libhilog/vsnprintf/include/)
include_directories(${HILOG_DIR}interfaces/native/innerkits/include/)

set(LIBRARY_OUTPUT_PATH ${LIB_PATH})
set(MACRO_DEFINITION_LOG -D__LINUX__)

# TEST_SOURCE_FILES
file(GLOB LOG_SRCS
    ${HILOG_SRC_DIR}hilog.cpp
    ${HILOG_SRC_DIR}hilog_printf.cpp
    ${HILOG_SRC_DIR}utils/log_print.cpp
    ${HILOG_SRC_DIR}utils/log_utils.cpp
    ${HILOG_SRC_DIR}vsnprintf/vsnprintf_s_p.cpp)

add_library(hilog STATIC ${LOG_SRCS})
target_compile_definitions(hilog PRIVATE ${MACRO_DEFINITION_LOG})
set_target_properties(hilog PROPERTIES COMPILE_FLAGS "-w")