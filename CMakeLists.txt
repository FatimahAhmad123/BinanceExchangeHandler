# CMakeLists.txt (root)

cmake_minimum_required(VERSION 3.12)

project(BinanceExchangeHandler)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/cmake-modules)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(app)

# Addthe following lines to include the tests directory
enable_testing()
add_subdirectory(unittesting)