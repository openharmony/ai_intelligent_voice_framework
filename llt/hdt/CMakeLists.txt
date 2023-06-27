# Copyright (c) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.13)

project(hdt VERSION 1.0 LANGUAGES C CXX)

# Library file compilation
add_subdirectory(depend_libs/cmake)

# COMPONENT CMAKE
include("${CMAKE_CURRENT_SOURCE_DIR}/conf/comp.cmake")

# PATH
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../)
set(CLANG_DIR ${ROOT_DIR}prebuilts/clang/host/linux-x86/clang-r353983c/)

# COMPILE_OPTIONS
set(CMAKE_CXX_COMPILER "${CLANG_DIR}/bin/clang++")
set(CMAKE_C_COMPILER "${CLANG_DIR}/bin/clang")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

# INCLUDE_DIR
add_compile_options(-include memory)
include("${CMAKE_CURRENT_SOURCE_DIR}/conf/include.cmake")
include_directories(${INCLUDE_DIRECTORIES})


# TEST_SOURCE_FILES
aux_source_directory(${TEST_SRC_DIR}/src/ TEST_SRC)
aux_source_directory(${TEST_SRC_DIR}/src/statemgr TEST_SRC)


# TEST_CASE_FILES
aux_source_directory(testcase/intell_voice_utils TEST_CASE)

# STUB_FILE
aux_source_directory(stub STUB_FILE)

# TARGET
add_executable(intellvoicetest ${TEST_SRC} ${TEST_CASE} ${STUB_FILE} ${DEPEND_SRC})

add_dependencies(intellvoicetest hilog sec utilsbase)
target_link_libraries(intellvoicetest hilog sec utilsbase)

# MESSAGE
get_target_property(COMPILE_FLAGS intellvoicetest COMPILE_OPTIONS)
get_target_property(LINK_FLAGS intellvoicetest LINK_OPTIONS)
message(STATUS "Compiler id: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compile flags: ${COMPILE_FLAGS}")
message(STATUS "Link flags: ${LINK_FLAGS}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")