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

#include "mmsgui/mmsfbmanager.h"
#include "mmsgui/fb/mmsfbsurfacemanager.h"
#include <string.h>
#include <stdlib.h>

MMS_CREATEERROR(MMSFBManagerError);

/* initialize the mmsfbmanager object */
MMSFBManager mmsfbmanager;

/* exit handler routine */
void mmsfbmanager_onexit(int num, void *arg) {
    MMSFBManager *o=(MMSFBManager*)arg;
    o->release();
}

MMSFBManager::MMSFBManager() {
    /* init me */
    this->graphicslayer = NULL;
    this->videolayer = NULL;
    this->videolayerid = -1;
    this->graphicslayerid = -1;
    this->layercount = 0;
}

MMSFBManager::~MMSFBManager() {
}

bool MMSFBManager::init(int argc, char **argv, string appl_name, string appl_icon_name) {
	int myargc=argc;
	char *myargv[255];
	int i;

    this->layercount=1;

	for(i=0;i<argc;i++)
		myargv[i]=strdup(argv[i]);

#ifdef  __HAVE_DIRECTFB__
	if(config.getOutputType() == MMSFB_OT_X11) {
		myargv[myargc]=strdup("--dfb:system=x11");
		myargc++;
	}
#endif

    DEBUGMSG("MMSGUI", "init mmsfb");
    bool ea = config.getExtendedAccel();
#ifdef  __HAVE_DIRECTFB__
	if (config.getAllocMethod() == "DFB") {
		// use dfb even if extended accel
		ea = false;
	}
#endif


	// get layer settings from config
	MMSConfigDataLayer videolayer = this->config.getVideoLayer();
	MMSConfigDataLayer graphicslayer = this->config.getGraphicsLayer();

	// init the MMSFB class
    if (!mmsfb->init(myargc, myargv, config.getBackend(), config.getOutputType(), graphicslayer.rect,
					 ea, config.getFullScreen(), config.getPointer(), appl_name, appl_icon_name, config.getHideApplication())) {
	    DEBUGMSG("MMSGUI", "init mmsfb failed!");
        throw new MMSFBManagerError(0, MMSFB_LastErrorString);
	}

    DEBUGMSG("MMSGUI", "get video layer");
    if (!mmsfb->getLayer(videolayer.id, &this->videolayer))
        throw new MMSFBManagerError(0, MMSFB_LastErrorString);

    if(videolayer.id == graphicslayer.id) {
    	DEBUGMSG("MMSGUI", "video layer and graphics layer are the same");
        this->graphicslayer = this->videolayer;
    }
    else {
        this->layercount++;
        DEBUGMSG("MMSGUI", "get graphics layer");
        if (!mmsfb->getLayer(graphicslayer.id, &this->graphicslayer))
            throw new MMSFBManagerError(0, MMSFB_LastErrorString);

    	this->graphicslayer->setFlipFlags(MMSFB_FLIP_ONSYNC);
    }

	if (config.getOutputType() == MMSFB_OT_MATROXFB)
    	this->videolayer->setFlipFlags(MMSFB_FLIP_WAITFORSYNC);
    else
    	this->videolayer->setFlipFlags(MMSFB_FLIP_ONSYNC);

    if (!this->graphicslayer->getID(&this->graphicslayerid))
        throw new MMSFBManagerError(0, MMSFB_LastErrorString);

    if (!this->videolayer->getID(&this->videolayerid))
        throw new MMSFBManagerError(0, MMSFB_LastErrorString);

    /* set on exit handler */
    on_exit(mmsfbmanager_onexit, this);

    return true;
}

void MMSFBManager::release() {
	DEBUGMSG("MMSGUI", "release mmsfb");
    if (this->videolayer)
        delete this->videolayer;
    mmsfb->release();
}

void MMSFBManager::applySettings() {
	DEBUGMSG("MMSGUI", "configure graphics layer");

	// get layer settings from config
	MMSConfigDataLayer videolayer = this->config.getVideoLayer();
	MMSConfigDataLayer graphicslayer = this->config.getGraphicsLayer();

	// get the window pixelformat
	MMSFBSurfacePixelFormat window_pixelformat = config.getGraphicsWindowPixelformat();
	switch (window_pixelformat) {
	case MMSFB_PF_ARGB:
	case MMSFB_PF_AiRGB:
	case MMSFB_PF_AYUV:
	case MMSFB_PF_ARGB4444:
	case MMSFB_PF_RGB16:
		break;
	default:
		// window pixelformat not set or unsupported, use the layer pixelformat
		window_pixelformat = graphicslayer.pixelformat;
		if (!isAlphaPixelFormat(window_pixelformat)) {
			// the gui internally needs surfaces with alpha channel
			// now we have to decide if we are working in RGB or YUV color space
			if (!isRGBPixelFormat(window_pixelformat))
				// so switch all non-alpha pixelformats to AYUV
				window_pixelformat = MMSFB_PF_AYUV;
			else
				// so switch all non-alpha pixelformats to ARGB
				window_pixelformat = MMSFB_PF_ARGB;
		}
		else
		if (isIndexedPixelFormat(window_pixelformat))
			// the gui internally needs non-indexed surfaces
			// so switch all indexed pixelformats to ARGB
			window_pixelformat = MMSFB_PF_ARGB;
	}

	// get the surface pixelformat
	MMSFBSurfacePixelFormat surface_pixelformat = config.getGraphicsSurfacePixelformat();
	switch (surface_pixelformat) {
	case MMSFB_PF_ARGB:
	case MMSFB_PF_AiRGB:
	case MMSFB_PF_AYUV:
	case MMSFB_PF_ARGB4444:
	case MMSFB_PF_RGB16:
		break;
	default:
		// surface pixelformat not set or unsupported, use the layer pixelformat
		surface_pixelformat = graphicslayer.pixelformat;
		if (!isAlphaPixelFormat(surface_pixelformat)) {
			// the gui internally needs surfaces with alpha channel
			// now we have to decide if we are working in RGB or YUV color space
			if (!isRGBPixelFormat(surface_pixelformat))
				// so switch all non-alpha pixelformats to AYUV
				surface_pixelformat = MMSFB_PF_AYUV;
			else
				// so switch all non-alpha pixelformats to ARGB
				surface_pixelformat = MMSFB_PF_ARGB;
		}
		else
		if (isIndexedPixelFormat(surface_pixelformat))
			// the gui internally needs non-indexed surfaces
			// so switch all indexed pixelformats to ARGB
			surface_pixelformat = MMSFB_PF_ARGB;
	}

	// set exclusive access to the graphics layer
	DEBUGMSG("MMSGUI", "set exclusive access");
	if (!this->graphicslayer->setExclusiveAccess())
        throw new MMSFBManagerError(0, MMSFB_LastErrorString);

	DEBUGMSG("MMSGUI", "set configuration");
    if (!this->graphicslayer->setConfiguration(graphicslayer.rect.w, graphicslayer.rect.h,
											   graphicslayer.pixelformat,
											   graphicslayer.buffermode,
											   graphicslayer.options,
                                               window_pixelformat,
                                               surface_pixelformat))
        throw new MMSFBManagerError(0, MMSFB_LastErrorString);

    if (this->videolayerid != this->graphicslayerid) {
        if(config.getOutputType() == MMSFB_OT_X11)
        //give a little time to window routines
        usleep(300000);

        /* use both layers */
        DEBUGMSG("MMSGUI", "configure video layer");

        DEBUGMSG("MMSGUI", "set exclusive access");
        /* set exclusive access to the video layer */
        if (!this->videolayer->setExclusiveAccess())
            throw new MMSFBManagerError(0, MMSFB_LastErrorString);

    	DEBUGMSG("MMSGUI", "set configuration");
        /* set video layer's config */
        if (!this->videolayer->setConfiguration(videolayer.rect.w, videolayer.rect.h,
												videolayer.pixelformat,
												videolayer.buffermode,
												videolayer.options))
            throw new MMSFBManagerError(0, MMSFB_LastErrorString);
		//this->videolayer->dfblayer->SetFieldParity(this->videolayer->dfblayer,0);

        /* set the full opacity of the graphics layer */
        this->graphicslayer->setOpacity(0);

        if (config.getOutputType() == MMSFB_OT_VIAFB) {
            /* set the video layer behind the graphics layer */
        	DEBUGMSG("MMSGUI", "set the video layer behind the graphics layer");
            this->videolayer->setLevel(-1);
        }
    }

    // set global surface attributes
    string buffermode = graphicslayer.buffermode;
    MMSFBSurface *gls;
    if (this->graphicslayer->getSurface(&gls)) {
    	// set the static extended accel flag
		gls->setExtendedAcceleration(config.getExtendedAccel());

		// set the global alloc method (default is malloc)
		if (mmsfb->getBackend() == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
			string am = config.getAllocMethod();
			if (am == "MALLOC") {
				if (!config.getExtendedAccel())
					gls->setAllocMethod(MMSFBSurfaceAllocMethod_dfb);
			}
			else
				gls->setAllocMethod(MMSFBSurfaceAllocMethod_dfb);
#endif
		}
    }

    // init the mmsfbwindowmanager
	mmsfbwindowmanager->init(this->graphicslayer, (config.getPointer()==MMSFB_PM_TRUE));

    DEBUGMSG("MMSGUI", "creating temporary surface: %dx%d, %s", graphicslayer.rect.w, graphicslayer.rect.h, getMMSFBPixelFormatString(surface_pixelformat).c_str());
    mmsfbsurfacemanager->createTemporarySurface(graphicslayer.rect.w, graphicslayer.rect.h, surface_pixelformat, (buffermode == MMSFB_BM_BACKSYSTEM));
}


MMSFBLayer *MMSFBManager::getVideoLayer() {
    return this->videolayer;
}

MMSFBLayer *MMSFBManager::getGraphicsLayer() {
    return this->graphicslayer;
}

int MMSFBManager::getLayerCount() {
    return this->layercount;
}

