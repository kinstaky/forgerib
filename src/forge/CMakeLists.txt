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

add_library(forge_ssd forge_ssd.cpp)
target_link_libraries(forge_ssd PUBLIC silicon_event raw_ssd_event)

add_library(forge_t0csi forge_t0csi.cpp)
target_link_libraries(forge_t0csi PUBLIC csi_event raw_csi_event)

add_library(forge_t0d3 forge_t0d3.cpp)
target_link_libraries(forge_t0d3 PUBLIC dssd_event raw_dssd_event)

add_library(forge_t0d4 forge_t0d4.cpp)
target_link_libraries(forge_t0d4 PUBLIC dssd_event raw_dssd_event)

add_library(forge_vme forge_vme.cpp)
target_link_libraries(forge_vme PUBLIC vme_event ROOT::Graf)

add_library(alignment alignment.cpp)
target_link_libraries(alignment PUBLIC align_event ROOT::Graf)