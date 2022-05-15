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

#include "mmsgui/3d/mms3dobject.h"

#include <math.h>

MMS3DObject::MMS3DObject(MMS3DRegion &region) {
	this->region = region;
	this->finalized = false;
	this->rotate_object = false;
}

void MMS3DObject::setPoint(MMS3DPoint &p) {
	this->org_points.push_back(p);
}

void MMS3DObject::setPoint(double x, double y, double z) {
	this->org_points.push_back(MMS3DPoint(x, y, z));
}

void MMS3DObject::reset() {
	// reset the tmp_points
	this->tmp_points.clear();

	// reset finalize state
	this->finalized = false;
}

void MMS3DObject::rotate_point_x(MMS3DPoint *src, MMS3DPoint *dst) {
	// rotate single point around the x-axis
	double radius, angle;
	if (src->z == 0) {
		// optimized rotation used if z is zero, so radius is equal to y
		radius = src->y;
		angle = this->rotate_xa;
	}
	else {
		// get radius and angle
		radius = sqrt(src->y * src->y + src->z * src->z);
		angle = asin(src->z / radius) * 180 / M_PI;

		// change angle
		if (src->y >= 0)
			angle = angle + this->rotate_xa;
		else
			angle = 180 - angle + this->rotate_xa;
	}

	// calc the dst point
	dst->x = src->x;
	dst->y = radius * cos(angle * M_PI / 180);
	dst->z = radius * sin(angle * M_PI / 180);
}

void MMS3DObject::rotate_point_y(MMS3DPoint *src, MMS3DPoint *dst) {
	// rotation around the y-axis
	double radius, angle;
	if (src->x == 0) {
		// optimized rotation used if x is zero, so radius is equal to z
		radius = src->z;
		angle = this->rotate_ya;
	}
	else {
		// get radius and angle
		radius = sqrt(src->z * src->z + src->x * src->x);
		angle = asin(src->x / radius) * 180 / M_PI;

		// change angle
		if (src->z >= 0)
			angle = angle + this->rotate_ya;
		else
			angle = 180 - angle + this->rotate_ya;
	}

	// calc the dst point
	dst->x = radius * sin(angle * M_PI / 180);
	dst->y = src->y;
	dst->z = radius * cos(angle * M_PI / 180);
}

void MMS3DObject::rotate_point_z(MMS3DPoint *src, MMS3DPoint *dst) {
	// rotation around the z-axis
	double radius, angle;
	if (src->y == 0) {
		// optimized rotation used if y is zero, so radius is equal to x
		radius = src->x;
		angle = this->rotate_za;
	}
	else {
		// get radius and angle
		radius = sqrt(src->x * src->x + src->y * src->y);
		angle = asin(src->y / radius) * 180 / M_PI;

		// change angle
		if (src->x >= 0)
			angle = angle + this->rotate_za;
		else
			angle = 180 - angle + this->rotate_za;
	}

	// calc point and add it to the array
	dst->x = radius * cos(angle * M_PI / 180);
	dst->y = radius * sin(angle * M_PI / 180);
	dst->z = src->z;
}

void MMS3DObject::rotate_x(double angle) {
	// save angle
	this->rotate_xa = angle;

	if (this->tmp_points.size() == 0) {
		// tmp points not set, use org_points as source
		for (int i = 0; i < this->org_points.size(); i++) {
			MMS3DPoint dst;
			rotate_point_x(&this->org_points.at(i), &dst);
			this->tmp_points.push_back(dst);
		}
	}
	else {
		// work with tmp_points as source
		for (int i = 0; i < this->tmp_points.size(); i++) {
			rotate_point_x(&this->tmp_points.at(i), &this->tmp_points.at(i));
		}
	}

	// reset finalize state
	this->finalized = false;
}

void MMS3DObject::rotate_y(double angle) {
	// save angle
	this->rotate_ya = angle;

	if (this->tmp_points.size() == 0) {
		// tmp points not set, use org_points as source
		for (int i = 0; i < this->org_points.size(); i++) {
			MMS3DPoint dst;
			rotate_point_y(&this->org_points.at(i), &dst);
			this->tmp_points.push_back(dst);
		}
	}
	else {
		// work with tmp_points as source
		for (int i = 0; i < this->tmp_points.size(); i++) {
			rotate_point_y(&this->tmp_points.at(i), &this->tmp_points.at(i));
		}
	}

	// reset finalize state
	this->finalized = false;
}

void MMS3DObject::rotate_z(double angle) {
	// save angle
	this->rotate_za = angle;

	if (this->tmp_points.size() == 0) {
		// tmp points not set, use org_points as source
		for (int i = 0; i < this->org_points.size(); i++) {
			MMS3DPoint dst;
			rotate_point_z(&this->org_points.at(i), &dst);
			this->tmp_points.push_back(dst);
		}
	}
	else {
		// work with tmp_points as source
		for (int i = 0; i < this->tmp_points.size(); i++) {
			rotate_point_z(&this->tmp_points.at(i), &this->tmp_points.at(i));
		}
	}

	// reset finalize state
	this->finalized = false;
}

void MMS3DObject::finalize() {
	// check if already finalized?
	if (this->finalized)
		return;

	// clear the final points
	this->fin_points.clear();

	// move object using the region settings
	if (this->tmp_points.size() == 0) {
		// tmp points not set, use org_points as source
		for (int i = 0; i < this->org_points.size(); i++) {
			MMS3DPoint *src = &this->org_points.at(i);
			MMS3DPoint dst;

			dst.x = region.x_center + src->x;
			dst.y = region.y_center + src->y;
			dst.z = region.z_center + src->z;

			this->fin_points.push_back(dst);
		}
	}
	else {
		// work with tmp_points as source
		for (int i = 0; i < this->tmp_points.size(); i++) {
			MMS3DPoint *src = &this->tmp_points.at(i);
			MMS3DPoint dst;

			dst.x = region.x_center + src->x;
			dst.y = region.y_center + src->y;
			dst.z = region.z_center + src->z;

			this->fin_points.push_back(dst);
		}
	}

	this->finalized = true;
}


