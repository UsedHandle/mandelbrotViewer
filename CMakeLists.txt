cmake_minimum_required(VERSION 3.12)

set(EXE_NAME "mandelbrotViewer")

project(${EXE_NAME} C CXX)
set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

FetchContent_Declare(
	glfw3
	GIT_REPOSITORY	https://github.com/glfw/glfw.git
	GIT_TAG		7b6aead9fb88b3623e3b3725ebb42670cbe4c579 # 3.4
)

set( GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only" )
set( GLFW_INSTALL OFF CACHE BOOL "GLFW lib only" )

FetchContent_MakeAvailable(glfw3)


find_package(OpenGL REQUIRED)



add_library(glad STATIC src/glad.c)
add_executable(${EXE_NAME} src/mandelbrot.cpp )

if(MSVC)
	target_compile_options(${EXE_NAME} PRIVATE /W4)
else()
	target_compile_options(${EXE_NAME} PRIVATE -Wall -Wextra -Wshadow -Wconversion -Wpedantic -O3)
	target_compile_options(glad PRIVATE -O3)
endif()

include_directories( ${OPENGL_INCLUDE_DIRS} )
include_directories(include/)
target_link_libraries(${EXE_NAME} glad glfw ${OPENGL_LIBRARIES} )
