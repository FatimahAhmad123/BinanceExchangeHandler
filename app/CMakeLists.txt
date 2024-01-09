file(GLOB APP_SOURCES "app/*.cpp")

# Main executable
add_executable(${PROJECT_NAME} main.cpp)

# Add dependencies
add_dependencies(${PROJECT_NAME} BinanceHandler)

# Link libraries
target_link_libraries(${PROJECT_NAME} BinanceHandler)

find_package(rapidjson REQUIRED)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include ${RapidJSON_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}
)

# set_target_properties(${PROJECT_NAME} PROPERTIES
# RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
# )