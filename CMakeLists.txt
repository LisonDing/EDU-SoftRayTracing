cmake_minimum_required(VERSION 3.10)
project(ONERT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 高警告等级
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 头文件路径
include_directories(include)

# 第三方库路径
include_directories(external)

# 源文件路径
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(ONERT ${SOURCES})