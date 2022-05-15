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

#include "mmsgui/fb/mmsfbconv.h"
#include "mmstools/mmstools.h"

void mmsfb_stretchblit_rgb24_to_argb(MMSFBExternalSurfaceBuffer *extbuf, int src_height, int sx, int sy, int sw, int sh,
									 unsigned int *dst, int dst_pitch, int dst_height, int dx, int dy, int dw, int dh,
									 bool antialiasing) {
	// first time?
	static bool firsttime = true;
	if (firsttime) {
		printf("DISKO: Using accelerated stretch RGB24 to ARGB.\n");
		firsttime = false;
	}

	// get the first source ptr/pitch
	unsigned char *src = (unsigned char *)extbuf->ptr;
	int src_pitch = extbuf->pitch;

	// prepare...
	int src_pitch_pix = src_pitch / 3;
	int dst_pitch_pix = dst_pitch >> 2;

	// check the surface range
	if ((sw <= 0)||(sh <= 0))
		return;

	// antialiasing horizontal/vertical/both
	bool h_antialiasing = false;
	bool v_antialiasing = false;
	if (antialiasing) {
		h_antialiasing = true;
		if (sh != dh)
			v_antialiasing = true;
	}

	// stretch buffer
	stretch_324byte_buffer(h_antialiasing, v_antialiasing,
						   src, src_pitch, src_pitch_pix, src_height, sx, sy, sw, sh,
						   dst, dst_pitch, dst_pitch_pix, dst_height, dx, dy, dw, dh);
}
