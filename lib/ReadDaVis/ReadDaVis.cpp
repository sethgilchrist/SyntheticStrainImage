//      ReadDaVis.cpp
//
//      Copyright 2011 Seth Gilchrist <seth@mech.ubc.ca>
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
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

#include "ReadDaVis.h"

/** Constructor **/
ReadDaVis::ReadDaVis()
{
//    m_heightFileName; // Must be provided by user
//    m_strainFileName; // Must be provided by user
m_heightData  = vtkSmartPointer<vtkImageData>::New();
m_strainData  = vtkSmartPointer<vtkImageData>::New();
m_surface     = vtkSmartPointer<vtkPolyData>::New();
//m_surface       = vtkSmartPointer<vtkUnstructuredGrid>::New();
}

void ReadDaVis::SetHeightFileName( std::string fileName )
{
    if (m_heightFileName.compare(fileName) != 0) {m_heightFileName = fileName;}
}
std::string ReadDaVis::GetHeightFileName()
{
    return m_heightFileName;
}

void ReadDaVis::SetStrainFileName( std::string fileName )
{
    if (m_strainFileName.compare(fileName) != 0) {m_strainFileName = fileName;}
}

void ReadDaVis::ReadHeightFile()
{
    ReadFile(m_heightFileName,m_heightData);
}

void ReadDaVis::ReadStrainFile()
{
    ReadFile(m_strainFileName,m_strainData);
}

void ReadDaVis::ReadFile(std::string fileName, vtkSmartPointer<vtkImageData> pointData)
{
    // create a stringstream for passing info to the user
    std::stringstream msg(" ");
    // open the file
    std::ifstream inFile(fileName.c_str());
    if (!inFile){
        msg.str(" ");
        msg << "Cannot open\n" <<fileName<<"\nPlease check the name and try again."<<std::endl;
        std::cout << msg;
    }

    // Get the header line
    std::string headerLine;
    std::getline(inFile,headerLine);
    std::vector<std::string> headerTokens;
    boost::char_separator<char> sep(" ");
    boost::tokenizer< boost::char_separator<char> > tok(headerLine,sep); // the boost library tokenizer
    for (boost::tokenizer< boost::char_separator<char> >::iterator beg=tok.begin(); beg!=tok.end();++beg){
        headerTokens.push_back(*beg);
    }

    int xDimension = atoi(headerTokens[3].c_str());
    float xScale = atof(headerTokens[6].c_str());
    float xOffset = atof(headerTokens[7].c_str());
    int yDimension = atoi(headerTokens[4].c_str());
    float yScale = atof(headerTokens[10].c_str());
    float yOffset = atof(headerTokens[1].c_str());
    pointData->SetDimensions(xDimension,yDimension,1);
    pointData->SetOrigin(xOffset,yOffset,0);
    pointData->SetSpacing(xScale,yScale,1);

    // now step through the file and create the points
    std::string cline;
    unsigned int i = 0; // the x point number, will be used with x-scale and x-offest to produce an point

    while( std::getline (inFile,cline))
    {
        // break the current line into values of height
        std::vector<double> values;
        boost::tokenizer< boost::char_separator<char> > tok(cline,sep);
        for (boost::tokenizer< boost::char_separator<char> >::iterator beg=tok.begin(); beg!=tok.end();++beg)
        {
            std::string current = *beg;
            values.push_back(atof(current.c_str()));
        }
        for (int j = 0; j < yDimension; ++j)
        {
            pointData->SetScalarComponentFromDouble(i,j,0,0,values[j]);
        }
        ++i;

    }
    inFile.close();

}

void ReadDaVis::CreateDataSurface()
{
    // find out which point set has fewer points
    int* heightDimensions = m_heightData->GetDimensions();
    int* strainDimensions = m_strainData->GetDimensions();
    int xEnd = (heightDimensions[0] > strainDimensions[0]) ? strainDimensions[0] : heightDimensions[0];
    int yEnd = (heightDimensions[1] > strainDimensions[1]) ? strainDimensions[1] : heightDimensions[1];

    vtkSmartPointer<vtkPoints>  surfacePoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkDoubleArray> surfaceArray = vtkSmartPointer<vtkDoubleArray>::New();
    surfaceArray->SetNumberOfComponents(1);
    surfaceArray->SetName("MinPStrain");

    // iterate through x and y, if the data for both != 0, then save the point
    for (int i = 0; i<xEnd; ++i)
    {
        for (int j = 0; j< yEnd; ++j)
        {
            double heightPoint = m_heightData->GetScalarComponentAsDouble(i,j,0,0);
            double strainPoint = m_strainData->GetScalarComponentAsDouble(i,j,0,0);
            if (heightPoint!=0 && strainPoint!=0)
            {
                int pointIndex[3];
                pointIndex[0] = i;
                pointIndex[1] = j;
                pointIndex[2] = 0;
                double* pointLocation = m_heightData->GetPoint(m_heightData->ComputePointId(pointIndex));
                surfacePoints->InsertNextPoint(pointLocation[0],pointLocation[1],heightPoint);
                surfaceArray->InsertNextTuple1(strainPoint);

            }

        }
    }

    m_surface->SetPoints(surfacePoints);
    m_surface->GetPointData()->AddArray(surfaceArray);

    // use Delaunay2D to create a mesh, ignoring the z-dimension
//    vtkSmartPointer<vtkUnstructuredGrid> tempSurf = vtkSmartPointer<vtkUnstructuredGrid>::New();
//    tempSurf->DeepCopy(m_surface);
    vtkSmartPointer<vtkDelaunay2D> delauney = vtkSmartPointer<vtkDelaunay2D>::New();
    delauney->SetInput(m_surface);
    delauney->Update();
    m_surface = delauney->GetOutput();

}

vtkSmartPointer<vtkImageData> ReadDaVis::GetHeightData()
{
    return m_heightData;
}

vtkSmartPointer<vtkImageData> ReadDaVis::GetStrainData()
{
    return m_strainData;
}

vtkSmartPointer<vtkPolyData> ReadDaVis::GetSurface()
//vtkSmartPointer<vtkUnstructuredGrid> ReadDaVis::GetSurface()
{
    return m_surface;
}
