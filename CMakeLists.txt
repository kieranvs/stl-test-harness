cmake_minimum_required(VERSION 3.12)
project(STLTestHarness VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
	TestHarness
	src/counted_malloc.cpp
	src/main.cpp
	src/memory_correctness_item.cpp
)
#target_include_directories(TestHarness PUBLIC . src)