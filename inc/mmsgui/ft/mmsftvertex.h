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

#ifndef MMSFTVERTEX_H_
#define MMSFTVERTEX_H_

#include <list>
#include <vector>

#include <ft2build.h>
#include FT_GLYPH_H

//! describes a vertex including operators
class MMSFTVertex {
	private:
		double values[3];

	public:
        inline MMSFTVertex() {
        	values[0] = values[1] = values[2] = 0;
        }

        inline MMSFTVertex(const double x, const double y, const double z = 0) {
            values[0] = x;
            values[1] = y;
            values[2] = z;
        }

        inline MMSFTVertex(const FT_Vector &ft_vector) {
            values[0] = ft_vector.x;
            values[1] = ft_vector.y;
            values[2] = 0;
        }

        MMSFTVertex Normalise();

        inline MMSFTVertex& operator += (const MMSFTVertex &point) {
            values[0] += point.values[0];
            values[1] += point.values[1];
            values[2] += point.values[2];
            return *this;
        }

        inline MMSFTVertex operator + (const MMSFTVertex &point) const {
            MMSFTVertex temp;
            temp.values[0] = values[0] + point.values[0];
            temp.values[1] = values[1] + point.values[1];
            temp.values[2] = values[2] + point.values[2];
            return temp;
        }

        inline MMSFTVertex& operator -= (const MMSFTVertex &point) {
            values[0] -= point.values[0];
            values[1] -= point.values[1];
            values[2] -= point.values[2];
            return *this;
        }

        inline MMSFTVertex operator - (const MMSFTVertex &point) const {
            MMSFTVertex temp;
            temp.values[0] = values[0] - point.values[0];
            temp.values[1] = values[1] - point.values[1];
            temp.values[2] = values[2] - point.values[2];
            return temp;
        }

        inline MMSFTVertex operator * (double multiplier) const {
            MMSFTVertex temp;
            temp.values[0] = values[0] * multiplier;
            temp.values[1] = values[1] * multiplier;
            temp.values[2] = values[2] * multiplier;
            return temp;
        }

        inline friend MMSFTVertex operator * (double multiplier, MMSFTVertex &point) {
            return point * multiplier;
        }

        inline friend double operator * (MMSFTVertex &a, MMSFTVertex &b) {
            return a.values[0] * b.values[0]
                 + a.values[1] * b.values[1]
                 + a.values[2] * b.values[2];
        }

        inline MMSFTVertex operator ^ (const MMSFTVertex &point) {
            MMSFTVertex temp;
            temp.values[0] = values[1] * point.values[2]
                              - values[2] * point.values[1];
            temp.values[1] = values[2] * point.values[0]
                              - values[0] * point.values[2];
            temp.values[2] = values[0] * point.values[1]
                              - values[1] * point.values[0];
            return temp;
        }


        inline operator const double*() const {
            return values;
        }

        inline void X(double x) { values[0] = x; };
        inline void Y(double y) { values[1] = y; };
        inline void Z(double z) { values[2] = z; };

        inline double X() const { return values[0]; };
        inline double Y() const { return values[1]; };
        inline double Z() const { return values[2]; };
};

typedef std::list<MMSFTVertex> MMSFTVertexList;
typedef std::vector<MMSFTVertex> MMSFTVertexVector;

#endif /* MMSFTVERTEX_H_ */
