###########################################################################################
##
##    ____                      __      ____
##   /\  _`\   __             /'_ `\   /\  _`\
##   \ \ \L\_\/\_\    __  _  /\ \L\ \  \ \ \L\ \ _ __    ___
##    \ \  _\/\/\ \  /\ \/'\ \/_> _ <_  \ \ ,__//\`'__\ / __`\
##     \ \ \/  \ \ \ \/>  </   /\ \L\ \  \ \ \/ \ \ \/ /\ \L\ \
##      \ \_\   \ \_\ /\_/\_\  \ \____/   \ \_\  \ \_\ \ \____/
##       \/_/    \/_/ \//\/_/   \/___/     \/_/   \/_/  \/___/
##
##               Fix8Pro FIX Engine and Framework
##
## Copyright (C) 2010-22 Fix8 Market Technologies Pty Ltd (ABN 29 167 027 198)
## ALL RIGHTS RESERVED  https://www.fix8mt.com  heretohelp@fix8mt.com  @fix8mt
##
## This  file is released  under the  GNU LESSER  GENERAL PUBLIC  LICENSE  Version 3.  You can
## redistribute  it  and / or modify  it under the  terms of  the  GNU Lesser  General  Public
## License as  published  by  the Free  Software Foundation,  either version 3 of the License,
## or (at your option) any later version.
##
## This file is distributed in the hope that it will be useful, but  WITHOUT ANY WARRANTY  and
## without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
##
## You should  have received a copy of  the GNU Lesser General Public  License along with this
## file. If not, see <http://www.gnu.org/licenses/>.
##
## BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
## THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
## COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
## KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
## WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
## THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
## YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
##
## IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
## HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
## ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
## CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
## NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
## THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
## HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
##
#############################################################################################
cmake_minimum_required(VERSION 3.13)
set(CMAKE_VERBOSE_MAKEFILE FALSE)
project(fix8pro_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS NO)
if(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 9.0.0)
	message(FATAL_ERROR "Unsupported compiler version. Require gcc >= 9+; your version is ${CMAKE_CXX_COMPILER_VERSION}")
endif()

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Generate debug library name with a postfix.")
if ("${FIX8PRO_ROOT}" STREQUAL "")
	message(FATAL_ERROR "FIX8PRO_ROOT was not set, set it by adding -DFIX8PRO_ROOT=<path to fix8pro install dir>")
endif()
if ("${FIX8PRO_LICENSE_FILE}" STREQUAL "")
	message(FATAL_ERROR "FIX8PRO_LICENSE_FILE was not set, set it by adding -DFIX8PRO_LICENSE_FILE=<path to fix8pro license>")
endif()

message("-- fix8pro root === ${FIX8PRO_ROOT}")
include("${FIX8PRO_ROOT}/share/fix8pro/cmake/fix8pro-targets.cmake")
set(_IMPORT_PREFIX ${FIX8PRO_ROOT})
#include("${FIX8PRO_ROOT}/share/fix8pro/cmake/fix8pro-targets-relwithdebinfo.cmake")
include(cmake/macros.cmake)

# enable address sanitizer for tracing memory leaks
option(ENABLE_ADDRESS_SANITIZER "enable address sanitizer (default=no)" OFF)

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
add_gen_shared_library(FIX44_EXAMPLE schema/FIX44.xml -G ../../../schema/FIX44_en_phrases.xml -L -U --verbose --namespace FIX44_EXAMPLE -p FIX44_EXAMPLE)
target_link_libraries(FIX44_EXAMPLE PRIVATE fix8pro fix8proutils)

# shared FIX42 lib
# add_gen_shared_library(FIX42_EXAMPLE schema/FIX42.xml -G ../../../schema/FIX42_en_phrases.xml -L -U --verbose --namespace FIX42_EXAMPLE -p FIX42_EXAMPLE)
# target_link_libraries(FIX42_EXAMPLE PRIVATE fix8pro fix8proutils)

# static FIX50SP2 lib
# add_gen_static_library(FIX50SP2 schema/FIX50SP2.xml  -L --verbose --namespace FIX50SP2 -p FIX50SP2 --fixt ${CMAKE_CURRENT_SOURCE_DIR}/schema/FIXT11.xml)
# target_link_libraries(FIX50SP2 PRIVATE fix8pro fix8proutils)

add_executable(simpleclisrv src/simpleclisrv.cpp)
target_link_libraries(simpleclisrv PRIVATE FIX44_EXAMPLE fix8pro fix8proutils fix8pro_statapp)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/.install" CACHE PATH "install path" FORCE)
endif()
message("-- Installation path ${CMAKE_INSTALL_PREFIX}")
install(TARGETS simpleclisrv FIX44_EXAMPLE RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib/static)
install(DIRECTORY config DESTINATION share/fix8pro FILES_MATCHING PATTERN *.xml)

