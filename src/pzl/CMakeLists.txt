include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(pzl pzl.cpp)
target_link_libraries(pzl ghgl m GL GLU X11 boost_thread boost_system)
