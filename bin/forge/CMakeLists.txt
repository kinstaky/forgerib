add_executable(forge forge.cpp)
target_link_libraries(
	forge
	PRIVATE
	colossus
	forge_ppac
	forge_t0d1
	forge_t0d2
	forge_t0d3
	forge_t0d4
	forge_beam
	forge_trigger
	forge_ssd
	forge_t0csi
	raw_trigger_event
)
