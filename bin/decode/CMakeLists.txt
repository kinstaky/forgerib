add_executable(decode_raw decode_raw.cpp)
target_link_libraries(decode_raw raw_reader ROOT::RIO ROOT::Tree)

add_executable(decode_ppac decode_ppac.cpp)
target_link_libraries(decode_ppac PRIVATE decoder raw_ppac_event)

add_executable(decode_trigger decode_trigger.cpp)
target_link_libraries(decode_trigger PRIVATE decoder raw_trigger_event)

add_executable(build_ppac build_ppac.cpp)
target_link_libraries(build_ppac PRIVATE builder ROOT::RIO ROOT::Tree)