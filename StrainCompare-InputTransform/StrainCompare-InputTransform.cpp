/*
 * StrainCompare.cpp
 *
 * Copyright 2013 Seth Gilchrist <seth@fake.com
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


#include <iostream>
#include "../lib/CompareSurfaces-InputTransform/CompareSurfaces-InputTransform.h"
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>


int main(int argc, char **argv)
{
    if (argc < 4 || (argc >4 && argc != 10))
    {
        std::cerr<<"### Execution ERROR ###"<<std::endl;
        std::cerr<<"Not enough inputs. \n Usage:"<<std::endl;
        std::cerr<<argv[0]<<" [Reciever Surface] [Donor Surface] [Output Path] [Optional Initial Transform]"<<std::endl;
        std::cerr<<std::endl;
        std::cerr<<"[Reciever Surface] and [Donor Surface] files must be VTK Polydata files (.vtp). The output will be"<<std::endl;
        std::cerr<<"an VTK unstructured grid file (strainCompare.vtu) and a text file (strainCompare.txt) containing"<<std::endl;
        std::cerr<<"the input file names, any initial transform data and the point data."<<std::endl;
        std::cerr<<"This program assumes that the reciever will be from the drop tower and the donor from the instron."<<std::endl;
        std::cerr<<std::endl;
        std::cerr<<"The reciever surface will the aligned to the donor surface and the data from the donor surface"<<std::endl;
        std::cerr<<"will be transferred to the reciever. The program writes tha two output files files to [Output Path]."<<std::endl;
        std::cerr<<"strainCompare.vtu will have three point data sets:"<<std::endl;
        std::cerr<<"""Drop Tower Strain"", ""Instron Strain"" and ""diff"". diff is calculated as (donor (instron) - reciever (drop tower))"<<std::endl;
        std::cerr<<std::endl;
        std::cerr<<"The optional initial transform is to roughly align the reciever surface with the donor surface before"<<std::endl;
        std::cerr<<"the ICP registration, and is not necessary in all cases. If no transform is given then no rough alignment is made."<<std::endl;
        std::cerr<<"The initial transform must be given as six values the order:"<<std::endl;
        std::cerr<<"[Translate x] [Translate y] [Translate z] [Rotate x] [Rotate y] [Rotate z]"<<std::endl;
        std::cerr<<"Thes values can be obtained by manipulating the surfaces in ParaView."<<std::endl;
        std::cerr<<std::endl<<"### ABORTED ###"<<std::endl;
        return EXIT_FAILURE;
    }
    CompareSurfaces* compare = new CompareSurfaces;
    if ( argc == 10)
    {
        double translate[3] = {atof(argv[4]),atof(argv[5]),atof(argv[6])};
        double rotate[3] = {atof(argv[7]),atof(argv[8]),atof(argv[9])};
        compare->SetInitialTransform(translate,rotate);

    }
	std::cout<<"Reading Drop Tower"<<std::endl;
	// set and read the dt files
	compare->GetRecieverReader()->SetFileName(argv[1]);
	compare->GetRecieverReader()->Update();
	std::cout<<compare->GetRecieverReader()->GetOutput()->GetNumberOfPoints()<<" Points in Drop Tower Surface."<<std::endl;

    // set and read the intron files
    std::cout<<"Reading Instron"<<std::endl;
    compare->GetDonorReader()->SetFileName(argv[2]);
    compare->GetDonorReader()->Update();
	std::cout<<compare->GetDonorReader()->GetOutput()->GetNumberOfPoints()<<" Points in Instron Surface."<<std::endl;

    vtkSmartPointer<vtkPolyData> alignedSurf = compare->AlignSurfaces(compare->GetRecieverReader()->GetOutput(),compare->GetDonorReader()->GetOutput());
//    vtkSmartPointer<vtkXMLPolyDataWriter> polyDebugWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
//    polyDebugWriter->SetInput(alignedSurf);
//    polyDebugWriter->SetFileName("/home/seth/Desktop/aligned.vtp");
//    polyDebugWriter->Write();
    std::cout<<"Surfaces Aligned"<<std::endl;

    /* Note that I tried to use the vector from one centroid to the other, but that yeilded bad results,
    often the reviever surface would be tangent to the volume if the centroids were next to each other.
    For the DIC I can use z-direction of the donor surface and that workds well. Might need another solution
    for the FEA.
    double cent1[3];
    compare->GetSurfaceCentroid(alignedSurf,cent1);
    double cent2[3];
    compare->GetSurfaceCentroid(compare->GetDonorReader()->GetOutput(),cent2);
    double extrudeVector[3] = {cent1[0]-cent2[0],cent1[1]-cent2[1],cent1[2]-cent2[2]};*/
    double extrudeVector[3] = {0,0,1};

    compare->ExtrudeSurface(compare->GetDonorReader()->GetOutput(),extrudeVector);
//    vtkSmartPointer<vtkXMLUnstructuredGridWriter> UgDebugWriter = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
//    UgDebugWriter->SetInput(compare->GetExtrudedVolume());
//    UgDebugWriter->SetFileName("/home/seth/Desktop/volume.vtu");
//    UgDebugWriter->Write();
    std::cout<<"Surface Extruded"<<std::endl;

    vtkSmartPointer<vtkPolyData> probeSurf = compare->ProbeVolume(compare->GetExtrudedVolume(),alignedSurf);
//    polyDebugWriter->SetInput(probeSurf);
//    polyDebugWriter->SetFileName("/home/seth/Desktop/probed.vtp");
//    polyDebugWriter->Write();
    std::cout<<"Volume Probed"<<std::endl;

    std::string donorName = "Instron Strain";
    compare->SetDonorDataName(donorName);
    std::string recieverName = "Drop Tower Strain";
    compare->SetRecieverDataName(recieverName);

    compare->CompileData(alignedSurf,probeSurf);
    std::cout<<"Data Compiled"<<std::endl;

    std::string outPath = argv[3];
    int pathLength = outPath.length();
    if (outPath.compare(pathLength-1,1,"/"))
    {
        outPath.append("/");
    }

    std::string outMeshFile = outPath + "strainCompare.vtu";
    std::string outTextFile = outPath + "strainCompare.txt";

    compare->WriteDataToFile(outTextFile);

    vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
    writer->SetFileName(outMeshFile.c_str());
    writer->SetInput(compare->GetCompiledData());
    writer->Write();
    std::cout<<"Writing Finished"<<std::endl;


	return 0;
}

