/*
 * StrainCompare.cpp
 *
 * Copyright 2013 Seth Gilchrist <seth@laptop01>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#include "../lib/CompareSurfaces/CompareSurfaces.h"
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>


int main(int argc, char **argv)
{
    if (argc < 4 || (argc >4 && argc != 22))
    {
        std::cerr<<"Not enough inputs. \n Usage:"<<std::endl;
        std::cerr<<argv[0]<<" [DT Surface] [Instron Surface] [Output File Name] [Optional Points]"<<std::endl;
        std::cerr<<"The files are ASCII data files exported from StrainMaster and the output file will be a"<<std::endl;
        std::cerr<<"VTK Points file and must have the extionsion .vtp"<<std::endl;
        std::cerr<<"The optional points must have 18 values and given in the order:"<<std::endl;
        std::cerr<<"[surf 1, pt 1 x] [surf 1, pt 1 y] [surf 1, pt1 z] [surf1, pt2 x]...[surf2, pt3 z]"<<std::endl;
        std::cerr<<"Aborted"<<std::endl;
        return EXIT_FAILURE;
    }
    CompareSurfaces* compare = new CompareSurfaces;
    if ( argc == 22)
    {
        double s00[3] = {atof(argv[4]),atof(argv[5]),atof(argv[6])};
        double s01[3] = {atof(argv[7]),atof(argv[8]),atof(argv[9])};
        double s02[3] = {atof(argv[10]),atof(argv[11]),atof(argv[12])};
        double s10[3] = {atof(argv[13]),atof(argv[14]),atof(argv[15])};
        double s11[3] = {atof(argv[16]),atof(argv[17]),atof(argv[18])};
        double s12[3] = {atof(argv[19]),atof(argv[20]),atof(argv[21])};
        compare->SetInitialPoints(s00, s01, s02, s10, s11, s12);

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

    vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
    writer->SetFileName(argv[3]);
    writer->SetInput(compare->GetCompiledData());
    writer->Write();
    std::cout<<"Writing Finished"<<std::endl;


	return 0;
}

