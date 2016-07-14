PROJECT(read-plugins)

cmake_minimum_required(VERSION 3.0)

# include macros needed
include("./cmake/Utils.cmake")

IF(EXISTS ${CMAKE_SOURCE_DIR}/CMakeUser.cmake)
	include(${CMAKE_SOURCE_DIR}/CMakeUser.cmake)
ENDIF()

OPTION (ENABLE_PAGE_VIS "Compile PAGE Visualization Plugin" ON)
# OPTION (ENABLE_PAGE_EXTRACTION "Compile Page Extration Plugin" OFF)
OPTION (ENABLE_BINARIZATION "Compile Page Extration Plugin" ON)
OPTION (ENABLE_LAYOUT "Compile Layout Analysis Plugin" OFF)
OPTION (ENABLE_SKEW "Skew Estimation Plugin" ON)
OPTION (ENABLE_FOCUSMEASURE "Focus Measure Plugin" ON)
OPTION (ENABLE_WRITERIDENTIFICATION "Compile Writer Identification plugin" OFF)
OPTION (ENABLE_BATCH_TEST "Test plugin for new batch interface" OFF)

RDM_PREPARE_PLUGIN()

RDM_FIND_RDF()

RDM_FIND_QT()

# OpenCV
RDM_FIND_OPENCV()

if(CMAKE_CL_64)
	SET(PLUGIN_ARCHITECTURE "x64")
else()
	SET(PLUGIN_ARCHITECTURE "x86")
endif()


set(BUILDING_MULTIPLE_PLUGINS true)

IF (ENABLE_BINARIZATION)
	add_subdirectory(Modules/Binarization)
ENDIF()

IF (ENABLE_PAGE_EXTRACTION)
	add_subdirectory(Modules/PageExtraction)
ENDIF()

IF (ENABLE_PAGE_VIS)
	add_subdirectory(Modules/PageVis)
ENDIF()

IF (ENABLE_LAYOUT)
	add_subdirectory(Modules/Layout)
ENDIF()

IF (ENABLE_SKEW)
	add_subdirectory(Modules/Skew)
ENDIF()

IF (ENABLE_FOCUSMEASURE)
	add_subdirectory(Modules/FocusMeasure)
ENDIF()

IF (ENABLE_WRITERIDENTIFICATION)
	add_subdirectory(Modules/WriterIdentification)
ENDIF()

IF (ENABLE_BATCH_TEST)
	add_subdirectory(Modules/BatchTest)
ENDIF()


