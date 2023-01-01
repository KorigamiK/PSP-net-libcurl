cmake_minimum_required(VERSION 3.14)

project(
	curl_test
	VERSION 0.1.0
	DESCRIPTION "PSP cURL test"
	LANGUAGES CXX
)

add_executable(${PROJECT_NAME} main.cpp)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (PLATFORM_PSP)

	set(BUILD_PRX 1)

	target_link_libraries(${PROJECT_NAME} PRIVATE
	pspge
	pspwlan
	pspnet
	pspnet_apctl
	pspnet_resolver
	psphttp
	  pspdebug	
		pspdisplay
		curl
	)

	create_pbp_file(
		TARGET ${PROJECT_NAME}
		ICON_PATH NULL
		BACKGROUND_PATH NULL
		PREVIEW_PATH NULL
		TITLE ${PROJECT_DESCRIPTION}
	)

else()

	message(FATAL_ERROR "This project can only be built for the PSP")

endif()
