cmake_minimum_required(VERSION 3.2)

project(Introspector-generator)

if(UNIX)
	message("Building Introspector-generator for UNIX systems.")
elseif(MSVC)
	message("Building Introspector-generator for Windows systems.")
else()
	message("Unknown system.")
endif()
  
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

if(UNIX)
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} -std=gnu++1z -static -s"
	)
elseif(MSVC)
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} /Ob2 /GL /Oi /O2 /permissive- /std:c++latest"
	)
	
	set(CMAKE_EXE_LINKER_FLAGS
		"${CMAKE_EXE_LINKER_FLAGS} /LTCG"
	)
endif()

add_executable(Introspector-generator "src/main.cpp")

if(UNIX)
	target_link_libraries(Introspector-generator stdc++fs)
endif()

if(MSVC)
	# if you want to run the program on examples/input.cfg,
	# ensure that you run it with "input.cfg" in the command arguments.

	set_target_properties(Introspector-generator 
		PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/example"
	)

	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Introspector-generator)
endif()