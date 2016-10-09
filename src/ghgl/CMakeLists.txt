include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(ghgl SHARED ghgl.cpp)
target_link_libraries(ghgl m GL GLU X11)
