project(${package_name}-example)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${package_name}::${package_name}-lib
)

if(MINGW)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    stdc++exp
  )
endif()

target_sources(${PROJECT_NAME} PRIVATE
  src/main.cpp
)
