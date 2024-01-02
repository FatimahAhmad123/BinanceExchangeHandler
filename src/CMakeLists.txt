# src/CMakeLists.txt

# Include external projects
include(${CMAKE_SOURCE_DIR}/cmake-modules/boostBeast.cmake)

# Add your libraries here
add_library(BinanceHandler BinanceHandler.cpp)

# Link external libraries
target_include_directories(BinanceHandler PUBLIC ${BoostBeast_INCLUDE_DIRS})

# Include the headers from the 'include' folder
target_include_directories(BinanceHandler PUBLIC ${CMAKE_SOURCE_DIR}/include)
