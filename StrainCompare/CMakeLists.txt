cmake_minimum_required(VERSION 2.6)

project( StrainCompare )

FIND_PACKAGE(VTK)

IF(VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
ELSE(VTK_FOUND)
  MESSAGE(FATAL_ERROR
          "VTK not found. Please set VTK_DIR.")
ENDIF(VTK_FOUND)

ADD_LIBRARY( CompareSurfaces ../lib/CompareSurfaces/CompareSurfaces.cpp)
ADD_EXECUTABLE( StrainCompare StrainCompare.cpp )

TARGET_LINK_LIBRARIES( StrainCompare CompareSurfaces ${ITK_LIBRARIES} vtkHybrid )

