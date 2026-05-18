add_library(trigger_event trigger_event.cpp)
target_include_directories(trigger_event PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(trigger_event PUBLIC ROOT::Core ROOT::Tree ROOT::RIO)
