add_subdirectory(decode)
add_subdirectory(forge)
add_subdirectory(tune)

add_executable(fuse fuse.cpp)
target_link_libraries(fuse PRIVATE fuse_t0csi fuse_t1dssd fuse_t1csi)