
find_package(OpenSSL REQUIRED)

add_executable(UnitTests BinanceHandlerTests.cpp)

add_dependencies(UnitTests BinanceHandler)

target_link_libraries(UnitTests BinanceHandler gtest gtest_main OpenSSL::SSL
	OpenSSL::Crypto
	spdlog pthread
)

find_package(rapidjson REQUIRED)

# Include directories
target_include_directories(UnitTests PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/googletest/googletest/include ${CMAKE_SOURCE_DIR}/include ${RapidJSON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}
)