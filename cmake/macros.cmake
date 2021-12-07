macro(add_gen_library shared name xml extra_fields)
	if ("${extra_fields}" STREQUAL "_")
		set(args ${ARGV4} ${ARGV5} ${ARGV6} ${ARGV7} ${ARGV8} ${ARGV9} ${ARGV10} ${ARGV11} ${ARGV12} ${ARGV13} ${ARGV14} ${CMAKE_SOURCE_DIR}/${xml})
	else()
		set(args ${ARGV4} ${ARGV5} ${ARGV6} ${ARGV7} ${ARGV8} ${ARGV9} ${ARGV10} ${ARGV11} ${ARGV12} ${ARGV13} ${ARGV14} ${CMAKE_SOURCE_DIR}/${xml} -F ${extra_fields})
	endif()
	set(prefix ${CMAKE_BINARY_DIR}/generated/${name})
	file(MAKE_DIRECTORY ${prefix})

	string(FIND "${xml}" "/" has_path_delimiters)
	if (${has_path_delimiters} EQUAL -1)
		set(xml ${CMAKE_SOURCE_DIR}/${xml})
	endif()
	message("-- Using schema ${xml}")
	add_custom_command(
			OUTPUT
			${prefix}/${name}_classes.cpp
			${prefix}/${name}_traits.cpp
			${prefix}/${name}_types.cpp
			${prefix}/${name}_classes.hpp
			${prefix}/${name}_types.hpp
			COMMAND ${CMAKE_COMMAND} -E env FIX8PRO_LICENSE_FILE=${FIX8PRO_LICENSE_FILE} LD_LIBRARY_PATH=${FIX8PRO_ROOT}/lib ${FIX8PRO_ROOT}/bin/f8pc ${args}
			MAIN_DEPENDENCY ${xml}
			WORKING_DIRECTORY ${prefix}
			VERBATIM)
	if ("${shared}" STREQUAL "shared")
		set(libname ${name})
		add_library(${libname} SHARED
				${prefix}/${name}_classes.cpp
				${prefix}/${name}_traits.cpp
				${prefix}/${name}_types.cpp
				${prefix}/${name}_classes.hpp
				${prefix}/${name}_types.hpp
				)
		target_compile_definitions(${libname} PRIVATE BUILD_F8_${name}_API)
	else()
		set(libname ${name})
		add_library(${libname} STATIC
				${prefix}/${name}_classes.cpp
				${prefix}/${name}_traits.cpp
				${prefix}/${name}_types.cpp
				${prefix}/${name}_classes.hpp
				${prefix}/${name}_types.hpp
				)
	endif()
	target_include_directories(${libname} PUBLIC ${prefix})
	target_compile_options(${libname} PRIVATE -fno-var-tracking -fno-var-tracking-assignments)
	target_link_libraries(${libname} PUBLIC fix8pro fix8proutils)
endmacro()

macro(add_gen_shared_library name xml)
	add_gen_library(shared ${name} ${xml} "_" ${ARGV2} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6} ${ARGV7} ${ARGV8} ${ARGV9} ${ARGV10} ${ARGV11} ${ARGV12} ${ARGV13} ${ARGV14})
endmacro()

macro(add_gen_static_library name xml)
	add_gen_library(static ${name} ${xml} "_" ${ARGV2} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6} ${ARGV7} ${ARGV8} ${ARGV9} ${ARGV10} ${ARGV11} ${ARGV12} ${ARGV13} ${ARGV14})
endmacro()
