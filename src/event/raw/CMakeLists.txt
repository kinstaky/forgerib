add_library(raw_ppac_event raw_ppac_event.cpp)
target_include_directories(raw_ppac_event PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(raw_ppac_event PUBLIC ROOT::RIO ROOT::Tree ROOT::Core)

add_library(raw_trigger_event raw_trigger_event.cpp)
target_include_directories(raw_trigger_event PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(raw_trigger_event PUBLIC ROOT::RIO ROOT::Tree ROOT::Core)
