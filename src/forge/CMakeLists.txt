add_library(colossus colossus.cpp)
target_include_directories(colossus PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(colossus ROOT::RIO ROOT::Core ROOT::Tree ROOT::Graf)

add_library(forge_ppac forge_ppac.cpp)
target_link_libraries(forge_ppac PUBLIC ppac_event raw_ppac_event)

add_library(forge_t0d1 forge_t0d1.cpp)
target_link_libraries(forge_t0d1 PUBLIC dssd_event raw_dssd_event)

add_library(forge_t0d2 forge_t0d2.cpp)
target_link_libraries(forge_t0d2 PUBLIC dssd_event raw_dssd_event)

add_library(forge_beam forge_beam.cpp)
target_link_libraries(forge_beam PUBLIC beam_event raw_beam_event)

add_library(forge_trigger forge_trigger.cpp)
target_link_libraries(forge_trigger PUBLIC trigger_event raw_trigger_event)
