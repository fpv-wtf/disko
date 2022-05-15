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

#ifndef MMSFBDEVOMAP_H_
#define MMSFBDEVOMAP_H_

#ifdef __HAVE_FBDEV__

#include "mmsgui/fb/mmsfbdev.h"

class MMSFBDevOmap : public MMSFBDev {
    private:
    	typedef struct {
        	MMSFBDevOmap   			*fbdev;
        	char					device[100];
        	int						width;
        	int						height;
        	MMSFBSurfacePixelFormat	pixelformat;
        	int						backbuffer;
    	} MMSFBDevOmapLayer;

    	MMSFBDevOmapLayer	osd0;
    	MMSFBDevOmapLayer	vid;
    	MMSFBDevOmapLayer	osd1;
    	MMSFBDevOmapLayer	*primary;

    public:
        MMSFBDevOmap();
        virtual ~MMSFBDevOmap();

        bool openDevice(char *device_file = NULL, int console = -1);
        void closeDevice();

        bool waitForVSync();
        bool panDisplay(int buffer_id, void *framebuffer_base);

        bool testLayer(int layer_id);
        bool initLayer(int layer_id, int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer = 0);

        bool releaseLayer(int layer_id);
        bool restoreLayer(int layer_id);

        bool setMode(int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer = 0);

    private:

        bool vtGetFd(int *fd);
};

#endif

#endif /* MMSFBDEVOMAP_H_ */
