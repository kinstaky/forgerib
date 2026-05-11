add_executable(decode_raw decode_raw.cpp)
target_link_libraries(decode_raw raw_reader ROOT::RIO ROOT::Tree)

add_executable(decode_ppac decode_ppac.cpp)
target_link_libraries(decode_ppac PRIVATE decoder raw_ppac_event)

add_executable(decode_trigger decode_trigger.cpp)
target_link_libraries(decode_trigger PRIVATE decoder raw_trigger_event)

add_executable(decode_beam decode_beam.cpp)
target_link_libraries(decode_beam PRIVATE decoder raw_beam_event)

add_executable(decode_t0d1 decode_t0d1.cpp)
target_link_libraries(decode_t0d1 PRIVATE decoder raw_dssd_event)

add_executable(decode_t0d2 decode_t0d2.cpp)
target_link_libraries(decode_t0d2 PRIVATE decoder raw_dssd_event)

add_executable(decode_t0d3 decode_t0d3.cpp)
target_link_libraries(decode_t0d3 PRIVATE decoder raw_dssd_event)

add_executable(decode_t0d4 decode_t0d4.cpp)
target_link_libraries(decode_t0d4 PRIVATE decoder raw_dssd_event)

add_executable(decode_others decode_others.cpp)
target_link_libraries(decode_others PRIVATE decoder raw_csi_event raw_ssd_event)

add_executable(build_ppac build_ppac.cpp)
target_link_libraries(build_ppac PRIVATE builder ROOT::RIO ROOT::Tree)
