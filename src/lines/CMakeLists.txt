include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(lines lines.cpp)
target_link_libraries(lines ghgl m GL GLU X11 boost_system)
