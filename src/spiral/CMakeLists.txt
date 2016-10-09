include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(spiral spiral.cpp)
target_link_libraries(spiral ghgl m GL GLU X11 boost_system)
