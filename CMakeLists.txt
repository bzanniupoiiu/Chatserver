cmake_minimum_required(VERSION 3.0)
project(myself_chatserver)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#可执行文件
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)

include_directories(${PROJECT_SOURCE_DIR}/thirdparty)


#源文件搜索路径(加载子目录)，意味着这个目录下面还会有一个cmakelist
##告诉 CMake 进入当前目录下的 a 子目录
##在该子目录中寻找并执行另一个 CMakeLists.txt 文件
##这意味着 a 目录中确实会有源文件和对应的构建配置
add_subdirectory(src)