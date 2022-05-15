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

#include "mmsgui/ft/mmsftcontour.h"
#include <math.h>

static const unsigned int BEZIER_STEPS = 5;

MMSFTContour::MMSFTContour(FT_Vector *outlineVertexList, char *outlineVertexTags, unsigned int outlineNumVertices) {

	MMSFTVertex prev, cur(outlineVertexList[(outlineNumVertices - 1) % outlineNumVertices]), next(outlineVertexList[0]);
    MMSFTVertex a, b = next - cur;
    double olddir, dir = atan2((next - cur).Y(), (next - cur).X());
    double angle = 0.0;

    // See http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-6.html
    // for a full description of FreeType tags.
    for(unsigned int i = 0; i < outlineNumVertices; i++) {
        prev = cur;
        cur = next;
        next = MMSFTVertex(outlineVertexList[(i + 1) % outlineNumVertices]);
        olddir = dir;
        dir = atan2((next - cur).Y(), (next - cur).X());

        // Compute our path's new direction.
        double t = dir - olddir;
        if(t < -M_PI) t += 2 * M_PI;
        if(t > M_PI) t -= 2 * M_PI;
        angle += t;

        // Only process point tags we know.
        if (outlineNumVertices < 2 || FT_CURVE_TAG(outlineVertexTags[i]) == FT_Curve_Tag_On) {
            AddVertex(cur);
        }
        else
        if (FT_CURVE_TAG(outlineVertexTags[i]) == FT_Curve_Tag_Conic) {
            MMSFTVertex prev2 = prev, next2 = next;
//printf("FT_Curve_Tag_Conic\n");
            // Previous point is either the real previous point (an "on"
            // point), or the midpoint between the current one and the
            // previous "conic off" point.
            if (FT_CURVE_TAG(outlineVertexTags[(i - 1 + outlineNumVertices) % outlineNumVertices]) == FT_Curve_Tag_Conic) {
                prev2 = (cur + prev) * 0.5;
                AddVertex(prev2);
            }

            // Next point is either the real next point or the midpoint.
            if(FT_CURVE_TAG(outlineVertexTags[(i + 1) % outlineNumVertices]) == FT_Curve_Tag_Conic) {
                next2 = (cur + next) * 0.5;
            }

            evaluateQuadraticCurve(prev2, cur, next2);
        }
        else
        if (FT_CURVE_TAG(outlineVertexTags[i]) == FT_Curve_Tag_Cubic
          &&FT_CURVE_TAG(outlineVertexTags[(i + 1) % outlineNumVertices]) == FT_Curve_Tag_Cubic) {
//printf("FT_Curve_Tag_Cubic\n");
            evaluateCubicCurve(prev, cur, next, MMSFTVertex(outlineVertexList[(i + 2) % outlineNumVertices]));
        }
    }

    // If final angle is positive (+2PI), it's an anti-clockwise contour,
    // otherwise (-2PI) it's clockwise.
    clockwise = (angle < 0.0);
}


MMSFTContour::~MMSFTContour() {
    this->vertexList.clear();
    this->outsetVertexList.clear();
    this->frontVertexList.clear();
    this->backVertexList.clear();
}


void MMSFTContour::AddVertex(MMSFTVertex vertex) {
    if(vertexList.empty() || (vertex != vertexList[vertexList.size() - 1] && vertex != vertexList[0])) {
        vertexList.push_back(vertex);
    }
}


void MMSFTContour::AddOutsetVertex(MMSFTVertex vertex) {
    outsetVertexList.push_back(vertex);
}


void MMSFTContour::AddFrontVertex(MMSFTVertex vertex) {
    frontVertexList.push_back(vertex);
}


void MMSFTContour::AddBackVertex(MMSFTVertex vertex) {
    backVertexList.push_back(vertex);
}


void MMSFTContour::evaluateQuadraticCurve(MMSFTVertex A, MMSFTVertex B, MMSFTVertex C) {
    for(unsigned int i = 1; i < BEZIER_STEPS; i++)
    {
        double t = static_cast<double>(i) / BEZIER_STEPS;

        MMSFTVertex U = (1.0f - t) * A + t * B;
        MMSFTVertex V = (1.0f - t) * B + t * C;

        AddVertex((1.0f - t) * U + t * V);
    }
}


void MMSFTContour::evaluateCubicCurve(MMSFTVertex A, MMSFTVertex B, MMSFTVertex C, MMSFTVertex D) {
    for(unsigned int i = 0; i < BEZIER_STEPS; i++)
    {
        double t = static_cast<double>(i) / BEZIER_STEPS;

        MMSFTVertex U = (1.0f - t) * A + t * B;
        MMSFTVertex V = (1.0f - t) * B + t * C;
        MMSFTVertex W = (1.0f - t) * C + t * D;

        MMSFTVertex M = (1.0f - t) * U + t * V;
        MMSFTVertex N = (1.0f - t) * V + t * W;

        AddVertex((1.0f - t) * M + t * N);
    }
}


// This function is a bit tricky. Given a path ABC, it returns the
// coordinates of the outset point facing B on the left at a distance
// of 64.0.
//                                         M
//                            - - - - - - X
//                             ^         / '
//                             | 64.0   /   '
//  X---->-----X     ==>    X--v-------X     '
// A          B \          A          B \   .>'
//               \                       \<'  64.0
//                \                       \                  .
//                 \                       \                 .
//                C X                     C X
//
MMSFTVertex MMSFTContour::ComputeOutsetVertex(MMSFTVertex A, MMSFTVertex B, MMSFTVertex C) {
    /* Build the rotation matrix from 'ba' vector */
    MMSFTVertex ba = (A - B).Normalise();
    MMSFTVertex bc = C - B;

    /* Rotate bc to the left */
    MMSFTVertex tmp(bc.X() * -ba.X() + bc.Y() * -ba.Y(),
                bc.X() * ba.Y() + bc.Y() * -ba.X());

    /* Compute the vector bisecting 'abc' */
    double norm = sqrt(tmp.X() * tmp.X() + tmp.Y() * tmp.Y());
    double dist = 64.0 * sqrt((norm - tmp.X()) / (norm + tmp.X()));
    tmp.X(tmp.Y() < 0.0 ? dist : -dist);
    tmp.Y(64.0);

    /* Rotate the new bc to the right */
    return MMSFTVertex(tmp.X() * -ba.X() + tmp.Y() * ba.Y(),
                   tmp.X() * -ba.Y() + tmp.Y() * -ba.X());
}


void MMSFTContour::setParity(int parity) {
    unsigned int size = getVertexCount();
    MMSFTVertex vOutset;

    if(((parity & 1) && clockwise) || (!(parity & 1) && !clockwise)) {
        // Contour orientation is wrong! We must reverse all points.
        // FIXME: could it be worth writing FTVector::reverse() for this?
        for(unsigned int i = 0; i < size / 2; i++) {
            MMSFTVertex tmp = vertexList[i];
            vertexList[i] = vertexList[size - 1 - i];
            vertexList[size - 1 -i] = tmp;
        }

        clockwise = !clockwise;
    }

    for(unsigned int i = 0; i < size; i++) {
        unsigned int prev, cur, next;

        prev = (i + size - 1) % size;
        cur = i;
        next = (i + size + 1) % size;

        vOutset = ComputeOutsetVertex(Vertex(prev), Vertex(cur), Vertex(next));
        AddOutsetVertex(vOutset);
    }
}


void MMSFTContour::buildFrontOutset(double outset) {
	for (unsigned int i = 0; i < getVertexCount(); i++) {
		AddFrontVertex(Vertex(i) + Outset(i) * outset);
	}
}


void MMSFTContour::buildBackOutset(double outset) {
	for (unsigned int i = 0; i < getVertexCount(); i++) {
		AddBackVertex(Vertex(i) + Outset(i) * outset);
	}
}

