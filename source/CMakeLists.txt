cmake_minimum_required(VERSION 3.23)

# 项目名称
project(test)

# 指定c++标准
set(CMAKE_CXX_STANDARD 11)

# 编译选项
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCPPHTTPLIB_OPENSSL_SUPPORT")

# 引入库目录
link_directories(${PROJECT_SOURCE_DIR}/lib)
# target_link_directories(/usr/include/jsoncpp/json)
link_directories(/usr/include/jsoncpp/json)

# 包含头文件
include_directories(${PROJECT_SOURCE_DIR})

# 生成可执行程序cloud
add_executable(cloud cloud.cpp)
target_link_libraries(cloud
    ssl 
    crypto
    stdc++fs 
    jsoncpp 
    bundle
    pthread
)

# 生成可执行程序transfer
add_executable(transfer transfer.cpp)
target_compile_definitions(transfer PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
target_link_libraries(transfer 
    ssl 
    crypto 
    jsoncpp
)

# 指定可执行程序生成路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})