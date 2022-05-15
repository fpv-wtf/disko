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

#include <sys/types.h>
#include <linux/fb.h>
#include "mmsgui/fb/mmsfb.h"
#include "mmsgui/fb/mmsfbsurfacemanager.h"
#include <string.h>
#include <stdlib.h>

/* initialize the mmsfb object */
MMSFB *mmsfb = new MMSFB();


#define INITCHECK  if(!this->initialized){MMSFB_SetError(0,"not initialized");return false;}

void MMSFB_AtExit() {
	if (mmsfb)
		mmsfb->release(); 
}

MMSFB::MMSFB() :
	argc(0),
	argv(NULL),
	initialized(false),
#ifdef  __HAVE_DIRECTFB__
    dfb(NULL),
#endif
#ifdef  __HAVE_FBDEV__
    mmsfbdev(NULL),
#endif
#ifdef __HAVE_XLIB__
    x_display(NULL),
	xv_port(0),
#endif
	outputtype(MMSFB_OT_NONE) {
	// set the atexit routine
	atexit(MMSFB_AtExit);
}

MMSFB::~MMSFB() {
#ifdef __HAVE_XLIB__
	if(this->x_display && this->xv_port) {
		XvUngrabPort(this->x_display, this->xv_port, CurrentTime);
	}
#endif
}

bool MMSFB::init(int argc, char **argv, MMSFBBackend backend, MMSFBOutputType outputtype, MMSFBRectangle x11_win_rect,
				 bool extendedaccel, MMSFBFullScreenMode fullscreen, MMSFBPointerMode pointer,
				 string appl_name, string appl_icon_name, bool hidden) {

    // check if already initialized
    if (this->initialized) {
        MMSFB_SetError(0, "already initialized");
        return false;
    }

    // save arguments
    this->argc = argc;
    this->argv = argv;

    // init layer pointers
    memset(this->layer, 0, sizeof(MMSFBLayer *) * MMSFBLAYER_MAXNUM);

    // basic information mainly needed by X11 initialization
    this->outputtype = outputtype;
    this->x11_win_rect = x11_win_rect;

    // which backend should i use?
	this->backend = backend;
	if (this->backend == MMSFB_BE_DFB) {
#ifdef __HAVE_DIRECTFB__
#else
		MMSFB_SetError(0, "compile DFB support!");
		return false;
#endif
	}
    else
    if (this->backend == MMSFB_BE_X11) {
#ifdef __HAVE_XLIB__
		XInitThreads();
		this->resized=false;
#else
		MMSFB_SetError(0, "compile X11 support!");
		return false;
#endif
    }
    else
	if (this->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
#else
		MMSFB_SetError(0, "compile FBDEV support!");
		return false;
#endif
	}
	else
	if (this->backend == MMSFB_BE_NONE) {
		// fall back, auto detection
		this->backend = MMSFB_BE_DFB;
#ifdef __HAVE_XLIB__
		if ((this->outputtype == MMSFB_OT_X11)&&(extendedaccel)) {
			this->backend = MMSFB_BE_X11;
			XInitThreads();
			this->resized=false;
		}
#endif
	}
	else {
		MMSFB_SetError(0, "wrong backend " + getMMSFBBackendString(backend));
		return false;
	}

    if (this->backend == MMSFB_BE_DFB) {
#ifdef __HAVE_DIRECTFB__
        DFBResult dfbres;

        /* init dfb */
		DirectFBInit(&this->argc,&this->argv);

		/* get interface to dfb */
		if ((dfbres = DirectFBCreate(&this->dfb)) != DFB_OK) {
			MMSFB_SetError(dfbres, "DirectFBCreate() failed");
			return false;
		}
#endif
    }
    else
    if (this->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
		if (this->outputtype == MMSFB_OT_MATROXFB) {
			// matroxfb
    		this->mmsfbdev = new MMSFBDevMatrox();
		}
		else
		if (this->outputtype == MMSFB_OT_DAVINCIFB) {
			// davincifb
    		this->mmsfbdev = new MMSFBDevDavinci();
		}
		else
		if (this->outputtype == MMSFB_OT_OMAPFB) {
			// omapfb
		    DEBUGMSG("MMSGUI", "create new MMSFBDevOmap()");
    		this->mmsfbdev = new MMSFBDevOmap();
		    DEBUGMSG("MMSGUI", "created new MMSFBDevOmap()");
		}
		else {
			// default fbdev
		    DEBUGMSG("MMSGUI", "create generic fbdev");
    		this->mmsfbdev = new MMSFBDev();
		}

    	if (this->mmsfbdev)
			if (!this->mmsfbdev->openDevice()) {
				MMSFB_SetError(0, "MMSFBDEV device cannot be opened");
				return false;
			}
#endif
    }
    else {
#ifdef __HAVE_XLIB__
		// initialize the X11 window
        if (!(this->x_display = XOpenDisplay((char*)0))) {
			MMSFB_SetError(0, "XOpenDisplay() failed");
        	return false;
        }
        this->x_screen = DefaultScreen(this->x_display);

        Window myroot=RootWindow(this->x_display, this->x_screen);
        Window root_ret;
        int myx,myy;
        unsigned int borderw, depthret;
        XGetGeometry(this->x_display, myroot, &root_ret, &myx, &myy, (unsigned int *)&(this->display_w), (unsigned int *)&(this->display_h), &borderw, &depthret);

		//XF86VidModeGetModeLine(this->x_display, 0, &dot, &line);
		/*this->display_w=line.hdisplay;
		this->display_h=line.vdisplay;*/
        printf("w: %d, h: %d\n", this->display_w, this->display_h);

		XSetWindowAttributes x_window_attr;
		x_window_attr.event_mask        = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |PointerMotionMask|EnterWindowMask|ResizeRedirectMask;
		x_window_attr.background_pixel  = 0;
		x_window_attr.border_pixel      = 0;


		unsigned long x_window_mask;
		//this->window_w
		this->fullscreen = fullscreen;
		if (fullscreen == MMSFB_FSM_TRUE || fullscreen == MMSFB_FSM_ASPECT_RATIO) {
			x_window_mask = CWBackPixel | CWBorderPixel |  CWEventMask |CWOverrideRedirect;
			x_window_attr.override_redirect = True;
	/*		int cnt;
			XF86VidModeModeInfo **info;
			XF86VidModeGetAllModeLines(this->x_display, 0, &cnt, &info);
			int best=-1;
			for(int i=0;i<cnt;i++) {
				if((info[i]->hdisplay==w)&&(info[i]->vdisplay==h)) {
					best=i;
					break;
				}
				//printf("w,h: %d %d\n", info[i]->hdisplay,info[i]->vdisplay);
			}
*/
			int x_depth = DefaultDepth(this->x_display, this->x_screen);
			this->x_window = XCreateWindow(this->x_display, DefaultRootWindow(this->x_display), 0, 0, this->display_w, this->display_h, 0, x_depth,
										   InputOutput, CopyFromParent, x_window_mask, &x_window_attr);
		} else {
			this->fullscreen = MMSFB_FSM_FALSE;
			x_window_mask = CWBackPixel | CWBorderPixel |  CWEventMask ;
			x_window_attr.override_redirect = 0;
			int x_depth = DefaultDepth(this->x_display, this->x_screen);
			this->x_window = XCreateWindow(this->x_display, DefaultRootWindow(this->x_display),
										   this->x11_win_rect.x, this->x11_win_rect.y, this->x11_win_rect.w, this->x11_win_rect.h,
										   0, x_depth, InputOutput, CopyFromParent, x_window_mask, &x_window_attr);
		}

		XStoreName(this->x_display, this->x_window, appl_name.c_str());
		XSetIconName(this->x_display, this->x_window, appl_icon_name.c_str());
		XClassHint clhi;
		clhi.res_name=(argc ? basename(argv[0]) : (char*)"disko");
		clhi.res_class=(char*)"disko";
		XSetClassHint(this->x_display, this->x_window,&clhi);
		this->x_gc = XCreateGC(this->x_display, this->x_window, 0, 0);
		if(!hidden) {
			XMapWindow(this->x_display, this->x_window);
			XEvent x_event;
			do {
				XNextEvent(this->x_display, &x_event);
			}
			while (x_event.type != MapNotify || x_event.xmap.event != this->x_window);

			XRaiseWindow(this->x_display, this->x_window);
		}
		// hide X cursor
//		if(this->outputtype != MMS_OT_XSHM) {
		if (pointer != MMSFB_PM_EXTERNAL) {
			Pixmap bm_no;
			Colormap cmap;
			Cursor no_ptr;
			XColor black, dummy;
			static char bm_no_data[] = {0, 0, 0, 0, 0, 0, 0, 0};

			cmap = DefaultColormap(this->x_display, DefaultScreen(this->x_display));
			XAllocNamedColor(this->x_display, cmap, "black", &black, &dummy);
			bm_no = XCreateBitmapFromData(this->x_display, this->x_window, bm_no_data, 8, 8);
			no_ptr = XCreatePixmapCursor(this->x_display, bm_no, bm_no, &black, &black, 0, 0);

			XDefineCursor(this->x_display, this->x_window, no_ptr);
			XFreeCursor(this->x_display, no_ptr);
			if (bm_no != None)
					XFreePixmap(this->x_display, bm_no);
			XFreeColors(this->x_display, cmap, &black.pixel, 1, 0);
		}
		if(!hidden)
			XSetInputFocus(this->x_display, this->x_window,RevertToPointerRoot,CurrentTime);


		if (mmsfb->outputtype == MMSFB_OT_XSHM) {
			// XSHM
	        if (!XShmQueryExtension(this->x_display)) {
				MMSFB_SetError(0, "XShmQueryExtension() failed");
	        	return false;
	        }

	        this->x_visual = DefaultVisual(this->x_display, 0);
	        this->x_depth = DefaultDepth(this->x_display, 0);

//printf("depth=%d\n", this->x_depth);
//exit(0);
		}
		else {
			// XVSHM
			unsigned int 	p_version,
							p_release,
							p_request_base,
							p_event_base,
							p_error_base;
	        if (XvQueryExtension(this->x_display, &p_version, &p_release, &p_request_base, &p_event_base, &p_error_base) != Success) {
				MMSFB_SetError(0, "XvQueryExtension() failed");
	        	return false;
	        }

	        unsigned int num_adaptors;
			XvAdaptorInfo *ai;
			if (XvQueryAdaptors(this->x_display, DefaultRootWindow(this->x_display), &num_adaptors, &ai)) {
				MMSFB_SetError(0, "XvQueryAdaptors() failed");
				return false;
			}
			printf("DISKO: Available xv adaptors:\n");
			for(unsigned int cnt=0;cnt<num_adaptors;cnt++) {
				/* grab the first port with XvImageMask bit */
				if((this->xv_port == 0) &&
				   (ai[cnt].type & XvImageMask) &&
				   (XvGrabPort(this->x_display, ai[cnt].base_id, 0) == Success)) {
					this->xv_port = ai[cnt].base_id;
					printf("  %s (used)\n", ai[cnt].name);
				}
				else
					printf("  %s\n", ai[cnt].name);
			}
			XvFreeAdaptorInfo(ai);
		}
#endif
    }

    this->initialized = true;
    return true;
}

bool MMSFB::release() {
    if (this->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
		if (this->dfb)
			this->dfb->Release(this->dfb);
#endif
    }
    else
    if (this->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
    	if (this->mmsfbdev) {
    		delete this->mmsfbdev;
    		this->mmsfbdev = NULL;
    	}
#endif
    }
	else {
#ifdef __HAVE_XLIB__
#endif
    }

	this->initialized = false;
    return true;
}

bool MMSFB::isInitialized() {
	return this->initialized;
}

MMSFBBackend MMSFB::getBackend() {
	return this->backend;
}

bool MMSFB::getLayer(int id, MMSFBLayer **layer) {

	// check if initialized
    INITCHECK;

    if (this->layer[id]) {
        // i have already the layer
        *layer = this->layer[id];
        return true;
    }

    // create a new layer instance
    *layer = new MMSFBLayer(id);
    if (!*layer) {
        MMSFB_SetError(0, "cannot create new instance of MMSFBLayer");
        return false;
    }
    if (!(*layer)->isInitialized()) {
    	delete *layer;
    	*layer = NULL;
		MMSFB_SetError(0, "cannot initialize MMSFBLayer");
		return false;
	}

    // save this for the next call
    this->layer[id] = *layer;

    return true;
}

void *MMSFB::getX11Window() {
    if (this->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
#endif
    }
    else
	if (this->backend == MMSFB_BE_FBDEV) {
#ifdef  __HAVE_FBDEV__
#endif
	}
    else {
#ifdef __HAVE_XLIB__
    	return &this->x_window;
#endif
    }
    return NULL;
}
void *MMSFB::getX11Display() {
    if (this->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
#endif
    }
    else
	if (this->backend == MMSFB_BE_FBDEV) {
#ifdef  __HAVE_FBDEV__
#endif
	}
    else {
#ifdef __HAVE_XLIB__
    	return this->x_display;
#endif
    }
    return NULL;
}

bool MMSFB::refresh() {
    // check if initialized
    INITCHECK;

    if (this->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
#endif
    }
    else
	if (this->backend == MMSFB_BE_FBDEV) {
#ifdef  __HAVE_FBDEV__
#endif
	}
    else {
#ifdef __HAVE_XLIB__
    	MMSFBSurface *suf;
    	if (this->layer[0]->getSurface(&suf))
    		suf->refresh();
#endif
    }

    return true;
}

bool MMSFB::createSurface(MMSFBSurface **surface, int w, int h, MMSFBSurfacePixelFormat pixelformat, int backbuffer, bool systemonly) {
    /* check if initialized */
    INITCHECK;

    /* create or reuse a surface */
    *surface = mmsfbsurfacemanager->createSurface(w, h, pixelformat, backbuffer, systemonly);

    if (*surface)
        return true;
    else
        return false;
}

#ifdef  __HAVE_DIRECTFB__
bool MMSFB::createImageProvider(IDirectFBImageProvider **provider, string filename) {
	*provider = NULL;
    if (this->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
		DFBResult   dfbres;

		/* check if initialized */
		INITCHECK;

		/* create the provider */
		if ((dfbres=this->dfb->CreateImageProvider(this->dfb, filename.c_str(), provider)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFB::CreateImageProvider(" + filename + ") failed");
			return false;
		}

		return true;
#endif
    }
    if (this->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
#endif
    }
    else {
#ifdef __HAVE_XLIB__
#endif
    }
	return false;
}
#endif

bool MMSFB::createFont(MMSFBFont **font, string filename, int width, int height) {
	// check if initialized
	INITCHECK;

	// create new instance of MMSFBFont
	*font = new MMSFBFont(filename, width, height);
	if (!*font) {
		MMSFB_SetError(0, "cannot create new MMSFBFont instance for " + filename);
		return false;
	}
	if (!(*font)->isInitialized()) {
		delete *font;
		*font = NULL;
		MMSFB_SetError(0, "cannot initialize new MMSFBFont instance for " + filename);
		return false;
	}
	return true;
}

#ifdef __HAVE_XLIB__
bool MMSFB::resizeWindow() {
	printf("resize w,h: %d,%d\n", this->target_window_w, this->target_window_h );
	XWindowChanges chg;
	chg.width=this->target_window_w;
	chg.height=this->target_window_h;
	printf("rc %d\n",XConfigureWindow(this->x_display, this->x_window,CWWidth|CWHeight, &chg));
	//XMoveResizeWindow(this->x_display, this->x_window, this->target_window_w, this->target_window_h);
	return true;
}
#endif
