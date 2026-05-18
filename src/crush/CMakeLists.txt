add_library(raw_reader raw_reader.cpp)
target_include_directories(raw_reader PUBLIC ${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
target_link_libraries(raw_reader ROOT::Core)

add_library(crusher crusher.cpp)
target_link_libraries(crusher PUBLIC raw_reader)
