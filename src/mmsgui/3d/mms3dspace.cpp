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

#include "mmsgui/3d/mms3dspace.h"

MMS3DSpace::MMS3DSpace(double width, double height, double depth) {
	this->width = width;
	this->height = height;
	this->depth = depth;
}

void MMS3DSpace::addObject(MMS3DObject *o) {
	this->objects.push_back(o);
}

void MMS3DSpace::render(MMSFBSurface *surface, int distance) {
	unsigned int *ptr;
	int pitch;
	surface->setColor(0x80, 0, 0, 0xff);
	surface->fillRectangle(0, 0, this->width, this->height);
	surface->lock(MMSFB_LOCK_WRITE, (void**)&ptr, &pitch);


	if (distance < this->width / 2)
		distance = this->width / 2;
	if (distance < this->height / 2)
		distance = this->height / 2;

	double d = distance;

	for (int oc = 0; oc < this->objects.size(); oc++) {
		MMS3DObject *o = this->objects.at(oc);
		o->finalize();
		for (int pc = 0; pc < o->fin_points.size(); pc++) {
			MMS3DPoint *p = &o->fin_points.at(pc);

			double f = d + p->z;
			double x = (p->x * d) / f;
			double y = -(p->y * d) / f;

			int xx = x + this->width / 2;
			int yy = y + this->height / 2;

			if   ((xx >= 0) && (yy >= 0)
			   && (xx < this->width) && (yy < this->height)) {

				ptr[(int)xx+(int)yy*pitch/4] = 0xffffffff;
			}

		}
	}

	surface->unlock();
}

void MMS3DSpace::render(vector<MMS3DPoint> *points, int distance) {

	if (distance < this->width / 2)
		distance = this->width / 2;
	if (distance < this->height / 2)
		distance = this->height / 2;

	double d = distance;

	points->clear();

	for (int oc = 0; oc < this->objects.size(); oc++) {
		MMS3DObject *o = this->objects.at(oc);
		o->finalize();
		for (int pc = 0; pc < o->fin_points.size(); pc++) {
			MMS3DPoint *p = &o->fin_points.at(pc);

			double f = d + p->z;
			double x = (p->x * d) / f;
			double y = -(p->y * d) / f;

			int xx = x + this->width / 2;
			int yy = y + this->height / 2;

			if   ((xx >= 0) && (yy >= 0)
			   && (xx < this->width) && (yy < this->height)) {

				MMS3DPoint fp(xx, yy, p->z);

				points->push_back(fp);
			}

		}
	}
}

