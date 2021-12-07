cmake_minimum_required(VERSION 3.13)
set(CMAKE_VERBOSE_MAKEFILE FALSE)
project(examples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS NO)
if(${CMAKE_CXX_COMPILER_VERSION} MATCHES "^3.+" OR "${CMAKE_CXX_COMPILER_VERSION}" MATCHES "^4.+")
	message(ERROR "!!! unsupported compiler version, require >= GCC 5+, but only found ${CMAKE_CXX_COMPILER_VERSION}")
endif()

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Generate debug library name with a postfix.")
if ("${FIX8PRO_ROOT}" STREQUAL "")
	message(FATAL_ERROR "FIX8PRO_ROOT was not set, set it by added -DFIX8PRO_ROOT=<path to fix8pro install dir>")
endif()
if ("${FIX8PRO_LICENSE_FILE}" STREQUAL "")
	message(FATAL_ERROR "FIX8PRO_LICENSE_FILE was not set, set it by added -DFIX8PRO_LICENSE_FILE=<path to fix8pro license>")
endif()

message("-- fix8pro root === ${FIX8PRO_ROOT}")
include("${FIX8PRO_ROOT}/share/fix8pro/cmake/fix8pro-targets.cmake")
set(_IMPORT_PREFIX ${FIX8PRO_ROOT})
#include("${FIX8PRO_ROOT}/share/fix8pro/cmake/fix8pro-targets-relwithdebinfo.cmake")
include(cmake/macros.cmake)

# enable address sanitizer for tracing memory leaks
option(ENABLE_ADDRESS_SANITIZER    "enable address sanitizer (default=no)" OFF)

# gcc settings
set(${project_prefix}_COMPILER_SYMBOLS_VISIBILITY PRIVATE -fvisibility=hidden)
set(${project_prefix}_LINKER_SYMBOLS_VISIBILITY PRIVATE -Wl,--exclude-libs,ALL)
set(CMAKE_C_FLAGS "-fPIC")
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -ggdb")
set(CMAKE_CXX_FLAGS "-fPIC -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -D_DEBUG -ggdb -pthread")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -pthread")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -DNDEBUG -ggdb -pthread")
if (${ENABLE_ADDRESS_SANITIZER})
	# run with environment variable ASAN_OPTIONS=halt_on_error=0
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize-recover=address")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsanitize=address -fsanitize-recover=address")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fsanitize=address -fsanitize-recover=address")
	message("-- address sanitizer is ON")
endif()

include_directories(${FIX8PRO_ROOT}/include)
include_directories(${CMAKE_BINARY_DIR}/include)

# shared FIX44 lib
add_gen_shared_library(simple_FIX44 schema/FIX44.xml  -L --verbose --namespace FIX44 -p simple_FIX44)
	target_link_libraries(simple_FIX44 PRIVATE fix8pro fix8proutils)
# static FIX50SP2 lib
add_gen_static_library(simple_FIX50SP2 schema/FIX50SP2.xml  -L --verbose --namespace FIX50SP2 -p simple_FIX50SP2 --fixt ${CMAKE_CURRENT_SOURCE_DIR}/schema/FIXT11.xml)
	target_link_libraries(simple_FIX50SP2 PRIVATE fix8pro fix8proutils)
add_executable(simpleclisrv simpleclisrv.cpp)
	target_link_libraries(simpleclisrv PRIVATE simple_FIX44 simple_FIX50SP2 fix8pro fix8proutils fix8pro_statapp)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/.install" CACHE PATH "install path" FORCE)
endif()
message("-- installing to ${CMAKE_INSTALL_PREFIX}")
install(TARGETS simpleclisrv simple_FIX44 simple_FIX50SP2 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static)
install(DIRECTORY config DESTINATION share/fix8pro FILES_MATCHING PATTERN *.xml)

