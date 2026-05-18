add_executable(smelt smelt.cpp)
target_link_libraries(
	smelt
	PRIVATE
	config
	smelt_ppac
	smelt_t0d1
	smelt_t0d2
	smelt_t0d3
	smelt_t0d4
	smelt_beam
	smelt_ssd
	smelt_t0csi_bloom
	smelt_t0csi
	csi_event
	dssd_event
	tafd_event
	align_event
	trigger_event
	ROOT::Core
	ROOT::Hist
	ROOT::RIO
	ROOT::Tree
)

add_executable(sift sift.cpp)
target_link_libraries(sift PRIVATE sifting trigger_event config)
