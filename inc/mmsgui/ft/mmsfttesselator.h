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

#ifndef __MMSFTTESSELATOR__
#define __MMSFTTESSELATOR__

#include "mmsgui/ft/mmsftcontour.h"
#include "mmsgui/ft/mmsftglyph.h"

//! helper class that converts font outlines into vertices using GLUtesselator
class MMSFTTesselator {
	private:
		//! vectorized glyph
		MMSFTGlyph* glyph;

		//! freetype outline
		FT_Outline outline;

		//! list of contours in the glyph
		MMSFTContour **contourList;

		//! number of contours reported by freetype
		unsigned int contourCount;

		//! flag indicating the tesselation rule for the glyph
		int contourFlag;


		//! convert freetype outline data into contours
		void processContours();

	public:
        MMSFTTesselator(const FT_GlyphSlot glyph);
        virtual ~MMSFTTesselator();

        bool generateGlyph(double zNormal = 1.0f, int outsetType = 0, float outsetSize = 0.0f);
        const MMSFTGlyph* const getGlyph() const;

        unsigned int getContourCount();
        MMSFTContour *getContour(unsigned int index);
};

#endif  /* __MMSFTTESSELATOR__ */
