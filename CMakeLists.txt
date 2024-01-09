# CMakeLists.txt (root)

cmake_minimum_required(VERSION 3.12)

project(BinanceExchangeHandler)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/cmake-modules)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(app)
