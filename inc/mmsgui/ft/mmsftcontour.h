/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2011 BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <jens.schneider@diskohq.org>,                   *
 *      Guido Madaus       <guido.madaus@diskohq.org>,                     *
 *      Patrick Helterhoff <patrick.helterhoff@diskohq.org>,               *
 *      René Bählkow       <rene.baehlkow@diskohq.org>                     *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License version 2.1 as published by the Free Software Foundation.     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 **************************************************************************/

#ifndef __MMSFTCONTOUR__
#define __MMSFTCONTOUR__

#include "mmsgui/ft/mmsftvertex.h"

//! container of vertices that describe a vector font outline
class MMSFTContour {
    private:
        MMSFTVertexVector vertexList;
        MMSFTVertexVector outsetVertexList;
        MMSFTVertexVector frontVertexList;
        MMSFTVertexVector backVertexList;

        // clockwise or anti-clockwise?
        bool clockwise;

        inline void AddVertex(MMSFTVertex vertex);
        inline void AddOutsetVertex(MMSFTVertex vertex);
        inline void AddFrontVertex(MMSFTVertex vertex);
        inline void AddBackVertex(MMSFTVertex vertex);
        inline void evaluateQuadraticCurve(MMSFTVertex, MMSFTVertex, MMSFTVertex);
        inline void evaluateCubicCurve(MMSFTVertex, MMSFTVertex, MMSFTVertex, MMSFTVertex);
        inline double NormVector(const MMSFTVertex &v);
        inline void RotationMatrix(const MMSFTVertex &a, const MMSFTVertex &b, double *matRot, double *invRot);
        inline void MultMatrixVect(double *mat, MMSFTVertex &v);
        inline void ComputeBisec(MMSFTVertex &v);
        inline MMSFTVertex ComputeOutsetVertex(MMSFTVertex a, MMSFTVertex b, MMSFTVertex c);

	public:
        MMSFTContour(FT_Vector *outlineVertexList, char *outlineVertexTags, unsigned int outlineNumVertices);
        ~MMSFTContour();

        const MMSFTVertex &Vertex(unsigned int index) const { return vertexList[index]; }

        const MMSFTVertex &Outset(unsigned int index) const { return outsetVertexList[index]; }

        const MMSFTVertex &FrontVertex(unsigned int index) const {
            if(frontVertexList.size() == 0)
                return Vertex(index);
            return frontVertexList[index];
        }

        const MMSFTVertex& BackVertex(unsigned int index) const {
            if(backVertexList.size() == 0)
                return Vertex(index);
             return backVertexList[index];
        }

        unsigned int getVertexCount() const { return vertexList.size(); }

        void setParity(int parity);

        // FIXME: should probably go away
        void buildFrontOutset(double outset);
        void buildBackOutset(double outset);

};

#endif /* __MMSFTCONTOUR__ */
