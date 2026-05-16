add_library(fuse_t0csi fuse_t0csi.cpp)
target_link_libraries(fuse_t0csi PUBLIC csi_event align_event)

add_library(fuse_t1dssd fuse_t1dssd.cpp)
target_link_libraries(fuse_t1dssd PUBLIC dssd_event align_event)

add_library(fuse_t1csi fuse_t1csi.cpp)
target_link_libraries(fuse_t1csi PUBLIC csi_event align_event)