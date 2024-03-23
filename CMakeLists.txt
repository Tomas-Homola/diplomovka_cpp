
cmake_minimum_required(VERSION 3.16.0)

PROJECT(Diplomovka_TH VERSION 1.0.0 LANGUAGES CXX)

#Tu nastavit verziu Qt6
set(Qt6_Version 6.3.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_definitions( "/D_CRT_SECURE_NO_WARNINGS /MP /openmp" )
endif(MSVC)


enable_testing()

add_subdirectory(src)
