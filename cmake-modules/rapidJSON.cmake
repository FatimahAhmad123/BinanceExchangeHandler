# rapidjson.cmake

include(ExternalProject)

ExternalProject_Add(
	rapidjson
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/rapidjson
	GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
	GIT_TAG "v1.1.0" # Use a specific tag or commit hash
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)

set(RapidJSON_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/rapidjson/src/rapidjson/include)
