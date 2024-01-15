# Include external projects
include(${CMAKE_SOURCE_DIR}/cmake-modules/boostBeast.cmake)
include(${CMAKE_SOURCE_DIR}/cmake-modules/rapidjsonConfig.cmake)
include(${CMAKE_SOURCE_DIR}/cmake-modules/spdlog.cmake)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Add your libraries here
add_library(BinanceHandler
	HttpRequest.cpp
	JSONParser.cpp
	QueryHandler.cpp
)

# Link external libraries
target_include_directories(BinanceHandler
	PRIVATE ${Boost_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/include
	${RapidJSON_INCLUDE_DIRS}
)


add_dependencies(BinanceHandler Boost rapidjson spdlog)

# Link libraries
target_link_libraries(BinanceHandler
	${Boost_LIBRARIES}
	OpenSSL::SSL
	OpenSSL::Crypto
	spdlog
	pthread
)
