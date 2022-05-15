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

#ifndef MMSFTGLYPH_H_
#define MMSFTGLYPH_H_

#include "mmsgui/ft/mmsftmesh.h"

//! container of meshes that describe a polygon glyph
class MMSFTGlyph {
	private:
		// current mesh
		MMSFTMesh *currentMesh;

		// meshes of this glyph
		std::vector<MMSFTMesh*> meshList;

		// extra vertices needed for combine
		MMSFTVertexList tempVertexList;

		// GL ERROR returned by the glu tesselator
		unsigned int err;

    public:
        MMSFTGlyph();
        ~MMSFTGlyph();

        void tessVertex(const double x, const double y, const double z);
        const double* tessCombine(const double x, const double y, const double z);
        void tessBegin(unsigned int meshType);
        void tessEnd();
        void tessError(unsigned int e);

        unsigned int getErrorCode() const;

        unsigned int getMeshCount() const;
        const MMSFTMesh* const getMesh(unsigned int index) const;
};

#endif /* MMSFTGLYPH_H_ */
