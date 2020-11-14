cmake_minimum_required(VERSION 3.6)

project(libs3)
	
set(libs3_src_SOURCES
	src/bucket.c
	src/bucket_metadata.c
	src/error_parser.c
	src/general.c
	src/mingw_functions.c
	src/mingw_s3_functions.c
	src/multipart.c
	src/object.c
	src/request.c
	src/request_context.c
	src/response_headers_handler.c
	src/service.c
	src/service_access_logging.c
	src/simplexml.c
	src/util.c
	mswin/libs3.def)

set(libs3_inc_wingw_sys_HEADERS
	inc/mingw/sys/select.h
	inc/mingw/sys/utsname.h)
	
set(libs3_inc_wingw_HEADERS
	inc/mingw/pthread.h)
	
set(libs3_inc_HEADERS
	inc/error_parser.h
	inc/libs3.h
	inc/request.h
	inc/request_context.h
	inc/response_headers_handler.h
	inc/simplexml.h
	inc/string_buffer.h
	inc/util.h)
	
set(libs3_SOURCES
	
	${libs3_src_SOURCES})

set(libs3_HEADERS
    ${libs3_inc_wingw_sys_HEADERS}
	${libs3_inc_wingw_HEADERS}
	${libs3_inc_HEADERS}
	)
	
source_group("Header Files\\inc\\mingw\\sys" FILES ${libs3_inc_wingw_sys_HEADERS})
source_group("Header Files\\inc\\mingw" FILES ${libs3_inc_wingw_HEADERS})
source_group("Header Files\\inc" FILES ${libs3_inc_HEADERS})
source_group("Source Files\\src" FILES ${libs3_src_SOURCES})


add_library(libs3 SHARED
	${libs3_SOURCES}
	${libs3_HEADERS})
	
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_suffix 64)
else()
	set(_suffix 32)
endif()

SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/win${_suffix})
include_directories(inc;inc/mingw;wininc;win-test)
set(mylibcurl "${libs3_SOURCE_DIR}/winlib/libx${_suffix}/libcurl/libcurl.lib")
set(mylibxml2 "${libs3_SOURCE_DIR}/winlib/libx${_suffix}/libxml2/libxml2.lib")
set(mylibeay32 "${libs3_SOURCE_DIR}/winlib/libx${_suffix}/openssl/libeay32.lib")
set(mylibssleay32 "${libs3_SOURCE_DIR}/winlib/libx${_suffix}/openssl/ssleay32.lib")
target_link_libraries(libs3 "${mylibxml2}" "${mylibcurl}" "${mylibeay32}" "${mylibssleay32}")
target_compile_definitions(libs3 PRIVATE -DLIBS3_VER_MAJOR=\"4\" PRIVATE -DLIBS3_VER_MINOR=\"1\")
target_compile_definitions(libs3 PRIVATE -DLIBS3_VER=${LIBS3_VER_MAJOR}.${LIBS3_VER_MINOR})

add_executable(send_data
    win-test/send-data.c
)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/win${_suffix})

ADD_DEPENDENCIES(send_data libs3)
target_link_libraries(send_data libs3)

add_executable(s3
    src/s3.c
    win-test/getopt.h
    win-test/getopt.c
)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/win${_suffix})
ADD_DEPENDENCIES(s3 libs3)
target_link_libraries(s3 libs3 "${mylibxml2}" "${mylibcurl}" "${mylibeay32}" "${mylibssleay32}")

add_executable(testsimplexml
    src/testsimplexml.c
    src/simplexml.c
)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/win${_suffix})
ADD_DEPENDENCIES(testsimplexml libs3)
target_link_libraries(testsimplexml libs3 "${mylibxml2}" "${mylibeay32}" "${mylibssleay32}")

#if(MSVC_VERSION EQUAL 1700)
#  message(STATUS "1700-msvc_version:" ${MSVC_VERSION})
#elseif(MSVC_VERSION EQUAL 1800)
#  message(STATUS "1800-msvc_version:" ${MSVC_VERSION})
#elseif(MSVC_VERSION EQUAL 1900)
#  message(STATUS "1900-msvc_version:" ${MSVC_VERSION})
#endif()

function(helper_copy_dir target target_configs source dest)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${CMAKE_COMMAND}"
			"-DCONFIG=$<CONFIGURATION>"
			"-DTARGET_CONFIGS=${target_configs}"
			"-DINPUT=${source}"
			"-DOUTPUT=${dest}"
			-P "${CMAKE_SOURCE_DIR}/mswin/cmake/copy_helper.cmake"
		VERBATIM)
endfunction()

function(install_additional maintarget)
    set(addfdir "${CMAKE_SOURCE_DIR}/mswin/additional_install_files")
	
	helper_copy_dir(${maintarget} ALL
		"${addfdir}/exec${_suffix}/"
		"${CMAKE_SOURCE_DIR}/bin/win${_suffix}/$<CONFIGURATION>/")
endfunction()

install_additional(s3)
