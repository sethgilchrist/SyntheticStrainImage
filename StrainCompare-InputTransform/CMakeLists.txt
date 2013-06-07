cmake_minimum_required(VERSION 2.6)

project( StrainCompare-InputTransform )

FIND_PACKAGE(VTK)

IF(VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
ELSE(VTK_FOUND)
  MESSAGE(FATAL_ERROR
          "VTK not found. Please set VTK_DIR.")
ENDIF(VTK_FOUND)

ADD_LIBRARY( CompareSurfaces-InputTransform ../lib/CompareSurfaces-InputTransform/CompareSurfaces-InputTransform.cpp)
ADD_EXECUTABLE( StrainCompare-InputTransform StrainCompare-InputTransform.cpp )

TARGET_LINK_LIBRARIES( StrainCompare-InputTransform CompareSurfaces-InputTransform ${ITK_LIBRARIES} vtkHybrid )

