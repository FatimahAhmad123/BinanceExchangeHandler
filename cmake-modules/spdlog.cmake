include(ExternalProject)

ExternalProject_Add(
	spdlog_external
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_TAG v1.9.2
	PREFIX ${CMAKE_BINARY_DIR}/external/spdlog
	INSTALL_COMMAND ""
)

ExternalProject_Get_Property(spdlog_external source_dir)

add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE ${source_dir}/include)
add_dependencies(spdlog spdlog_external)
