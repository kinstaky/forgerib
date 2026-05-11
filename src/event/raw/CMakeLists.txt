add_library(raw_ppac_event raw_ppac_event.cpp)
target_include_directories(raw_ppac_event PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(raw_ppac_event PUBLIC ROOT::RIO ROOT::Tree ROOT::Core)

add_library(raw_trigger_event raw_trigger_event.cpp)
target_include_directories(raw_trigger_event PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(raw_trigger_event PUBLIC ROOT::RIO ROOT::Tree ROOT::Core)

add_library(raw_beam_event raw_beam_event.cpp)
target_include_directories(raw_beam_event PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(raw_beam_event PUBLIC ROOT::RIO ROOT::Tree ROOT::Core)

add_library(raw_ssd_event raw_ssd_event.cpp)
target_include_directories(raw_ssd_event PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(raw_ssd_event PUBLIC ROOT::RIO ROOT::Tree ROOT::Core)

add_library(raw_csi_event raw_csi_event.cpp)
target_include_directories(raw_csi_event PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(raw_csi_event PUBLIC ROOT::RIO ROOT::Tree ROOT::Core)

add_library(raw_dssd_event raw_dssd_event.cpp)
target_include_directories(raw_dssd_event PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(raw_dssd_event PUBLIC ROOT::RIO ROOT::Tree ROOT::Core)
