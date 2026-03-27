cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

include(version.cmake)
set(package_name kcurl)
set(build_version ${kcurl_version})

project(${package_name} VERSION ${build_version})

option(KCURL_BUILD_EXAMPLE "Build kcurl example" ${PROJECT_IS_TOP_LEVEL})
set(KCURL_LIBCURL_LINK_TARGET "" CACHE STRING "Custom CMake target to link to for libcurl (and its headers)")
option(KCURL_CI_NO_LIBCURL "Do not use libcurl at all (speed up CI builds)" OFF)

if(KCURL_CI_NO_LIBCURL)
  message(STATUS "[kcurl] not vendoring or linking to libcurl")
  set(vendor_libcurl OFF)
  set(curl_link_target "")
elseif(NOT "${KCURL_LIBCURL_LINK_TARGET}" STREQUAL "")
  message(STATUS "[kcurl] not vendoring libcurl, linking to ${KCURL_LIBCURL_LINK_TARGET}")
  set(vendor_libcurl OFF)
  set(curl_link_target ${KCURL_LIBCURL_LINK_TARGET})
else()
  message(STATUS "[kcurl] vendoring libcurl and linking to CURL::libcurl")
  set(vendor_libcurl ON)
  set(curl_link_target CURL::libcurl)
endif()

add_subdirectory(ext)
add_subdirectory(lib)

if(KCURL_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()
