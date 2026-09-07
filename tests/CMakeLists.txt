cmake_minimum_required(VERSION 3.16)

project(ImUiTests
	VERSION 1.0.0
	LANGUAGES C CXX
)

include(CTest)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/.. imui_lib)

add_executable(imui_tests
	src/tests_base.c
	src/tests_data_types.c
	src/tests_input.c
	src/tests_surface.c
	src/tests_window.c
	src/tests_widget.c
	src/tests_layout.c
	src/main.c
)

target_link_libraries(imui_tests
	PRIVATE
		ImUi::Lib
		ImUi::LibCpp
)

if(BUILD_TESTING)
	add_test(NAME imui_tests COMMAND imui_tests)
endif()
