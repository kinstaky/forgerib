add_library(colossus colossus.cpp)
target_include_directories(colossus PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(colossus ROOT::RIO ROOT::Core ROOT::Tree ROOT::Graf)

add_library(forge_ppac forge_ppac.cpp)
target_link_libraries(forge_ppac PUBLIC ppac_event raw_ppac_event)