add_subdirectory(crush)
add_subdirectory(smelt)
add_subdirectory(tune)

add_executable(mine mine.cpp)
target_link_libraries(mine PRIVATE config raw_reader ROOT::RIO ROOT::Tree)

add_executable(crush_vme crush_vme.cpp)
target_link_libraries(
	crush_vme
	PRIVATE
	config
	tafd_event
	dssd_event
	csi_event
	beam_event
	ppac_event
	ROOT::RIO
	ROOT::Tree
	ROOT::Core
)

add_executable(coke coke.cpp)
target_link_libraries(coke PRIVATE config coke_trigger raw_trigger_event ROOT::Core ROOT::RIO ROOT::Tree)
