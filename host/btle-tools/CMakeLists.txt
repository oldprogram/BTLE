# Copyright 2012 Jared Boone
# Copyright 2013 Benjamin Vernoux
#
# This file is part of HackRF and bladeRF
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

# Based heavily upon the libftdi cmake setup.

cmake_minimum_required(VERSION 2.8)                             # 最小版本依赖
project(btle-tools C)                                           # 设置工程名字，使用 C 语言 
set(MAJOR_VERSION 0)                                            # 设置主版本号变量
set(MINOR_VERSION 5)                                            # 设置次版本号变量
set(PACKAGE btle-tools)                                         # 设置 PCAKAGE 变量
set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION})           # 设置版本字符串变量
set(VERSION ${VERSION_STRING})                                  # 设置 VERSION 变量
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake/modules)   # 设置 CMAKE_MODULE_PATH 变量，指定要加载的 CMake 模块的搜索路径include()或find_package()

if(MSVC)                                                        # 如果为 true 表示用 Microsoft Visual C++    
# include_directories(getopt)
add_definitions(/D _CRT_SECURE_NO_WARNINGS)
else()
add_definitions(-Wall)                                          # 增加编译源码的编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu90")            # 给 CMAKE_<LANG>_FLAGS  编译 flag 全局变量增加一个 gnu90
endif()

# Find needed packages.
IF (USE_BLADERF MATCHES 1)                                      # cmake ../ -DUSE_BLADERF=1   (only for bladeRF)
  MESSAGE(STATUS "Use BLADERF")
  find_package(LIBBLADERF REQUIRED)
  include_directories(${LIBBLADERF_INCLUDE_DIR})
  set(USE_RFBOARD "USE_BLADERF")
ELSE (USE_BLADERF MATCHES 1)                                    # 如果外设使用 HACKRF
  MESSAGE(STATUS "Use HACKRF")
  find_package(LIBHACKRF REQUIRED)                                  # 在 CMAKE_MODULE_PATH 搜索空间搜索 Find<PackageName>.cmake, 读取并处理
  include_directories(${LIBHACKRF_INCLUDE_DIR})                     # 增加头文件的搜索目录 
  set(USE_RFBOARD "USE_HACKRF")
ENDIF (USE_BLADERF MATCHES 1)                                   # cmake ../                   (default. for HackRF)

CONFIGURE_FILE (                                                # 根据 .h 模板生成真正 .h (主要有一个 USE_RFBOARD 的宏)
  "${PROJECT_SOURCE_DIR}/include/common.h.in"
  "${PROJECT_SOURCE_DIR}/src/common.h"
)

add_subdirectory(src)                                           # 增加一个子目录

########################################################################
# Create uninstall target
########################################################################

if(NOT btle_all_SOURCE_DIR)
configure_file(
    ${PROJECT_SOURCE_DIR}/../cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
@ONLY)


add_custom_target(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
endif()

########################################################################
# Install udev rules
########################################################################
option(INSTALL_UDEV_RULES "Install udev rules for HackRF" OFF)
if (INSTALL_UDEV_RULES)
    install (
        FILES 52-hackrf.rules
        DESTINATION "/etc/udev/rules.d"
        COMPONENT "udev"
        )
else (INSTALL_UDEV_RULES)
    message (STATUS "Udev rules not being installed, install them with -DINSTALL_UDEV_RULES=ON")
endif (INSTALL_UDEV_RULES)
