add_library(ppac_event ppac_event.cpp)
target_include_directories(ppac_event PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(ppac_event PUBLIC ROOT::Core ROOT::Tree ROOT::RIO)