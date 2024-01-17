# - Try to find the libhackrf library
# Once done this defines
#
#  LIBHACKRF_FOUND - system has libhackrf
#  LIBHACKRF_INCLUDE_DIR - the libhackrf include directory
#  LIBHACKRF_LIBRARIES - Link these to use libhackrf

# Copyright (c) 2013  Benjamin Vernoux
#


if (LIBHACKRF_INCLUDE_DIR AND LIBHACKRF_LIBRARIES)

  # in cache already
  set(LIBHACKRF_FOUND TRUE)

# find_package(PkgConfig) 是 CMake 中的一个命令，它用于查找和解析由 pkg-config 工具（一个用于管理库依赖关系的跨平台工具）生成的 .pc 文件。
# .pc 文件包含了库的相关信息，包括库的名称、头文件的路径、库的路径以及该库链接到其他库的方式等。当你在 CMakeLists.txt 文件中使用 find_package(PkgConfig) 时，CMake 会去查找这些 .pc 文件，并根据它们的信息来设置相应的变量，以便于后续的编译和链接操作。

# 例如，假设你有一个名为 libfoo 的库，并且它有一个 .pc 文件。如果你在 CMakeLists.txt 文件中使用 find_package(PkgConfig REQUIRED libfoo)，那么 CMake 就会去查找 libfoo.pc 文件，并根据该文件的信息来设置相应的变量，如 libfoo_INCLUDE_DIRS、libfoo_LIBRARIES 等。这样你就可以在后续的代码中使用这些变量来链接和使用 libfoo 库。

# 总的来说，find_package(PkgConfig) 是用于解析和管理库依赖关系的 CMake 命令。

else (LIBHACKRF_INCLUDE_DIR AND LIBHACKRF_LIBRARIES)
  IF (NOT WIN32)                                                # 非 windows
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PC_LIBHACKRF QUIET libhackrf)             # 和上面一个类似，最终找到 PC_LIBHACKRF_INCLUDEDIR .... _LIBRARY_DIRS ....
  ENDIF(NOT WIN32)

  FIND_PATH(LIBHACKRF_INCLUDE_DIR                               # 查找指定名称文件所在的目录，命令的结果是创建一条名称为 <var> 的缓存条目
    NAMES hackrf.h                                                  # 指定一个或多个文件名称
    HINTS $ENV{LIBHACKRF_DIR}/include ${PC_LIBHACKRF_INCLUDEDIR}    # 除了默认搜索位置之外，添加新的搜索位置
    PATHS /usr/local/include/libhackrf /usr/include/libhackrf /usr/local/include
    /usr/include ${CMAKE_SOURCE_DIR}/../libhackrf/src
    /opt/local/include/libhackrf
    ${LIBHACKRF_INCLUDE_DIR}
  )

  set(libhackrf_library_names hackrf)

  FIND_LIBRARY(LIBHACKRF_LIBRARIES                              # 查找 hrckrf 库    
    NAMES ${libhackrf_library_names}
    HINTS $ENV{LIBHACKRF_DIR}/lib ${PC_LIBHACKRF_LIBDIR}
    PATHS /usr/local/lib /usr/lib /opt/local/lib ${PC_LIBHACKRF_LIBDIR} ${PC_LIBHACKRF_LIBRARY_DIRS} ${CMAKE_SOURCE_DIR}/../libhackrf/src
  )

  if(LIBHACKRF_INCLUDE_DIR)
    set(CMAKE_REQUIRED_INCLUDES ${LIBHACKRF_INCLUDE_DIR})
  endif()

  if(LIBHACKRF_LIBRARIES)
    set(CMAKE_REQUIRED_LIBRARIES ${LIBHACKRF_LIBRARIES})
  endif()

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBHACKRF DEFAULT_MSG LIBHACKRF_LIBRARIES LIBHACKRF_INCLUDE_DIR)

  MARK_AS_ADVANCED(LIBHACKRF_INCLUDE_DIR LIBHACKRF_LIBRARIES)

endif (LIBHACKRF_INCLUDE_DIR AND LIBHACKRF_LIBRARIES)
