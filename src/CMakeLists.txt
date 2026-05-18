add_library(config config.cpp)
target_include_directories(config PUBLIC ${PROJECT_SOURCE_DIR})

add_library(coke_trigger coke/coke_trigger.cpp)
target_include_directories(coke_trigger PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(coke_trigger PUBLIC trigger_event raw_trigger_event ROOT::Core ROOT::Hist ROOT::RIO ROOT::Tree)

add_subdirectory(event)
add_subdirectory(crush)
add_subdirectory(smelt)
