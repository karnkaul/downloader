cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

set(package_name downloader)
set(build_version 0.1.3)

project(${package_name} VERSION ${build_version})

set(vendor_curl ON)

if(TARGET CURL::libcurl)
  set(vendor_curl OFF)
endif()

option(DOWNLOADER_VENDOR_CURL "downloader: vendor CURL" ${vendor_curl})
option(DOWNLOADER_BUILD_EXAMPLE "Build downloader example" ${PROJECT_IS_TOP_LEVEL})

add_subdirectory(ext)
add_subdirectory(lib)

if(DOWNLOADER_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()
