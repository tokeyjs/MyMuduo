cmake_minimum_required(VERSION 3.0)
project(newmuduo)

# 编译进去的源文件(搜索文件不能递归)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_FILES)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# 客户端源文件路径
set(CLIENT_SRC ${PROJECT_SOURCE_DIR}/client/client.cpp ${PROJECT_SOURCE_DIR}/src/TimeStamp.cpp)
# 服务端原文件路径
set(SERVER_SRC ${PROJECT_SOURCE_DIR}/server/server.cpp)

# 设置可执行文件存放路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


add_executable(server ${SERVER_SRC} ${SRC_FILES}) # 生成可执行文件
target_link_libraries(server pthread) # 添加其他库
target_include_directories(server PUBLIC ${PROJECT_SOURCE_DIR}/include) # 添加头文件路径

add_executable(client ${CLIENT_SRC}) # 生成可执行文件
target_include_directories(client PUBLIC ${PROJECT_SOURCE_DIR}/include) # 添加头文件路径
