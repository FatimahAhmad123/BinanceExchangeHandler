# app/CMakeLists.txt

file(GLOB APP_SOURCES "*.cpp")

# Main executable
add_executable(app_executable ${APP_SOURCES})

# Link libraries
target_link_libraries(app_executable BinanceHandler)
