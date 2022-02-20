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
## Copyright (C) 2010-21 Fix8 Market Technologies Pty Ltd (ABN 29 167 027 198)
## All Rights Reserved. [http://www.fix8mt.com] <heretohelp@fix8mt.com>
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
			${prefix}/${name}_phrases.cpp
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
				${prefix}/${name}_phrases.cpp
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
