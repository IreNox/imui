cmake_minimum_required(VERSION 3.16)

include(FetchContent)

project(ImUiSamples
	VERSION 1.0.0
	LANGUAGES C CXX
)

FetchContent_Declare(
	SDL2
	GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
	GIT_TAG release-2.32.10
)

FetchContent_Declare(
	glad
	GIT_REPOSITORY https://github.com/Dav1dde/glad.git
	GIT_TAG glad2
)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(
	SDL2
	glad
)

add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)
glad_add_library(glad REPRODUCIBLE API gl:core=4.6)

find_package(OpenGL REQUIRED)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/.. imui_lib)

add_executable(imui_samples
	src/00_samples.h
	src/01_hello_world.c
	src/02_layout.c
	src/03_input.c
	src/04_widgets.c
	src/05_cpp.cpp
	src/framework.c
	src/framework.h
	src/main.c
)

target_compile_definitions(imui_samples PRIVATE
	IMUI_SAMPELS_USE_GLAD
)

target_link_libraries(imui_samples
	PRIVATE
		ImUi::Lib
		ImUi::LibCpp
		SDL2::SDL2
		SDL2::SDL2main
		glad
		OpenGL::GL
)
