cmake_minimum_required(VERSION 3.28)
project(audio_client)

set(CMAKE_CXX_STANDARD 17)

add_definitions("-g")

file(GLOB_RECURSE SRC_FILES src/*.cpp)
file(GLOB_RECURSE SRC_FILES_CC src/*.cc)

# 头文件
include_directories(lib/include)
include_directories(src)
include_directories(src/utils)

# 添加环境宏
add_definitions(-DPRIOJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/")
add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)

# 检查操作系统
if ((CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin"))
  # macOS
  set(OS_IS_APPLE TRUE)
  add_definitions(-D_HOST_APPLE_)
  add_definitions(-D_CODEMODE_UTF8_)
elseif(CMAKE_HOST_WIN32)
  # Windows
  set(OS_IS_WINDOWS TRUE)
  add_definitions(-D_HOST_WINDOWS_)
  add_definitions(-D_CODEMODE_GBK_)

  # windows该干的事
  if (NOT MODULES_DIR)
    # boost
    include_directories($ENV{MODULES_DIR}/boost/include)
    link_directories($ENV{MODULES_DIR}/boost/lib)

    # openssl
    include_directories($ENV{MODULES_DIR}/openssl/include)
    link_directories($ENV{MODULES_DIR}/openssl/lib)

  else()
    # boost
    include_directories(${MODULES_DIR}/boost/include)
    link_directories(${MODULES_DIR}/boost/lib)

    # openssl
    include_directories(${MODULES_DIR}/openssl/include)
    link_directories(${MODULES_DIR}/openssl/lib)

  endif()

elseif(CMAKE_HOST_UNIX)
  # Linux
  set(OS_IS_LINUX TRUE)
  add_definitions(-D_HOST_LINUX_)
  add_definitions(-D_CODEMODE_UTF8_)
endif()

# 检查指针大小, 32位还是64位
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  add_definitions(-D_X64_)
elseif (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
  add_definitions(-D_X86_)
else()
  message(FATAL_ERROR "Unknown pointer size: ${CMAKE_SIZEOF_VOID_P}")
endif()

link_directories(lib/lib)

add_executable(audio_client ${SRC_FILES} ${SRC_FILES_CC} main.cpp)

target_link_libraries(audio_client -lportaudio)

if (OS_IS_WINDOWS)
  target_link_libraries(audio_client -lWinmm  libcrypto libssl)
elseif (OS_IS_LINUX)
  target_link_libraries(audio_client -lpthread -lcrypto -lssl)
endif ()