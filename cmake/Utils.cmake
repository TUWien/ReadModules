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

# search for opencv
unset(OpenCV_LIB_DIR_DBG CACHE)
unset(OpenCV_3RDPARTY_LIB_DIR_DBG CACHE)
unset(OpenCV_3RDPARTY_LIB_DIR_OPT CACHE)
unset(OpenCV_CONFIG_PATH CACHE)
unset(OpenCV_LIB_DIR_DBG CACHE)
unset(OpenCV_LIB_DIR_OPT CACHE)
unset(OpenCV_LIBRARY_DIRS CACHE)
unset(OpenCV_DIR)

find_package(OpenCV REQUIRED core imgproc)

if(NOT OpenCV_FOUND)
	message(FATAL_ERROR "OpenCV not found.") 
else()
	add_definitions(-DWITH_OPENCV)
endif()

# unset include directories since OpenCV sets them global
get_property(the_include_dirs  DIRECTORY . PROPERTY INCLUDE_DIRECTORIES)
list(REMOVE_ITEM the_include_dirs ${OpenCV_INCLUDE_DIRS})
set_property(DIRECTORY . PROPERTY INCLUDE_DIRECTORIES ${the_include_dirs})

endmacro(RDM_FIND_OPENCV)

macro(RDM_PREPARE_PLUGIN)
  
  CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

  MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)

    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_BINARY_DIR})		
	find_package(nomacs)
    if(NOT NOMACS_FOUND)
      SET(NOMACS_BUILD_DIRECTORY "NOT_SET" CACHE PATH "Path to the nomacs build directory")
      IF (${NOMACS_BUILD_DIRECTORY} STREQUAL "NOT_SET")
        MESSAGE(FATAL_ERROR "You have to set the nomacs build directory")
      ENDIF()
    endif()
    SET(NOMACS_PLUGIN_INSTALL_DIRECTORY ${CMAKE_SOURCE_DIR}/install CACHE PATH "Path to the plugin install directory for deploying")
	
  if(GLOBAL_READ_BUILD) # this is a complete build of the READ framework, thus we need to set compiler/linker settings for ReallyRelease
	set(CMAKE_CXX_FLAGS_REALLYRELEASE "${CMAKE_CXX_FLAGS_RELEASE}  /DQT_NO_DEBUG_OUTPUT") 
	set(CMAKE_EXE_LINKER_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE") # /subsystem:windows does not work due to a bug in cmake (see http://public.kitware.com/Bug/view.php?id=12566)
	SET(CMAKE_SHARED_LINKER_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE") # /subsystem:windows does not work due to a bug in cmake (see http://public.kitware.com/Bug/view.php?id=12566)
  endif(GLOBAL_READ_BUILD)
    
  if (CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
      message(STATUS "A debug build. -DDEBUG is defined")
      add_definitions(-DDEBUG)
      ADD_DEFINITIONS(-DQT_NO_DEBUG)
  elseif (NOT MSVC) # debug and release need qt debug outputs on windows
      message(STATUS "A release build (non-debug). Debugging outputs are silently ignored.")
      add_definitions(-DQT_NO_DEBUG_OUTPUT)
  endif ()
  
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()
  
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
  
endmacro(RDM_PREPARE_PLUGIN)

macro(RDM_FIND_RDF)

	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_BINARY_DIR})
	find_package(ReadFramework)
		
	if(NOT RDF_FOUND)
	  SET(RDF_BUILD_DIRECTORY "NOT_SET" CACHE PATH "Path to the READ Framework build directory")
	  IF (${RDF_BUILD_DIRECTORY} STREQUAL "NOT_SET")
		MESSAGE(FATAL_ERROR "You have to set the READ Framework build directory")
	  ENDIF()
	endif()
	
      
endmacro(RDM_FIND_RDF)

# you can use this NMC_CREATE_TARGETS("myAdditionalDll1.dll" "myAdditionalDll2.dll")
macro(RDM_CREATE_TARGETS)
	
  set(ADDITIONAL_DLLS ${ARGN})
  
  list(LENGTH ADDITIONAL_DLLS NUM_ADDITONAL_DLLS) 
  if( ${NUM_ADDITONAL_DLLS} GREATER 0) 
    foreach(DLL ${ADDITIONAL_DLLS})
      message(STATUS "extra_macro_args: ${DLL}")
    endforeach()
  endif()
  
  
IF (MSVC)
	file(GLOB RDM_AUTOMOC "${CMAKE_BINARY_DIR}/*_automoc.cpp")
	source_group("Generated Files" FILES ${PLUGIN_RCC} ${RDM_QM} ${RDF_AUTOMOC})
	
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/plugins/)
  if(${NUM_ADDITONAL_DLLS} GREATER 0) 
    foreach(DLL ${ADDITIONAL_DLLS})
      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DLL} ${NOMACS_BUILD_DIRECTORY}/$<CONFIGURATION>/)
    endforeach()
  endif()
  
	# write dll to d.txt (used for automated plugin download)
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/d.txt)
		file(READ ${CMAKE_CURRENT_SOURCE_DIR}/d.txt fileContent)	
		if(CMAKE_CL_64)
			string (REGEX MATCHALL "x64" matches ${fileContent})
			if(NOT matches)
				file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x64 x64/${PROJECT_NAME}.dll\n")
        if(${NUM_ADDITONAL_DLLS} GREATER 0) 
          foreach(DLL ${ADDITIONAL_DLLS})
            file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x64 x64/${DLL}\n")        
          endforeach()
        endif()
			endif()
		else()
			string (REGEX MATCHALL "x86" matches ${fileContent})		
			if(NOT matches)
				file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x86 x86/${PROJECT_NAME}.dll\n")
        if(${NUM_ADDITONAL_DLLS} GREATER 0) 
          foreach(DLL ${ADDITIONAL_DLLS})
            file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x86 x86/${DLL}\n")        
          endforeach()
        endif()
			endif()	
		endif(CMAKE_CL_64)
	else()
		if(CMAKE_CL_64)
      file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x64 x64/${PROJECT_NAME}.dll\n")
      if(${NUM_ADDITONAL_DLLS} GREATER 0) 
        foreach(DLL ${ADDITIONAL_DLLS})
          file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x64 x64/${DLL}\n")        
        endforeach()
      endif()

		else()
			file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x86 x86/${PROJECT_NAME}.dll\n")
      if(${NUM_ADDITONAL_DLLS} GREATER 0) 
        foreach(DLL ${ADDITIONAL_DLLS})
          file(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/d.txt "x86 x86/${DLL}\n")        
        endforeach()
      endif()      
		endif()	
	endif()
	install(FILES ${${CMAKE_BINARY_DIR}}/Release/${PROJECT_NAME}.dll DESTINATION ${NOMACS_PLUGIN_INSTALL_DIRECTORY}/${PLUGIN_ID}/${PLUGIN_VERSION}/${PLUGIN_ARCHITECTURE}/ CONFIGURATIONS Release)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/d.txt DESTINATION ${NOMACS_PLUGIN_INSTALL_DIRECTORY}/${PLUGIN_ID}/${PLUGIN_VERSION}/ CONFIGURATIONS Release)
  # if(${NUM_ADDITONAL_DLLS} GREATER 0) 
    # foreach(DLL ${ADDITIONAL_DLLS})
		# check this command from nomacs-plugins (was altered)
      # install(${DLL} DESTINATION ${NOMACS_PLUGIN_INSTALL_DIRECTORY}/${PLUGIN_ID}/${PLUGIN_VERSION}/${PLUGIN_ARCHITECTURE}/ CONFIGURATIONS Release)
    # endforeach()
  # endif()      
    
elseif(UNIX)
  if(${NUM_ADDITONAL_DLLS} GREATER 0) 
    foreach(DLL ${ADDITIONAL_DLLS})
      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DLL} ${NOMACS_BUILD_DIRECTORY}/)
    endforeach()
  endif()
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${NOMACS_BUILD_DIRECTORY})
  install(TARGETS ${PROJECT_NAME} RUNTIME LIBRARY DESTINATION lib/nomacs-plugins)
endif(MSVC)
endmacro(RDM_CREATE_TARGETS)
