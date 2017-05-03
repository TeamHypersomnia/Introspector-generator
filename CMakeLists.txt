project(IntrospectorGenerator)

set(CMAKE_CXX_FLAGS
 "${CMAKE_CXX_FLAGS} /Ob2 /GL /Oi /O2"
)

set(CMAKE_EXE_LINKER_FLAGS
 "${CMAKE_EXE_LINKER_FLAGS} /LTCG"
)

add_executable(IntrospectorGenerator "src/main.cpp")

if(MSVC)
	# if you want to run the program on examples/input.cfg,
	# ensure that you run it with "input.cfg" in the command arguments.

	set_target_properties(IntrospectorGenerator 
		PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/example"
	)

	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT IntrospectorGenerator)
endif()
 