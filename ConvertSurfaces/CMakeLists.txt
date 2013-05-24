cmake_minimum_required(VERSION 2.6)

project( ConvertSurfaces )

FIND_PACKAGE(VTK)

IF(VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
ELSE(VTK_FOUND)
  MESSAGE(FATAL_ERROR
          "VTK not found. Please set VTK_DIR.")
ENDIF(VTK_FOUND)

ADD_LIBRARY( ReadDaVis ../lib/ReadDaVis/ReadDaVis.cpp )
ADD_EXECUTABLE( ConvertSurfaces ConvertSurfaces.cpp )

TARGET_LINK_LIBRARIES( ConvertSurfaces ReadDaVis ${ITK_LIBRARIES} vtkHybrid )

