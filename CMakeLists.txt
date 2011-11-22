cmake_minimum_required (VERSION 2.6)
project (WW)
set (WW_VERSION_MAJOR 0)
set (WW_VERSION_MINOR 1)

configure_file ("${PROJECT_SOURCE_DIR}/ww_config.h.in"
    "${PROJECT_BINARY_DIR}/ww_config.h")
include_directories (${PROJECT_BINARY_DIR})

find_package (PkgConfig)

#
# OpenSolaris has libncurses.so in /usr/gnu/lib.
#
if (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
	set (CMAKE_LIBRARY_PATH "/usr/gnu/lib")
	set (CMAKE_INCLUDE_PATH "/usr/include/ncurses")
endif (${CMAKE_SYSTEM_NAME} MATCHES "SunOS")

set (CURSES_NEED_NCURSES TRUE)
find_package (Curses)

#
# GNT absolutely, positively needs ncurses. Unfortunately, the FindCurses macro
# can get confused between old curses and ncurses when both are installed on
# the same system. In part, this is due to FindCurses relying on the detection
# of wsyncup() in a shared object's symbols to ascertain whether it is or isn't
# ncurses. On Solaris at least, this detection is flawed as /lib/libcurses.so,
# which is NOT ncurses, does have the symbol. Thus, we'll need to complement the
# logic used in FindCurses to make sure that the proper header files and shared
# objects are used.
#
if (NOT CURSES_NCURSES_LIBRARY)
	message (FATAL_ERROR "NCurses is required for ${CMAKE_PROJECT_NAME}")
endif (NOT CURSES_NCURSES_LIBRARY)

#
# Use libncurses.so's path in the link stage
#
get_filename_component (CURSES_LINK_PATH ${CURSES_NCURSES_LIBRARY} PATH)
link_directories (${CURSES_LINK_PATH})
message (STATUS "\tUsing ${CURSES_NCURSES_LIBRARY}, linking against "
   "${CURSES_LINK_PATH}")

#
# NCurses will install a ncurses.h file in a include directory. Only ncurses
# does this and so we know we have ncurses when we find a ncurses.h file
#
if (CURSES_HAVE_NCURSES_H OR CURSES_HAVE_NCURSES_NCURSES_H)

	#
	# FindCurses found at least one ncurses.h file
	#
	if (CURSES_HAVE_NCURSES_NCURSES_H)

		#
		# We know we have a include/ncurses/ directory. Include it and
		# its parent.
		#
		get_filename_component (CURSES_INCLUDE_PATH
		    ${CURSES_HAVE_NCURSES_NCURSES_H} PATH)
		set (CURSES_INCLUDE_PATH ${CURSES_INCLUDE_PATH}/..
		    ${CURSES_INCLUDE_PATH})
	else (CURSES_HAVE_NCURSES_NCURSES_H)

		#
		# We only have CURSES_HAVE_NCURSES
		#
		if (CURSES_HAVE_NCURSES_H MATCHES "ncurses/ncurses.h")
			get_filename_component (CURSES_INCLUDE_PATH
			    ${CURSES_HAVE_NCURSES_H} PATH)
			set (CURSES_INCLUDE_PATH ${CURSES_INCLUDE_PATH}/..
			    ${CURSES_INCLUDE_PATH})
		else (CURSES_HAVE_NCURSES_H MATCHES "ncurses/ncurses.h")
			get_filename_component (CURSES_INCLUDE_PATH
			    ${CURSES_HAVE_NCURSES_H} PATH)
			set (CURSES_INCLUDE_PATH ${CURSES_INCLUDE_PATH}
			    ${CURSES_INCLUDE_PATH}/ncurses)
		endif (CURSES_HAVE_NCURSES_H MATCHES "ncurses/ncurses.h")
		
	endif (CURSES_HAVE_NCURSES_NCURSES_H)
else (CURSES_HAVE_NCURSES_H OR CURSES_HAVE_NCURSES_NCURSES_H)

	#
	# FindCurses did not find any ncurses.h files, yet it found a
	# libncurses.so shared object. Ergo, there may be a ncurses.h file
	# somewhere that FindCurses missed.
	#
	if (CURSES_HAVE_CURSES_H)
		if (CURSES_HAVE_CURSES_H MATCHES "ncurses/curses.h")
			get_filename_component (CURSES_INCLUDE_PATH
			    ${CURSES_HAVE_CURSES_H} PATH)
		endif (CURSES_HAVE_CURSES_H MATCHES "ncurses/curses.h")
	endif (CURSES_HAVE_CURSES_H)
endif (CURSES_HAVE_NCURSES_H OR CURSES_HAVE_NCURSES_NCURSES_H)

include_directories (${CURSES_INCLUDE_PATH})
message (STATUS "\tInclude dirs for ncurses: ${CURSES_INCLUDE_PATH}")


pkg_check_modules (GLIB2 REQUIRED glib-2.0)
if (GLIB2_FOUND)
	include_directories (${GLIB2_INCLUDE_DIRS})
	link_directories (${GLIB2_LIBRARY_DIRS})
endif (GLIB2_FOUND)

pkg_check_modules (GNT REQUIRED gnt)
if (GNT_FOUND)
	include_directories (${GNT_INCLUDE_DIRS})
	link_directories (${GNT_LIBRARY_DIRS})
endif (GNT_FOUND)

set ( ww_SRCS
	ww_ui.c
	portable_info.c
	ww.c
)
add_executable (ww ${ww_SRCS})

if (${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")

	#
	# ww on NetBSD needs libkvm
	#
	find_library (LIBKVM "kvm")
	target_link_libraries (ww kvm)

	if (NOT LIBKVM)
		message (FATAL_ERROR "libkvm is required for "
		    "${CMAKE_PROJECT_NAME}")
	endif (NOT LIBKVM)
endif (${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

	find_library (LIBPROC "proc")
	target_link_libraries (ww proc)

	if (NOT LIBPROC)
		message (FATAL_ERROR "libproc is required for "
		    "${CMAKE_PROJECT_NAME}")
	endif (NOT LIBPROC)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

target_link_libraries (ww glib-2.0 gobject-2.0 gnt ncurses panel)

# vim:set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab list:
