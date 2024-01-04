cmake_minimum_required(VERSION 3.12)

project(BinanceExchangeHandler)

include(ExternalProject)

# Include BoostBeast configuration
include(cmake-modules/boostBeast.cmake)

find_package(OpenSSL REQUIRED)

# Include RapidJSON
include(cmake-modules/rapidJSON.cmake)

# Include spdlog
include(cmake-modules/spdlog.cmake)

file(GLOB APP_SOURCES "app/*.cpp")
file(GLOB SRC_SOURCES "src/*.cpp")
file(GLOB INCLUDE_HEADERS "include/*.h")

# Main executable
add_executable(${PROJECT_NAME} ${APP_SOURCES} ${SRC_SOURCES} ${INCLUDE_HEADERS})

add_dependencies(${PROJECT_NAME} Boost rapidjson spdlog) # Add RapidJSON and spdlog as dependencies

target_include_directories(${PROJECT_NAME}
	PRIVATE ${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${RapidJSON_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} pthread OpenSSL::SSL OpenSSL::Crypto spdlog)
