include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(grid grid.cpp)
target_link_libraries(grid ghgl m GL GLU X11 boost_system)
