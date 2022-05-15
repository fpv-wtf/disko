/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009      BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <pupeider@gmx.de>,                              *
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

#ifndef MMS3DOBJECT_H_
#define MMS3DOBJECT_H_

#include "mmstools/mmstypes.h"

//! Describes a 3D object.
/*!
\author Jens Schneider
*/
class MMS3DObject {
	private:
		MMS3DRegion region;

		vector<MMS3DPoint>	org_points;
		vector<MMS3DPoint>	tmp_points;
		vector<MMS3DPoint>	fin_points;

		bool finalized;

		bool rotate_object;
		double rotate_xa;
		double rotate_ya;
		double rotate_za;

	public:
		MMS3DObject(MMS3DRegion &region);

		void setPoint(MMS3DPoint &p);

		void setPoint(double x, double y, double z);

		void reset();

		void rotate_point_x(MMS3DPoint *src, MMS3DPoint *dst);
		void rotate_point_y(MMS3DPoint *src, MMS3DPoint *dst);
		void rotate_point_z(MMS3DPoint *src, MMS3DPoint *dst);

		void rotate_x(double angle = 0);
		void rotate_y(double angle = 0);
		void rotate_z(double angle = 0);

		void finalize();


	friend class MMS3DLine;
	friend class MMS3DCircle;
	friend class MMS3DSpace;
};

#endif /*MMS3DOBJECT_H_*/
