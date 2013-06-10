/*
 * CompareSurfaces-InputTransform.cpp
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

#include "CompareSurfaces-InputTransform.h"

CompareSurfaces::CompareSurfaces()
{
    // create a new readers
    m_recieverReader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
    m_donorReader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
    /* set the points for the initial transform. The default
     * values don't transform anything. */
    m_translate[0] = 0; m_translate[1] = 0; m_translate[2] = 0;
    m_rotate[0] = 0; m_rotate[1] = 0; m_rotate[2] = 0;
    // create the output surface
    //m_compiledSurf = vtkSmartPointer<vtk>::New();
    // create the extruded volume made from the donor
    m_extrudedVolume = vtkSmartPointer<vtkUnstructuredGrid>::New();
    // set the default data names
    m_recieverName = "reciever";
    m_donorName = "donor";
}

CompareSurfaces::~CompareSurfaces()
{
    //destructor. Nothing to do here.
}

void CompareSurfaces::ExtrudeSurface(vtkSmartPointer<vtkPolyData> surf,double vect[3])
{
    // make the vector 5 mm long
    double length = sqrt(pow(vect[0],2)+pow(vect[1],2)+pow(vect[2],2));
    double scale = 5/length;
    vect[0] = vect[0]*scale;
    vect[1] = vect[1]*scale;
    vect[2] = vect[2]*scale;
    // transform the surface to be extruded in the -vect
    vtkSmartPointer<vtkTransform> transformer = vtkSmartPointer<vtkTransform>::New();
    transformer->Translate(-vect[0],-vect[1],-vect[2]);
    vtkSmartPointer<vtkTransformPolyDataFilter> polyMover = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    polyMover->SetTransform(transformer);
    polyMover->SetInput(surf);

    // use the append filter to convert the PolyData to UnstructuredGrid
    vtkSmartPointer<vtkAppendFilter> appender = vtkSmartPointer<vtkAppendFilter>::New();
    appender->SetInputConnection(polyMover->GetOutputPort());
    appender->Update();
    vtkSmartPointer<vtkUnstructuredGrid> tempGrid = appender->GetOutput();

    // iterate through the points
    unsigned int originalNumberOfPoints = tempGrid->GetNumberOfPoints();
    for (unsigned int i = 0; i < originalNumberOfPoints;++i)
    {
        double cPoint[3];
        tempGrid->GetPoint(i,cPoint);
        // for each point, create a child point in the point data at a location 2*vect away
        tempGrid->GetPoints()->InsertNextPoint(cPoint[0]+2*vect[0],cPoint[1]+2*vect[1],cPoint[2]+2*vect[2]);
        // get the data for the parent point and copy it into the child point
        tempGrid->GetPointData()->GetArray(0)->InsertNextTuple(tempGrid->GetPointData()->GetArray(0)->GetTuple(i));
    }

    // iterate through the cells (trangles) of the original data
    unsigned int originalNumberOfCells = tempGrid->GetNumberOfCells();
    for (unsigned int i= 0; i < originalNumberOfCells;++i)
    {
        // for each cell, get a list of the defining points
        vtkSmartPointer<vtkIdList> cPoints = vtkSmartPointer<vtkIdList>::New();
        tempGrid->GetCellPoints(i,cPoints);
        // create a list of points for the new, wedge cell. The points are the original list
        // plus the child poit of each one of those. They are sepearted by the originial number of points
        vtkSmartPointer<vtkIdList> newPoints = vtkSmartPointer<vtkIdList>::New();
        newPoints->InsertNextId(cPoints->GetId(0));
        newPoints->InsertNextId(cPoints->GetId(1));
        newPoints->InsertNextId(cPoints->GetId(2));
        newPoints->InsertNextId(cPoints->GetId(0)+originalNumberOfPoints);
        newPoints->InsertNextId(cPoints->GetId(1)+originalNumberOfPoints);
        newPoints->InsertNextId(cPoints->GetId(2)+originalNumberOfPoints);
        // insert a new wedge cell, defined by the points
        tempGrid->InsertNextCell(13,newPoints);

    }

    // put the data into the classes extruded volume.
    m_extrudedVolume = tempGrid;
}

void CompareSurfaces::GetSurfaceCentroid(vtkSmartPointer<vtkPolyData> surface,double centroid[3])
{
    double ptx = 0;
    double pty = 0;
    double ptz = 0;
    for (vtkIdType i = 0; i < surface->GetPoints()->GetNumberOfPoints(); ++i)
    {
        double pt[3];
        surface->GetPoints()->GetPoint(i,pt);
        ptx += pt[0];
        pty += pt[1];
        ptz += pt[2];
    }
    centroid[0] = ptx/surface->GetPoints()->GetNumberOfPoints();
    centroid[1] = pty/surface->GetPoints()->GetNumberOfPoints();
    centroid[2] = ptz/surface->GetPoints()->GetNumberOfPoints();
}

vtkSmartPointer<vtkPolyData> CompareSurfaces::ProbeVolume(vtkSmartPointer<vtkUnstructuredGrid> volume, vtkSmartPointer<vtkPolyData> surface)
{
    vtkSmartPointer<vtkPolyData> outputSurface = vtkSmartPointer<vtkPolyData>::New();
    outputSurface->DeepCopy(surface);
    unsigned int numberOfArrays = outputSurface->GetPointData()->GetNumberOfArrays();
    for (unsigned int i = 0; i < numberOfArrays; ++i)
    {
        outputSurface->GetPointData()->RemoveArray(i);
    }

    // create a new array to hold the data
    vtkSmartPointer<vtkDoubleArray> newArray = vtkSmartPointer<vtkDoubleArray>::New();
    newArray->SetNumberOfComponents(1);
    newArray->SetNumberOfTuples(outputSurface->GetNumberOfPoints());
    newArray->SetName("Extracted Data");
    outputSurface->GetPointData()->AddArray(newArray);

    // create a cell locator to aid in finding the cells that points belong to
    vtkSmartPointer<vtkCellLocator> cellLocator =
    vtkSmartPointer<vtkCellLocator>::New();
    cellLocator->SetDataSet(volume);
    cellLocator->BuildLocator();

    // these will be used in the loop to hold data
    vtkSmartPointer<vtkWedge> cCell;
    double blankTuple = -1000000.;
    double cPoint[3];
    double closestPoint[3];
    int subId;
    double pcoords[3];
    double dist2;
    double weights[6];
    double cellData[6];
    double cValue;
    vtkIdType cCellNo;
    vtkSmartPointer<vtkIdList> cellPoints;

    // loop through each point in the input surface
    for (unsigned int i = 0; i < outputSurface->GetNumberOfPoints(); i++)
    {
        // get the point location
        outputSurface->GetPoint(i,cPoint);
        // find the cell that contains the point
        cCellNo = cellLocator->FindCell(cPoint);
        if (cCellNo == -1)  // if the point is outside of cells, enter a value of zero in the data array
        {
            newArray->SetTuple(i,&blankTuple);
            continue;
        }
        if (volume->GetCellType(cCellNo) != 13) // if the cell isn't a wedge, skip it.
        {
            continue;
        }
        // get the point IDs that define the containing cell
        cellPoints = volume->GetCell(cCellNo)->GetPointIds();
        // get the strain values for each point in the containing cell
        cellData[0] = *volume->GetPointData()->GetArray(0)->GetTuple(cellPoints->GetId(0));
        cellData[1] = *volume->GetPointData()->GetArray(0)->GetTuple(cellPoints->GetId(1));
        cellData[2] = *volume->GetPointData()->GetArray(0)->GetTuple(cellPoints->GetId(2));
        cellData[3] = *volume->GetPointData()->GetArray(0)->GetTuple(cellPoints->GetId(3));
        cellData[4] = *volume->GetPointData()->GetArray(0)->GetTuple(cellPoints->GetId(4));
        cellData[5] = *volume->GetPointData()->GetArray(0)->GetTuple(cellPoints->GetId(5));

        // use the EvaluatePosition method of the cell to get the interpolation function weight values. The rest of the data is not used
        volume->GetCell(cCellNo)->EvaluatePosition(cPoint,closestPoint,subId,pcoords,dist2,weights);
        // calculate the value at the point by multiplying each weight by the data
        cValue = cellData[0]*weights[0] + cellData[1]*weights[1] + cellData[2]*weights[2] + cellData[3]*weights[3] + cellData[4]*weights[4] + cellData[5]*weights[5];
        // set the data.
        newArray->SetTuple(i,&cValue);

    }
    return outputSurface;
}

vtkSmartPointer<vtkPolyData> CompareSurfaces::AlignSurfaces(vtkSmartPointer<vtkPolyData> recieverSurf, vtkSmartPointer<vtkPolyData> donorSurf)
{
    // create a general transform for the initial transform
    vtkSmartPointer<vtkTransform> initialTranRot = vtkSmartPointer<vtkTransform>::New();
    initialTranRot->Translate(m_translate);
    initialTranRot->RotateX(m_rotate[0]);
    initialTranRot->RotateY(m_rotate[1]);
    initialTranRot->RotateZ(m_rotate[2]);

    // transform recieverSurf using the rough transform
    vtkSmartPointer<vtkTransformPolyDataFilter> initialTransform = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    initialTransform->SetInput(recieverSurf);
    initialTransform->SetTransform(initialTranRot);
    initialTransform->Update();

    vtkSmartPointer<vtkXMLPolyDataWriter> tempWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    tempWriter->SetInput(initialTransform->GetOutput());
    tempWriter->SetFileName("/home/seth/Desktop/test.vtp");
    tempWriter->Write();

    // use the output of the of the rough transform as the input to the fine icp calculation
    vtkSmartPointer<vtkIterativeClosestPointTransform> icp = vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
    icp->SetSource(initialTransform->GetOutput());
    icp->SetTarget(donorSurf);
    icp->GetLandmarkTransform()->SetModeToRigidBody();
    icp->Modified();
    icp->Update();

    // use the output of icp as the final transform.
    vtkSmartPointer<vtkTransformPolyDataFilter> finalTransform = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    finalTransform->SetInput(initialTransform->GetOutput());
    finalTransform->SetTransform(icp);
    finalTransform->Update();

    // return the output from the final transfrom
    return finalTransform->GetOutput();
}

void CompareSurfaces::CompileData( vtkSmartPointer<vtkPolyData> recieverSurf, vtkSmartPointer<vtkPolyData> donorSurf)
{
    // copy the structure of the reciever surface as this is the of the compiled surface
    vtkSmartPointer<vtkPolyData> tempSurface = vtkSmartPointer<vtkPolyData>::New();
    tempSurface->CopyStructure(recieverSurf);

    // create a new data array for the drop tower strain
    vtkSmartPointer<vtkDoubleArray> recieverData = vtkSmartPointer<vtkDoubleArray>::New();
    recieverData->DeepCopy(recieverSurf->GetPointData()->GetArray(0));
    recieverData->SetName(m_recieverName.c_str());

    // create a new data array for the instron strain
    vtkSmartPointer<vtkDoubleArray> donorData = vtkSmartPointer<vtkDoubleArray>::New();
    donorData->DeepCopy(donorSurf->GetPointData()->GetArray(0));
    donorData->SetName(m_donorName.c_str());

    // create a new data array for the difference between them
    vtkSmartPointer<vtkDoubleArray> diff = vtkSmartPointer<vtkDoubleArray>::New();
    diff->SetNumberOfTuples(tempSurface->GetNumberOfPoints());
    diff->SetNumberOfComponents(1);
    diff->SetName("delta");

    // iterate through the points in the compliled surface and fill in the data arrays.
    for (unsigned int i = 0; i<tempSurface->GetNumberOfPoints();++i)
    {
        double* cDonor = donorData->GetTuple(i);
        double* cReciever = recieverData->GetTuple(i);
        double cDiff;
        // in the ProbvVolume method, -1000000 was used to indicate a point with no data. Carry that though.
        if (*cReciever == -1000000)
        {
            cDiff = -1000000;
        }
        else
        {
            cDiff = *cDonor-*cReciever;
        }
        diff->SetTuple(i,&cDiff);
    }

    // add the arrays to the point data
    tempSurface->GetPointData()->AddArray(recieverData);
    tempSurface->GetPointData()->AddArray(donorData);
    tempSurface->GetPointData()->AddArray(diff);

    // threshold the temporary surface to remove data where there was no overlap
    vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
    threshold->SetInput(tempSurface);
    threshold->SetInputArrayToProcess(0,0,0,
                                      vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                      "delta");
    threshold->ThresholdByUpper(-999990);
    threshold->Update();

    // return the compiled surface
    m_compiledSurf = threshold->GetOutput();
}

void CompareSurfaces::WriteDataToFile(std::string fileName)
{
    // open the file for writing
    std::ofstream outFile;
    outFile.open(fileName.c_str(), std::ios::trunc);
    if (!outFile.is_open()) // if it failes to open, exit
    {
        std::cerr<<"Error opening output file: "<<fileName<<"\nPlease check the name and try again."<<std::endl;
        return;
    }
    // write the header line
    outFile << "Reviever (Moving) File Name: "<<m_recieverReader->GetFileName()<<std::endl;
    outFile << "Donor (Fixed) File Name:" <<m_donorReader->GetFileName()<<std::endl;
    outFile << "Initial Transform. Translate ("<<m_translate[0]<<","<<m_translate[1]<<","<<m_translate[2]<<"). Rotate ("<<
        m_rotate[0]<<","<<m_rotate[1]<<","<<m_rotate[2]<<")"<<std::endl;
    outFile << "Point,"<<m_compiledSurf->GetPointData()->GetArray(0)->GetName()<<","<<
        m_compiledSurf->GetPointData()->GetArray(1)->GetName()<<",Diff,x,y,z"<<std::endl;

    // write the rest of the file
    double aStrain;
    double bStrain;
    double diff;
    double loc[3];
    for (int i = 0; i < m_compiledSurf->GetNumberOfPoints(); ++i)
    {
        m_compiledSurf->GetPointData()->GetArray(0)->GetTuple(i,&aStrain);
        m_compiledSurf->GetPointData()->GetArray(1)->GetTuple(i,&bStrain);
        m_compiledSurf->GetPointData()->GetArray(2)->GetTuple(i,&diff);
        m_compiledSurf->GetPoint(i,loc);

        outFile << i <<","<<aStrain<<","<<bStrain<<","<<diff<<","<<loc[0]<<","<<loc[1]<<","<<loc[2]<<std::endl;
    }

    outFile.close();


}
