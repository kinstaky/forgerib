add_executable(search_cycle search_cycle.cpp)
target_include_directories(search_cycle PRIVATE ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(search_cycle PRIVATE ROOT::Tree ROOT::RIO ROOT::Graf)