# cmake-modules/BoostBeast.cmake

include(ExternalProject)

ExternalProject_Add(
	Boost
	PREFIX ${CMAKE_BINARY_DIR}/boost
	GIT_REPOSITORY https://github.com/boostorg/boost.git
	GIT_TAG "boost-1.78.0"
	CONFIGURE_COMMAND ""
	BUILD_COMMAND bjam --with-date_time --with-system toolset=gcc variant=debug link=static install --prefix=${CMAKE_BINARY_DIR}/boostinstall
	BUILD_IN_SOURCE 1
	INSTALL_COMMAND ""
)

ExternalProject_Get_Property(Boost SOURCE_DIR)
set(Boost_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/boostinstall/include CACHE INTERNAL "")
set(Boost_LIBRARIES
	${CMAKE_BINARY_DIR}/boostinstall/lib/libboost_date_time.a
	${CMAKE_BINARY_DIR}/boostinstall/lib/libboost_system.a
)
