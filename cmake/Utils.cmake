# Searches for Qt with the required components
macro(RDM_FIND_QT)

	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC OFF)

	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	if(NOT QT_QMAKE_EXECUTABLE)
	 find_program(QT_QMAKE_EXECUTABLE NAMES "qmake" "qmake-qt5" "qmake.exe")
	endif()
	if(NOT QT_QMAKE_EXECUTABLE)
		message(FATAL_ERROR "you have to set the path to the Qt5 qmake executable")
	endif()
	message(STATUS "QMake found: path: ${QT_QMAKE_EXECUTABLE}")
	GET_FILENAME_COMPONENT(QT_QMAKE_PATH ${QT_QMAKE_EXECUTABLE} PATH)
	set(QT_ROOT ${QT_QMAKE_PATH}/)
	SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5)
	find_package(Qt5 REQUIRED Core Network LinguistTools)
	if (NOT Qt5_FOUND)
		message(FATAL_ERROR "Qt5 not found. Check your QT_QMAKE_EXECUTABLE path and set it to the correct location")
	endif()
	add_definitions(-DQT5)
endmacro(RDM_FIND_QT)

# add OpenCV dependency
macro(RDM_FIND_OPENCV)
	find_package(OpenCV)

	if(NOT OpenCV_FOUND)
		message(FATAL_ERROR "OpenCV not found.")
	else()
		add_definitions(-DWITH_OPENCV)
	endif()

	# unset include directories since OpenCV sets them global
	get_property(the_include_dirs  DIRECTORY . PROPERTY INCLUDE_DIRECTORIES)
	list(REMOVE_ITEM the_include_dirs ${OpenCV_INCLUDE_DIRS})
	set_property(DIRECTORY . PROPERTY INCLUDE_DIRECTORIES ${the_include_dirs})

	# make RelWithDebInfo link against release instead of debug opencv dlls
	set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
	set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE)
endmacro(RDM_FIND_OPENCV)

macro(RDM_PREPARE_PLUGIN)

	CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

	MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)

	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_BINARY_DIR})
	find_package(nomacs)
	if(NOT NOMACS_FOUND)
		set(NOMACS_BUILD_DIRECTORY "NOT_SET" CACHE PATH "Path to the nomacs build directory")
		if (${NOMACS_BUILD_DIRECTORY} STREQUAL "NOT_SET")
			message(FATAL_ERROR "You have to set the nomacs build directory")
		endif()
	endif()

 	if(CMAKE_CL_64)
		SET(PLUGIN_ARCHITECTURE "x64")
	else()
		SET(PLUGIN_ARCHITECTURE "x86")
	endif()

	if (CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
		message(STATUS "A debug build. -DDEBUG is defined")
		add_definitions(-DDEBUG)
		ADD_DEFINITIONS(-DQT_NO_DEBUG)
	elseif (NOT MSVC) # debug and release need qt debug outputs on windows
		message(STATUS "A release build (non-debug). Debugging outputs are silently ignored.")
		add_definitions(-DQT_NO_DEBUG_OUTPUT)
		add_definitions(-DNDEBUG)
	endif ()

	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
	CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
	if(COMPILER_SUPPORTS_CXX11)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	elseif(COMPILER_SUPPORTS_CXX0X)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	elseif(!MSVC)
		message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
	endif()

	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")

endmacro(RDM_PREPARE_PLUGIN)

macro(RDM_FIND_RDF)

	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_BINARY_DIR})
	find_package(ReadFramework)

	if(NOT RDF_FOUND)
		set(RDF_BUILD_DIRECTORY "NOT_SET" CACHE PATH "Path to the READ Framework build directory")
		if(${RDF_BUILD_DIRECTORY} STREQUAL "NOT_SET")
			message(FATAL_ERROR "You have to set the READ Framework build directory")
		endif()
	endif()
endmacro(RDM_FIND_RDF)


macro(RDM_CREATE_TARGETS)

	if(DEFINED GLOBAL_READ_BUILD)
		message(STATUS "project name: ${NOMACS_PROJECT_NAME}")
		add_dependencies(${PROJECT_NAME} ${NOMACS_PROJECT_NAME})
	else()
		# global build automatically puts the dll in the correct directory
		if(MSVC) # copy files on Windows in the correct directory
			set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${NOMACS_BUILD_DIRECTORY}/Debug/plugins/)
			set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${NOMACS_BUILD_DIRECTORY}/Release/plugins/)
			set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${NOMACS_BUILD_DIRECTORY}/RelWithDebInfo/plugins/)
			set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${NOMACS_BUILD_DIRECTORY}/MinSizeRel/plugins/)

			### DependencyCollector
			set(DC_SCRIPT ${CMAKE_SOURCE_DIR}/cmake/DependencyCollector.py)
			set(DC_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/DependencyCollector.ini)

			GET_FILENAME_COMPONENT(VS_PATH ${CMAKE_LINKER} PATH)
			if(CMAKE_CL_64)
				SET(VS_PATH "${VS_PATH}/../../../Common7/IDE/Remote Debugger/x64")
			else()
				SET(VS_PATH "${VS_PATH}/../../Common7/IDE/Remote Debugger/x86")
			endif()
			SET(DC_PATHS_RELEASE ${OpenCV_DIR}/bin/Release ${QT_QMAKE_PATH} ${VS_PATH} ${ReadFramework_DIR}/Release)
			SET(DC_PATHS_DEBUG ${OpenCV_DIR}/bin/Debug ${QT_QMAKE_PATH} ${VS_PATH} ${ReadFramework_DIR}/Debug)

			configure_file(${CMAKE_SOURCE_DIR}/cmake/DependencyCollector.config.cmake.in ${DC_CONFIG})

			add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND python ${DC_SCRIPT} --infile $<TARGET_FILE:${PROJECT_NAME}> --configfile ${DC_CONFIG} --configuration $<CONFIGURATION>)
		else()
			install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION bin)
		endif(MSVC)

	endif()


	if(MSVC)
		file(GLOB RDM_AUTOMOC "${CMAKE_BINARY_DIR}/*_automoc.cpp")
		source_group("Generated Files" FILES ${PLUGIN_RCC} ${RDM_QM} ${RDF_AUTOMOC})

		message(STATUS "${PROJECT_NAME} \t will be installed to: ${NOMACS_INSTALL_DIRECTORY}")

		set(PACKAGE_DIR ${NOMACS_INSTALL_DIRECTORY}/packages/plugins.${PLUGIN_ARCHITECTURE}.${PROJECT_NAME})
		set(PACKAGE_DATA_DIR ${PACKAGE_DIR}/data/nomacs-${PLUGIN_ARCHITECTURE}/plugins/)
		install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${PACKAGE_DATA_DIR} CONFIGURATIONS Release)
		install(FILES ${ADDITIONAL_DLLS} DESTINATION ${PACKAGE_DATA_DIR} CONFIGURATIONS Release)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/package.xml DESTINATION ${PACKAGE_DIR}/meta CONFIGURATIONS Release)
		install(FILES ${ADDITIONAL_OPENCV_PACKAGES_PATHS} DESTINATION ${PACKAGE_DATA_DIR} CONFIGURATIONS Release)
	else()
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${NOMACS_BUILD_DIRECTORY}/plugins/)
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${NOMACS_BUILD_DIRECTORY}/plugins/)
		install(TARGETS ${PROJECT_NAME} RUNTIME LIBRARY DESTINATION lib/nomacs-plugins)
	endif(MSVC)
endmacro(RDM_CREATE_TARGETS)

macro(RDM_READ_PLUGIN_ID_AND_VERSION)
	list(LENGTH PLUGIN_JSON NUM_OF_FILES)
	if(NOT ${NUM_OF_FILES} EQUAL 1)
		message(FATAL_ERROR "${PROJECT_NAME} plugin has zero or more than one .json file")
	endif()
	file(STRINGS ${PLUGIN_JSON} line REGEX ".*\"PluginId\".*:")
	if(line)
		string(REGEX REPLACE ".*:\ +\"" "" PLUGIN_ID ${line})
		string(REGEX REPLACE "\".*" "" PLUGIN_ID ${PLUGIN_ID})
	else()
		message(FATAL_ERROR "${PROJECT_NAME}: PluginId missing in json file")
	endif()
	file(STRINGS ${PLUGIN_JSON} line REGEX ".*\"Version\".*:")
	if(line)
		string(REGEX REPLACE ".*:\ +\"" "" PLUGIN_VERSION ${line})
		string(REGEX REPLACE "\".*" "" PLUGIN_VERSION ${PLUGIN_VERSION})
	else()
		message(FATAL_ERROR "${PROJECT_NAME}: Version missing in json file")
	endif()

	if(ADDITIONAL_OPENCV_PACKAGES_PATHS)
		file(STRINGS ${PLUGIN_JSON} line REGEX ".*\"Dependencies\".*:")
		if(NOT line)
			message(WARNING "${PROJECT_NAME}: JSON file does not contain a Dependencies line but additional opencv libraries are required")
		endif()
	endif()
endmacro(RDM_READ_PLUGIN_ID_AND_VERSION)

macro(RDM_GENERATE_PACKAGE_XML)
	set(JSON_FILE ${ARGN})

	# replace DATE_MODIFIED in json file to last cmake run
	file(STRINGS ${JSON_FILE} date_modified_line REGEX ".*\"DateModified\".*:")
	file(READ ${JSON_FILE} JSON_CONTENT)
	string(TIMESTAMP CURRENT_DATE "%Y-%m-%d")
	string(REPLACE "${date_modified_line}" "\t\"DateModified\"\t: \"${CURRENT_DATE}\"," JSON_CONTENT ${JSON_CONTENT})
	file(WRITE ${JSON_FILE} ${JSON_CONTENT})

	file(STRINGS ${JSON_FILE} line REGEX ".*\"PluginName\".*:")
	string(REGEX REPLACE ".*:\ +\"" "" PLUGIN_NAME ${line})
	string(REGEX REPLACE "\".*" "" PLUGIN_NAME ${PLUGIN_NAME})
	# message(STATUS "PLUGIN_NAME: ${PLUGIN_NAME}")

	file(STRINGS ${JSON_FILE} line REGEX ".*\"AuthorName\".*:")
	string(REGEX REPLACE ".*:\ +\"" "" AUTHOR_NAME ${line})
	string(REGEX REPLACE "\".*" "" AUTHOR_NAME ${AUTHOR_NAME})
	# message(STATUS "AUTHOR_NAME: ${AUTHOR_NAME}")

	file(STRINGS ${JSON_FILE} line REGEX ".*\"Company\".*:")
	string(REGEX REPLACE ".*:\ +\"" "" COMPANY_NAME ${line})
	string(REGEX REPLACE "\".*" "" COMPANY_NAME ${COMPANY_NAME})
	# message(STATUS "COMPANY_NAME: ${COMPANY_NAME}")

	file(STRINGS ${JSON_FILE} line REGEX ".*\"Tagline\".*:")
	string(REGEX REPLACE ".*:\ +\"" "" TAGLINE ${line})
	string(REGEX REPLACE "\".*" "" TAGLINE ${TAGLINE})
	# message(STATUS "TAGLINE: ${TAGLINE}")


	set(XML_CONTENT "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
	set(XML_CONTENT "${XML_CONTENT}<Package>\n")
	set(XML_CONTENT "${XML_CONTENT}\t<DisplayName>${PLUGIN_NAME}</DisplayName>\n")
	set(XML_CONTENT "${XML_CONTENT}\t<Description>${TAGLINE}</Description>\n")
	set(XML_CONTENT "${XML_CONTENT}\t<Version>${PLUGIN_VERSION}</Version>\n")
	set(XML_CONTENT "${XML_CONTENT}\t<ReleaseDate>${CURRENT_DATE}</ReleaseDate>\n")
	set(XML_CONTENT "${XML_CONTENT}\t<Default>true</Default>\n")
	set(XML_CONTENT "${XML_CONTENT}</Package>\n")

	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/package.xml ${XML_CONTENT})

endmacro(RDM_GENERATE_PACKAGE_XML)

macro(RDM_GENERATE_USER_FILE)
	if(MSVC) # create user file only when using Visual Studio
		if(NOT EXISTS "${PROJECT_NAME}.vcxproj.user")
			configure_file(../../cmake/project.vcxproj.user.in ${PROJECT_NAME}.vcxproj.user)
		endif()
	endif(MSVC)
endmacro(RDM_GENERATE_USER_FILE)
