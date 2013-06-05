//      ReadInputFiles.h
//
//      Copyright 2011 Seth Gilchrist <seth@fake.com>
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#ifndef READDAVIS_H
#define READDAVIS_H


#include <cstring>
#include <sstream>
#include <iterator>
#include <boost/tokenizer.hpp>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkDoubleArray.h>
#include <vtkDelaunay2D.h>


class ReadDaVis
{
public:

    /** Constructor **/
    ReadDaVis();

    /** Set/Get the height file name.**/
    void SetHeightFileName( std::string fileName );
    std::string GetHeightFileName();
    /** Set the strain file name. **/
    void SetStrainFileName( std::string fileName );

    /** Read the height file **/
    void ReadHeightFile();
    /** Read the strain file **/
    void ReadStrainFile();
    /** Read a file given in fileName and put the results in the pointset **/
    void ReadFile(std::string fileName, vtkSmartPointer<vtkImageData> pointData);
    /** Put the height data into a surface and put the z-comp of the strain
      * point data as a dataset at the points of the hight data. **/
    void CreateDataSurface();

    /** Get the height point data. **/
    vtkSmartPointer<vtkImageData> GetHeightData();
    /** Get the strain point data. **/
    vtkSmartPointer<vtkImageData> GetStrainData();
    /** Get the surface. **/
    vtkSmartPointer<vtkPolyData> GetSurface();
    //vtkSmartPointer<vtkUnstructuredGrid> GetSurface();


private:
    std::string                                 m_heightFileName;
    std::string                                 m_strainFileName;
    vtkSmartPointer<vtkImageData>               m_heightData;
    vtkSmartPointer<vtkImageData>               m_strainData;
    vtkSmartPointer<vtkPolyData>                m_surface;
    //vtkSmartPointer<vtkUnstructuredGrid>        m_surface;

};
#endif
