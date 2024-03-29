# CMakeList.txt: hms 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("cdplayer")

set(SIMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../hsm)

include_directories(
        ${SIMPLE_DIR}
        ${TARGET_DIR}
)

file(GLOB_RECURSE CDPLAYER ${SIMPLE_DIR}/*.c )
file(GLOB_RECURSE HSM ${TARGET_DIR}/*.c )

add_executable(cdplayer ${HSM} ${CDPLAYER})