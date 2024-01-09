# # rapidjson.cmake

# include(ExternalProject)

# ExternalProject_Add(
# rapidjson
# PREFIX ${CMAKE_CURRENT_BINARY_DIR}/rapidjson
# GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
# GIT_TAG "v1.1.0" # Use a specific tag or commit hash
# CONFIGURE_COMMAND ""
# BUILD_COMMAND ""
# INSTALL_COMMAND ""
# )

# set(RapidJSON_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/rapidjson/src/rapidjson/include/)

include(ExternalProject)

if(NOT TARGET rapidjson)
	ExternalProject_Add(rapidjson
		URL https://github.com/Tencent/rapidjson/archive/refs/tags/v1.1.0.tar.gz
		PREFIX ${CMAKE_CURRENT_BINARY_DIR}/rapidjson
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
endif()

ExternalProject_Get_Property(rapidjson SOURCE_DIR)
set(RapidJSON_INCLUDE_DIRS ${SOURCE_DIR}/include)
message(${RapidJSON_INCLUDE_DIRS})