cmake_minimum_required(VERSION 3.2)

project(Introspector-generator)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	message("Building with Clang.")
	set(CLANG ON)
	set(FUSE_LD_FLAG "-fuse-ld=lld")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	message("Building with AppleClang.")
	set(CLANG ON)
	set(APPLE_CLANG ON)
	set(FUSE_LD_OPT "")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	message("Building with gcc.")
	set(GCC ON)
endif()

if(CLANG)
	EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string )
	string (REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})

	message("Clang version: ${CLANG_VERSION_STRING}")
endif()

if(MSVC)
	if(CLANG)
		message("Building with Clang on Windows.")
	else()
		set(MSVC_SPECIFIC ON)
		message("Building with MSVC specifically.")
	endif()
endif()

if(UNIX)
	message("Building Introspector-generator for UNIX systems.")
elseif(MSVC)
	message("Building Introspector-generator for Windows systems.")
else()
	message("Unknown system.")
endif()
  
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

if(MSVC_SPECIFIC)
	set(CMAKE_EXE_LINKER_FLAGS
		"${CMAKE_EXE_LINKER_FLAGS} /LTCG"
	)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL")
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} /Ob2 /Oi /O2 /permissive- /std:c++latest"
	)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++1z")
endif()

if (CLANG AND NOT MSVC)
	set(USE_LIBCXX ON)
endif()

if(USE_LIBCXX)
	# Looks like stdlib=libc++ is duplicated for the linker as well,
	# so we only have to add it to CXX.
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

if (CLANG AND NOT MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FUSE_LD_OPT} ")
endif()

add_executable(Introspector-generator "src/main.cpp")

if(MSVC)

elseif(CLANG)
	if (CLANG_VERSION_STRING VERSION_LESS 9.0)
		message("Appending c++fs.")
		target_link_libraries(Introspector-generator c++fs)
	else()
		message("Omitting c++fs.")
		target_link_libraries(Introspector-generator)
	endif()
elseif(GCC)
	target_link_libraries(Introspector-generator stdc++fs)
endif()

if(MSVC_SPECIFIC)
	# if you want to run the program on examples/input.cfg,
	# ensure that you run it with "input.cfg" in the command arguments.

	set_target_properties(Introspector-generator 
		PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/example"
	)

	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Introspector-generator)
endif()