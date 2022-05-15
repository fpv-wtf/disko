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

#include "mmsgui/3d/mms3dcircle.h"

#include <math.h>

MMS3DCircle::MMS3DCircle(MMS3DRegion &region) : MMS3DObject(region) {
}

void MMS3DCircle::point(int x, int y, int z, int octant) {

	MMS3DPoint p = MMS3DPoint(x, y, z);
	this->org_points.push_back(p);
}

void MMS3DCircle::draw(int x, int y, int z, int radius, int start_octant, int end_octant) {

	// check input
	if (end_octant < start_octant)
		end_octant = start_octant;

	// draw first point
	if ((start_octant<=4)&&(end_octant>=3))
		point(x, y + radius, z, 4);
	if ((start_octant==0)||(end_octant==7))
		point(x, y - radius, z, 0);
	if ((start_octant<=2)&&(end_octant>=1))
		point(x + radius, y, z, 2);
	if ((start_octant<=6)&&(end_octant>=5))
		point(x - radius, y, z, 6);

	// init
	int mr = radius * radius;
	int mx = 1;
	int my = (int) (sqrt(mr - 1) + 0.5);

	while (mx < my) {
		if ((start_octant<=0)&&(end_octant>=0))
			point(x + mx, y - my, z, 0);
		if ((start_octant<=1)&&(end_octant>=1))
			point(x + my, y - mx, z, 1);
		if ((start_octant<=2)&&(end_octant>=2))
			point(x + my, y + mx, z, 2);
		if ((start_octant<=3)&&(end_octant>=3))
			point(x + mx, y + my, z, 3);
		if ((start_octant<=4)&&(end_octant>=4))
			point(x - mx, y + my, z, 4);
		if ((start_octant<=5)&&(end_octant>=5))
			point(x - my, y + mx, z, 5);
		if ((start_octant<=6)&&(end_octant>=6))
			point(x - my, y - mx, z, 6);
		if ((start_octant<=7)&&(end_octant>=7))
			point(x - mx, y - my, z, 7);

		mx++;
		my = (int) (sqrt(mr - mx*mx) + 0.5);
	}

	if (mx == my) {
		if ((start_octant<=3)&&(end_octant>=2))
			point(x + mx, y + my, z, 3);
		if ((start_octant<=1)&&(end_octant>=0))
			point(x + mx, y - my, z, 1);
		if ((start_octant<=5)&&(end_octant>=4))
			point(x - mx, y + my, z, 5);
		if ((start_octant<=7)&&(end_octant>=6))
			point(x - mx, y - my, z, 7);
	}

}

void MMS3DCircle::draw(int x, int y, int z, int radius, bool sort) {
	if (!sort) {
		draw(x, y, z, radius);
	}
	else {
		// sort around beginning from right middle (quarter)
		unsigned int s, e, j;

		draw(x, y, z, radius, 1, 1);

		s = this->org_points.size();
		draw(x, y, z, radius, 0, 0);
		e = this->org_points.size();
		for (unsigned int i = s, j = e - 1; i < s + ((e - s) / 2); i++, j--) {
			MMS3DPoint p = this->org_points.at(i);
			this->org_points.at(i) = this->org_points.at(j);
			this->org_points.at(j) = p;
		}

		draw(x, y, z, radius, 7, 7);

		s = this->org_points.size();
		draw(x, y, z, radius, 6, 6);
		e = this->org_points.size();
		for (unsigned int i = s, j = e - 1; i < s + ((e - s) / 2); i++, j--) {
			MMS3DPoint p = this->org_points.at(i);
			this->org_points.at(i) = this->org_points.at(j);
			this->org_points.at(j) = p;
		}

		draw(x, y, z, radius, 5, 5);

		s = this->org_points.size();
		draw(x, y, z, radius, 4, 4);
		e = this->org_points.size();
		for (unsigned int i = s, j = e - 1; i < s + ((e - s) / 2); i++, j--) {
			MMS3DPoint p = this->org_points.at(i);
			this->org_points.at(i) = this->org_points.at(j);
			this->org_points.at(j) = p;
		}

		draw(x, y, z, radius, 3, 3);

		s = this->org_points.size();
		draw(x, y, z, radius, 2, 2);
		e = this->org_points.size();
		for (unsigned int i = s, j = e - 1; i < s + ((e - s) / 2); i++, j--) {
			MMS3DPoint p = this->org_points.at(i);
			this->org_points.at(i) = this->org_points.at(j);
			this->org_points.at(j) = p;
		}
	}
}

