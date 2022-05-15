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

#include "mmsgui/ft/mmsftglyph.h"

MMSFTGlyph::MMSFTGlyph() : currentMesh(NULL), err(0) {
    meshList.reserve(16);
}


MMSFTGlyph::~MMSFTGlyph() {
    for(unsigned int m = 0; m < this->meshList.size(); m++) {
        delete this->meshList[m];
    }
    this->meshList.clear();
}


void MMSFTGlyph::tessVertex(const double x, const double y, const double z) {
	this->currentMesh->addVertex(x, y, z);
}


const double* MMSFTGlyph::tessCombine(const double x, const double y, const double z) {
	this->tempVertexList.push_back(MMSFTVertex(x, y,z));
    return static_cast<const double*>(this->tempVertexList.back());
}


void MMSFTGlyph::tessBegin(unsigned int meshType) {
	this->currentMesh = new MMSFTMesh(meshType);
}


void MMSFTGlyph::tessEnd() {
	this->meshList.push_back(this->currentMesh);
}


void MMSFTGlyph::tessError(unsigned int e) {
	this->err = e;
}


unsigned int MMSFTGlyph::getErrorCode() const {
	return this->err;
}


unsigned int MMSFTGlyph::getMeshCount() const {
	return this->meshList.size();
}


const MMSFTMesh* const MMSFTGlyph::getMesh(unsigned int index) const {
    return (index < this->meshList.size()) ? this->meshList[index] : NULL;
}

