add_executable(forge forge.cpp)
target_link_libraries(
	forge
	PRIVATE
	forge_ppac
	forge_t0d1
	forge_t0d2
	forge_t0d3
	forge_t0d4
	forge_beam
	forge_trigger
	forge_ssd
	forge_t0csi
	forge_vme
	raw_trigger_event
	trigger_event
)

add_executable(align align.cpp)
target_link_libraries(align PRIVATE alignment trigger_event)

add_executable(map_vme map_vme.cpp)
target_link_libraries(
	map_vme
	PRIVATE
	tafd_event
	dssd_event
	csi_event
	beam_event
	ppac_event
	ROOT::RIO
	ROOT::Tree
	ROOT::Core
)
