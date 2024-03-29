# CMakeList.txt: hms 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("demo")

add_subdirectory(cdplayer)
add_subdirectory(simplestate)
add_subdirectory(web)
