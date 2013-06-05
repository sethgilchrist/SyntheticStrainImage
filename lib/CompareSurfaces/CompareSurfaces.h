/*
 * CompareSurfaces.h
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

#ifndef COMPARESURFACES_H
#define COMPARESURFACES_H

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

        /** A function to set the initial points used in the transform.
          * 6 points are given, three for each surface. s0* are the points
          * for the reciever surface and s1* are the points for the donor
          * surface. s*0, s*1 and s*2 should be pairs. **/
        void SetInitialPoints(double s00[3], double s01[3], double s02[3], double s10[3],double s11[3],double s12[3])
            {
                // s00
                if (m_s00[0] != s00[0])
                {
                    m_s00[0] = s00[0];
                }
                if (m_s00[1] != s00[1])
                {
                    m_s00[1] = s00[1];
                }
                if (m_s00[2] != s00[2])
                {
                    m_s00[2] = s00[2];
                }
                // s01
                if (m_s01[0] != s01[0])
                {
                    m_s01[0] = s01[0];
                }
                if (m_s01[1] != s01[1])
                {
                    m_s01[1] = s01[1];
                }
                if (m_s01[2] != s01[2])
                {
                    m_s01[2] = s01[2];
                }
                // s02
                if (m_s02[0] != s02[0])
                {
                    m_s02[0] = s02[0];
                }
                if (m_s02[1] != s02[1])
                {
                    m_s02[1] = s02[1];
                }
                if (m_s02[2] != s02[2])
                {
                    m_s02[2] = s02[2];
                }

                // s10
                if (m_s10[0] != s10[0])
                {
                    m_s10[0] = s10[0];
                }
                if (m_s10[1] != s10[1])
                {
                    m_s10[1] = s10[1];
                }
                if (m_s10[2] != s10[2])
                {
                    m_s10[2] = s10[2];
                }
                // s11
                if (m_s11[0] != s11[0])
                {
                    m_s11[0] = s11[0];
                }
                if (m_s11[1] != s11[1])
                {
                    m_s11[1] = s11[1];
                }
                if (m_s11[2] != s11[2])
                {
                    m_s11[2] = s11[2];
                }
                // s12
                if (m_s12[0] != s12[0])
                {
                    m_s12[0] = s12[0];
                }
                if (m_s12[1] != s12[1])
                {
                    m_s12[1] = s12[1];
                }
                if (m_s12[2] != s12[2])
                {
                    m_s12[2] = s12[2];
                }


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
    double m_s00[3];
    double m_s01[3];
    double m_s02[3];
    double m_s10[3];
    double m_s11[3];
    double m_s12[3];
    vtkSmartPointer<vtkUnstructuredGrid> m_compiledSurf;
    vtkSmartPointer<vtkUnstructuredGrid> m_extrudedVolume;
    std::string     m_recieverName;
    std::string     m_donorName;

};

#endif // COMPARESURFACES_H
