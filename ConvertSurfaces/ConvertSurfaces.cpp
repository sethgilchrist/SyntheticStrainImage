/*
 * ConvertSurfaces.cpp
 *
 * Copyright 2013 Seth Gilchrist <seth@fake.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include "../lib/ReadDaVis/ReadDaVis.h"
#include <vtkIterativeClosestPointTransform.h>
#include <vtkLandmarkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkXMLPolyDataWriter.h>

int main(int argc, char **argv)
{
    if (argc < 6)
    {
        std::cerr<<"Not enough inputs.\nUsage:"<<std::endl;
        std::cerr<<argv[0]<<" [DropTower Surfrace] [DropTower Strain] [Instron Surface] [Instron Strain] [Output Folder]"<<std::endl<<"Aborted."<<std::endl;
        return EXIT_FAILURE;
    }
    if (argc > 6)
    {
        std::cout<<"Too many inputs give. Extra inputs ignored.\nUsage:"<<std::endl;
        std::cout<<argv[0]<<" [DropTower Surfrace] [DropTower Strain] [Instron Surface] [Instron Strain] [Output Folder]"<<std::endl;
    }

    ReadDaVis *dtReader = new ReadDaVis;
    dtReader->SetHeightFileName(argv[1]);
    dtReader->SetStrainFileName(argv[2]);

    ReadDaVis *inReader = new ReadDaVis;
    inReader->SetHeightFileName(argv[3]);
    inReader->SetStrainFileName(argv[4]);

    std::cout<<"Reading drop tower file..."<<std::endl;
    dtReader->ReadHeightFile();
    dtReader->ReadStrainFile();
    dtReader->CreateDataSurface();
    std::cout<<"Drop tower file successfully read. Number of points in droptower surface: "<<dtReader->GetSurface()->GetNumberOfPoints()<<std::endl;

    std::cout<<"Reading instron file..."<<std::endl;
    inReader->ReadHeightFile();
    inReader->ReadStrainFile();
    inReader->CreateDataSurface();
    std::cout<<"Instron file successfully read. Number of points in instron surface: "<<inReader->GetSurface()->GetNumberOfPoints()<<std::endl;

    std::string outPath = argv[5];
    int pathLength = outPath.length();
    if (outPath.compare(pathLength-1,1,"/"))
    {
        outPath.append("/");
    }

    std::string dtOutFile = outPath;
    dtOutFile.append("dropTowerSurface.vtp");
    std::string inOutFile = outPath;
    inOutFile.append("instronSurface.vtp");

    std::cout<<"Writing the drop tower surface to "<<dtOutFile<<std::endl;
    vtkSmartPointer<vtkXMLPolyDataWriter> dtWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    dtWriter->SetInput(dtReader->GetSurface());
    dtWriter->SetFileName(dtOutFile.c_str());
    dtWriter->Write();
    std::cout<<"Drop tower file successfully written."<<std::endl;

    std::cout<<"Writing the instron surface to "<<inOutFile<<std::endl;
    vtkSmartPointer<vtkXMLPolyDataWriter> polyWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    polyWriter->SetInput(inReader->GetSurface());
    polyWriter->SetFileName(inOutFile.c_str());
    polyWriter->Write();
    std::cout<<"Instron file successfully written."<<std::endl;
}
