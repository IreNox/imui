cmake_minimum_required(VERSION 3.16)

project(ImUi
	VERSION 0.8.0
	LANGUAGES C CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(imui
	src/imui.c
	src/imui_data_types.c
	src/imui_draw.c
	src/imui_draw.h
	src/imui_font.c
	src/imui_font.h
	src/imui_helpers.c
	src/imui_helpers.h
	src/imui_input.c
	src/imui_input.h
	src/imui_internal.h
	src/imui_memory.c
	src/imui_memory.h
	src/imui_text.c
	src/imui_text.h
	src/imui_toolbox.c
	src/imui_types.h
	src/stb_truetype.h
)

add_library(imuicpp
	src/imui_cpp.cpp
)

add_library(ImUi::Lib ALIAS imui)
add_library(ImUi::LibCpp ALIAS imuicpp)

target_include_directories(imui
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_include_directories(imuicpp
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_compile_features(imui PUBLIC c_std_11 )
target_compile_features(imuicpp PUBLIC cxx_std_14 )

target_link_libraries(imuicpp
	PUBLIC
		imui
)

#
# Installation
#

install(TARGETS imui imuicpp
	EXPORT ImUiTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
	DIRECTORY include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

#
# Export targets
#

install(EXPORT ImUiTargets
	FILE ImUiTargets.cmake
	NAMESPACE ImUi::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imui
)

#
# Package config
#

configure_package_config_file(
	cmake/ImUiConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/ImUiConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imui
)

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/ImUiConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/ImUiConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/ImUiConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imui
)
