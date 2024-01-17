#top dir cmake project

cmake_minimum_required(VERSION 2.8)     # 指定最小版本
project (btle_all)                      # 设置项目名称

add_subdirectory(btle-tools)            # 引入一个子目录，此时子目录中也要有个 CmakeLists.txt

########################################################################
# Create uninstall target
########################################################################

configure_file(                         # 使用 cmake.in 模板生成 cmake 文件
    ${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
@ONLY)


add_custom_target(uninstall             # 添加一个没有输出的目标，以便始终构建它(add_custom_common 生成具有依赖项的文件)
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake   # ${CMAKE_COMMAND} 是 cmake 的完整路径
)
