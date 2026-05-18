add_executable(gated_ppac gated_ppac.cpp)
target_link_libraries(gated_ppac PRIVATE config ppac_event beam_event ROOT::RIO ROOT::Tree ROOT::Graf ROOT::Gpad ROOT::Gui ROOT::Hist ROOT::Core)
