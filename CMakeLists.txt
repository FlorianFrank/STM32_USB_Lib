cmake_minimum_required(VERSION 2.9)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake")

project(STM32USB_Lib C ASM)

file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include"
					"${CMAKE_CURRENT_SOURCE_DIR}/include/STMFiles")


if("${specs}" STREQUAL "rdimon.specs")
	message("Enable rdimon.specs")
	add_definitions(-DRDMON_SPECS=1)
endif() # disable syscalls.c

add_library(STM32_USB_Lib ${src_files})
