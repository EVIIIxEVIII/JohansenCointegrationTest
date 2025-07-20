cmake_minimum_required(VERSION 3.15)
project(johansen_test LANGUAGES CXX)

file(GLOB_RECURSE JOHANSEN_TEST_SRC CONFIGURE_DEPENDS src/*.cpp)

add_executable(johansen_test ${JOHANSEN_TEST_SRC})

target_include_directories(johansen_test
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/vendor
)

target_compile_options(johansen_test PRIVATE
    -march=native
    -ffp-contract=fast
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)
