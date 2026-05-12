add_executable(forge forge.cpp)
target_link_libraries(
	forge
	PRIVATE
	colossus
	forge_ppac
	forge_t0d1
	forge_t0d2
	forge_beam
	forge_trigger
	raw_trigger_event
)
