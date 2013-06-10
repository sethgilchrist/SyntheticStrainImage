/*
 * CompareSurfaces-InputTransform.h
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

#ifndef COMPARESURFACES_INPUTTRANSFORM_H
#define COMPARESURFACES_INPUTTRANSFORM_H

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkIterativeClosestPointTransform.h>
#include <vtkLandmarkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>
#include <vtkCellLocator.h>
#include <vtkWedge.h>
#include <vtkCell.h>
#include <vtkDoubleArray.h>
#include <vtkAppendFilter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkThreshold.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>

#include <vtkXMLPolyDataWriter.h>

class CompareSurfaces
{
    public:
        CompareSurfaces();
        virtual ~CompareSurfaces();

        /** A funciton to get the input file reader **/
        vtkSmartPointer<vtkXMLPolyDataReader> GetRecieverReader()
            {
                return m_recieverReader;
            }

        vtkSmartPointer<vtkXMLPolyDataReader> GetDonorReader()
            {
                return m_donorReader;
            }

        /** A function to extrude the a surface. It extrudes the surface
          * in the direaction defined by a vector from the centroid of
          * one of the input reader's surfaces to the other.**/
        void ExtrudeSurface(vtkSmartPointer<vtkPolyData> surface,double direction[3]);

        /** A function to get the volume created by ExtrudeSurface. **/
        vtkSmartPointer<vtkUnstructuredGrid> GetExtrudedVolume()
            {
                return m_extrudedVolume;
            }

        /** A function to get and return the centroid of a surface, or any
          * poly data for that mater. The passed variable centroid is modified
          * in place. **/
        void GetSurfaceCentroid( vtkSmartPointer<vtkPolyData> surface, double centroid[3]);

        /** A function to probe an extruded volume. Returns the surface
          * with data information from the volume projected onto it. **/
        vtkSmartPointer<vtkPolyData> ProbeVolume(vtkSmartPointer<vtkUnstructuredGrid> volume,
                                                 vtkSmartPointer<vtkPolyData> surface);

        /** A function to align the surfaces, with the points set using
          * SetInitialPoints as the initial transform. Returs recieverSurf
          * transformed to be aligned with donorSurf. **/
        vtkSmartPointer<vtkPolyData> AlignSurfaces(vtkSmartPointer<vtkPolyData> recieverSurf,
                                                   vtkSmartPointer<vtkPolyData> donorSurf);

        /** A function to set the initial transform. A translation vector
         * and rotation vector are provided and used to rigidly transform
         * the reciever surface before the ICP alignment starts. **/
        void SetInitialTransform(double translate[3], double rotate[3])
            {
              if (m_translate[0] != translate[0]) m_translate[0] = translate[0];
              if (m_translate[1] != translate[1]) m_translate[1] = translate[1];
              if (m_translate[2] != translate[2]) m_translate[2] = translate[2];

              if (m_rotate[0] != rotate[0]) m_rotate[0] = rotate[0];
              if (m_rotate[1] != rotate[1]) m_rotate[1] = rotate[1];
              if (m_rotate[2] != rotate[2]) m_rotate[2] = rotate[2];
            }

        /** reciever and donor are identical surfaces. The donor surface has
         * the data transferred from the donor data. Data is compiled onto a
         * single surface and and the names of the datasets are taken from
         * those set in SetRecieverDataName() and SetDonorDataName(). The
         * defalut names are "reciever" and "donor". A thrid dataset is
         * created called "delta" which is the difference between the two and
         * is calculated as (donor - reciever).
         *
         * Finally, the data is thresholded. The ProbeVolume() method
         * uses a value of -1000000 in locations where there was no overlap
         * between the volume ans surface. This step removes cells that
         * have values in the "delta" field of <-999999. **/
        void CompileData( vtkSmartPointer<vtkPolyData> recieverSurf, vtkSmartPointer<vtkPolyData> donorSurf);

        /** A function to return the surface with the compiled data in it.
          * The surface will have the data fields: DTStrain, InstronStrain
          * and StrainDifference. **/
        vtkSmartPointer<vtkUnstructuredGrid> GetCompiledData()
            {
                return m_compiledSurf;
            }

        /** A function to set the reciever data name used when CompileData()
         * is called. The default is "reciever".**/
        void SetRecieverDataName(std::string recieverName)
        {
            if (m_recieverName.compare(recieverName))
            {
                m_recieverName = recieverName;
            }
        }

        /** A function to set the donor data name used when CompileData()
         * is called. The default is "donor". **/
        void SetDonorDataName(std::string donorName)
        {
            if (m_donorName.compare(donorName))
            {
                m_donorName = donorName;
            }
        }

        /** A function to write the strain difference to a file given as
          * an input. The header will contain the format of the file.
          * where the strains are in whatever units were specified in
          * the input files. **/
        void WriteDataToFile(std::string fileName);


    protected:
    private:

    /** Private types **/
    vtkSmartPointer<vtkXMLPolyDataReader>   m_recieverReader;
    vtkSmartPointer<vtkXMLPolyDataReader>   m_donorReader;
    double m_translate[3];
    double m_rotate[3];
    vtkSmartPointer<vtkUnstructuredGrid> m_compiledSurf;
    vtkSmartPointer<vtkUnstructuredGrid> m_extrudedVolume;
    std::string     m_recieverName;
    std::string     m_donorName;

};

#endif // COMPARESURFACES-INPUTTRANSFORM_H
