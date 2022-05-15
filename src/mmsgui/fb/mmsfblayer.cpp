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

#include "mmsgui/fb/mmsfblayer.h"
#include "mmsgui/fb/mmsfb.h"
#include <string.h>
#include <cerrno>

#ifdef __HAVE_XLIB__
#include <sys/shm.h>
#endif


#ifdef  __HAVE_DIRECTFB__
D_DEBUG_DOMAIN( MMS_Layer, "MMS/Layer", "MMS Layer" );
#endif

#ifdef __HAVE_XLIB__
#define GUID_YUV12_PLANAR ('2'<<24)|('1'<<16)|('V'<<8)|'Y')
#endif


// static variables
bool MMSFBLayer::firsttime_createsurface		= true;
bool MMSFBLayer::firsttime_createwindow_usealpha= true;
bool MMSFBLayer::firsttime_createwindow_noalpha	= true;


#define INITCHECK  if(!this->initialized){MMSFB_SetError(0,"not initialized");return false;}


MMSFBLayer::MMSFBLayer(int id) {
    // init me
	this->initialized = false;
    this->surface = NULL;
    this->flipflags = MMSFB_FLIP_NONE;
    this->config.avail = false;
    this->config.id = id;
    this->config.window_pixelformat = MMSFB_PF_ARGB;
    this->config.surface_pixelformat = MMSFB_PF_ARGB;

#ifdef __HAVE_FBDEV__
    this->mmsfbdev_surface = NULL;
#endif

#ifdef __HAVE_XLIB__
    this->x_image1 = NULL;
    this->x_image2 = NULL;
    this->x_image_scaler = NULL;
    this->scaler = NULL;
    this->xv_image1 = NULL;
    this->xv_image2 = NULL;
#endif

    if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
		// get the layer
		DFBResult	dfbres;
		this->dfblayer = NULL;
		if ((dfbres = mmsfb->dfb->GetDisplayLayer(mmsfb->dfb, this->config.id, &this->dfblayer)) != DFB_OK) {
			this->dfblayer = NULL;
			MMSFB_SetError(dfbres, "IDirectFB::GetDisplayLayer(" + iToStr(id) + ") failed");
			return;
		}
		this->initialized = true;
#endif
    }
    else
    if (mmsfb->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
        if (mmsfb->mmsfbdev) {
			// test layer initialization
			if (!mmsfb->mmsfbdev->testLayer(this->config.id)) {
				MMSFB_SetError(0, "init test of layer " + iToStr(this->config.id) + " failed!");
				return;
			}

			this->initialized = true;
        }
#endif
    }
    else {
#ifdef __HAVE_XLIB__
    	// check layer 0
        if (this->config.id != 0) {
			MMSFB_SetError(0, "X11 support needs layer 0!");
        	return;
        }

		// fill my config partly from mmsfb
		this->config.w = mmsfb->x11_win_rect.w;
		this->config.h = mmsfb->x11_win_rect.h;
		this->config.pixelformat = MMSFB_PF_NONE;
		this->config.buffermode = MMSFB_BM_BACKSYSTEM;
		this->config.options = MMSFB_LO_NONE;

		if (mmsfb->outputtype == MMSFB_OT_XSHM) {
			// XSHM
			switch (mmsfb->x_depth) {
			case 16:
				this->config.pixelformat = MMSFB_PF_RGB16;
				break;
			case 24:
				this->config.pixelformat = MMSFB_PF_RGB24;
				break;
			case 32:
				this->config.pixelformat = MMSFB_PF_RGB32;
				break;
			}

			// create x11 buffer #1
			XLockDisplay(mmsfb->x_display);
			this->x_image1 = XShmCreateImage(mmsfb->x_display, mmsfb->x_visual, mmsfb->x_depth, ZPixmap,
											 NULL, &this->x_shminfo1, this->config.w, this->config.h);
			if (!this->x_image1) {
				XUnlockDisplay(mmsfb->x_display);
				MMSFB_SetError(0, "XShmCreateImage() failed");
				return;
			}

			// map shared memory for x-server communication
			this->x_shminfo1.shmid    = shmget(IPC_PRIVATE, this->x_image1->bytes_per_line * this->x_image1->height, IPC_CREAT | 0777);
			this->x_shminfo1.shmaddr  = this->x_image1->data = (char *)shmat(this->x_shminfo1.shmid, 0, 0);
			this->x_shminfo1.readOnly = False;

			// attach the x-server to that segment
			if (!XShmAttach(mmsfb->x_display, &this->x_shminfo1)) {
				XFree(this->x_image1);
				this->x_image1 = NULL;
				XUnlockDisplay(mmsfb->x_display);
				MMSFB_SetError(0, "XShmAttach() failed");
				return;
			}

			// create x11 buffer #2
			this->x_image2 = XShmCreateImage(mmsfb->x_display, mmsfb->x_visual, mmsfb->x_depth, ZPixmap,
											 NULL, &this->x_shminfo2, this->config.w, this->config.h);
			if (!this->x_image2) {
				XUnlockDisplay(mmsfb->x_display);
				MMSFB_SetError(0, "XShmCreateImage() failed");
				return;
			}

			// map shared memory for x-server communication
			this->x_shminfo2.shmid    = shmget(IPC_PRIVATE, this->x_image2->bytes_per_line * this->x_image2->height, IPC_CREAT | 0777);
			this->x_shminfo2.shmaddr  = this->x_image2->data = (char *)shmat(this->x_shminfo2.shmid, 0, 0);
			this->x_shminfo2.readOnly = False;

			// attach the x-server to that segment
			if (!XShmAttach(mmsfb->x_display, &this->x_shminfo2)) {
				XFree(this->x_image2);
				this->x_image2 = NULL;
				XUnlockDisplay(mmsfb->x_display);
				MMSFB_SetError(0, "XShmAttach() failed");
				return;
			}

			// check if rgb32 instead of rgb24
			if (this->config.pixelformat == MMSFB_PF_RGB24)
				if (this->x_image1->bytes_per_line / this->config.w >= 4)
					this->config.pixelformat = MMSFB_PF_RGB32;

			if (mmsfb->fullscreen == MMSFB_FSM_TRUE || mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {

				// calc ratio
				MMSFBRectangle dest;
				calcAspectRatio(this->config.w, this->config.h, mmsfb->display_w, mmsfb->display_h, dest,
								(mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO), false);

				// create scale buffer
				this->x_image_scaler = XShmCreateImage(mmsfb->x_display, mmsfb->x_visual, mmsfb->x_depth, ZPixmap,
													   NULL, &this->x_shminfo_scaler, dest.w, dest.h);
				if (!this->x_image_scaler) {
					XUnlockDisplay(mmsfb->x_display);
					MMSFB_SetError(0, "XShmCreateImage() failed");
					return;
				}

				// map shared memory for x-server communication
				this->x_shminfo_scaler.shmid    = shmget(IPC_PRIVATE, this->x_image_scaler->bytes_per_line * this->x_image_scaler->height, IPC_CREAT | 0777);
				this->x_shminfo_scaler.shmaddr  = this->x_image_scaler->data = (char *)shmat(this->x_shminfo_scaler.shmid, 0, 0);
				this->x_shminfo_scaler.readOnly = False;

				// attach the x-server to that segment
				if (!XShmAttach(mmsfb->x_display, &this->x_shminfo_scaler)) {
					XFree(this->x_image_scaler);
					this->x_image_scaler = NULL;
					XUnlockDisplay(mmsfb->x_display);
					MMSFB_SetError(0, "XShmAttach() failed");
					return;
				}

				// create a scaler surface
				this->scaler = new MMSFBSurface(dest.w, dest.h, this->config.pixelformat,
											this->x_image_scaler, NULL, NULL);
				if (!this->scaler) {
					XUnlockDisplay(mmsfb->x_display);
					MMSFB_SetError(0, "cannot create scaler surface");
					return;
				}

				// we must switch extended accel on
				this->scaler->setExtendedAcceleration(true);
			}
			XUnlockDisplay(mmsfb->x_display);
		}
		else {
			// XVSHM
			this->config.pixelformat = MMSFB_PF_YV12;

			// important to set the image width to a multiple of 128
			// a few hardware need this to create a buffer where to U/V planes immediately follows the Y plane
			int image_width = this->config.w & ~0x7f;
			if (this->config.w & 0x7f)
				image_width += 0x80;

			// get id for yv12 pixelformat
			XLockDisplay(mmsfb->x_display);
			int nFormats, xvPixFormat = (('2'<<24)|('1'<<16)|('V'<<8)|'Y');
			XvImageFormatValues *formats = XvListImageFormats(mmsfb->x_display, mmsfb->xv_port, &nFormats);
			if(formats) {
				for(int i = 0; i < nFormats; i++) {
					if(formats[i].type == XvYUV && formats[i].format == XvPlanar) {
						xvPixFormat = formats[i].id;
						break;
					}
				}
				XFree(formats);
			}

			// create x11 buffer #1
			this->xv_image1 = XvShmCreateImage(mmsfb->x_display, mmsfb->xv_port, xvPixFormat, 0, image_width, this->config.h, &this->xv_shminfo1);
			if(!this->xv_image1) {
				XUnlockDisplay(mmsfb->x_display);
				MMSFB_SetError(0, "XvShmCreateImage() failed");
				return;
			}
			if(this->xv_image1->data_size == 0) {
				XFree(this->xv_image1);
				XUnlockDisplay(mmsfb->x_display);
				this->xv_image1 = NULL;
				MMSFB_SetError(0, "XvShmCreateImage() returned zero size");
				return;
			}

			// map shared memory for x-server communication
			this->xv_shminfo1.shmid = shmget(IPC_PRIVATE, this->xv_image1->data_size, IPC_CREAT | 0777);
			if(this->xv_shminfo1.shmid < 0) {
				MMSFB_SetError(0, string("Error in shmget: ") + strerror(errno));
				XFree(this->xv_image1);
				XUnlockDisplay(mmsfb->x_display);
				this->xv_image1 = NULL;
				return;
			}

			this->xv_shminfo1.shmaddr  = this->xv_image1->data = (char *)shmat(this->xv_shminfo1.shmid, 0, 0);
			if(!this->xv_shminfo1.shmaddr || (this->xv_shminfo1.shmaddr == (char*)-1)) {
				MMSFB_SetError(0, string("Error in shmat: ") + strerror(errno));
				XFree(this->xv_image1);
				XUnlockDisplay(mmsfb->x_display);
				this->xv_image1 = NULL;
				return;
			}

			this->xv_shminfo1.readOnly = False;

			// attach the x-server to that segment
			if (!XShmAttach(mmsfb->x_display, &this->xv_shminfo1)) {
				XFree(this->xv_image1);
				XUnlockDisplay(mmsfb->x_display);
				this->xv_image1 = NULL;
				MMSFB_SetError(0, "XShmAttach() failed");
				return;
			}

			//XFlush(mmsfb->x_display);
			XSync(mmsfb->x_display, False);
		    shmctl(this->xv_shminfo1.shmid, IPC_RMID, 0);

			// create x11 buffer #2
			this->xv_image2 = XvShmCreateImage(mmsfb->x_display, mmsfb->xv_port, xvPixFormat, 0, image_width, this->config.h, &this->xv_shminfo2);
			if (!this->xv_image2) {
				XFree(this->xv_image1);
				XUnlockDisplay(mmsfb->x_display);
				this->xv_image1 = NULL;
				MMSFB_SetError(0, "XvShmCreateImage() failed");
				return;
			}
			if(this->xv_image2->data_size == 0) {
				XFree(this->xv_image1);
				XFree(this->xv_image2);
				XUnlockDisplay(mmsfb->x_display);
				this->xv_image1 = NULL;
				this->xv_image2 = NULL;
				MMSFB_SetError(0, "XvShmCreateImage() returned zero size");
				return;
			}

			// map shared memory for x-server communication
			this->xv_shminfo2.shmid    = shmget(IPC_PRIVATE, this->xv_image2->data_size, IPC_CREAT | 0777);
			if(this->xv_shminfo2.shmid < 0) {
				MMSFB_SetError(0, string("Error in shmget: ") + strerror(errno));
				XFree(this->xv_image1);
				XFree(this->xv_image2);
				XUnlockDisplay(mmsfb->x_display);
				this->xv_image1 = NULL;
				this->xv_image2 = NULL;
				return;
			}

			this->xv_shminfo2.shmaddr  = this->xv_image2->data = (char *)shmat(this->xv_shminfo2.shmid, 0, 0);
			if(!this->xv_shminfo2.shmaddr || (this->xv_shminfo2.shmaddr == (char*)-1)) {
				MMSFB_SetError(0, string("Error in shmat: ") + strerror(errno));
				XFree(this->xv_image1);
				XFree(this->xv_image2);
				XUnlockDisplay(mmsfb->x_display);
				this->xv_image1 = NULL;
				this->xv_image2 = NULL;
				return;
			}

			this->xv_shminfo2.readOnly = False;

			// attach the x-server to that segment
			if (!XShmAttach(mmsfb->x_display, &this->xv_shminfo2)) {
				XFree(this->xv_image1);
				XFree(this->xv_image2);
				XUnlockDisplay(mmsfb->x_display);
				this->xv_image1 = NULL;
				this->xv_image2 = NULL;
				MMSFB_SetError(0, "XShmAttach() failed");
				return;
			}

			//XFlush(mmsfb->x_display);
			XSync(mmsfb->x_display, False);
		    shmctl(this->xv_shminfo2.shmid, IPC_RMID, 0);

			XUnlockDisplay(mmsfb->x_display);
		}

		this->initialized = true;
#endif
    }

    // get the current config
    if (this->initialized) {
    	MMSFBLayerConfig config;
    	getConfiguration(&config);
    }
}


MMSFBLayer::~MMSFBLayer() {
    if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
		if (this->dfblayer)
			this->dfblayer->Release(this->dfblayer);
#endif
    }
    else
    if (mmsfb->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
        if (this->mmsfbdev_surface)
        	delete this->mmsfbdev_surface;
#endif
    }
    else {
#ifdef __HAVE_XLIB__
		if (mmsfb->outputtype == MMSFB_OT_XSHM) {
			// XSHM
			if (this->x_image1)
				XFree(this->x_image1);
			if (this->x_image2)
				XFree(this->x_image2);
		}
		else {
			// XVSHM
			if (this->xv_image1)
				XFree(this->xv_image1);
			if (this->xv_image2)
				XFree(this->xv_image2);
		}
#endif
    }
}

bool MMSFBLayer::isInitialized() {
    if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
    	return (this->dfblayer != NULL);
#endif
    }
    else
    if (mmsfb->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
    	return this->initialized;
#endif
    }
    else {
#ifdef __HAVE_XLIB__
		if (mmsfb->outputtype == MMSFB_OT_XSHM) {
			// XSHM

			return (this->x_image1 != NULL);
		}
		else {
			// XVSHM

			return (this->xv_image1 != NULL);
		}
#endif
    }

    return false;
}

bool MMSFBLayer::getID(int *id) {

    // check if initialized
    INITCHECK;

    // get configuration
	MMSFBLayerConfig config;
    if (!getConfiguration(&config))
        return false;

    // fill return values
    *id = this->config.id;

    return true;
}

bool MMSFBLayer::setExclusiveAccess() {

    // check if initialized
    INITCHECK;

    if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
		DFBResult   dfbres;

		/* set cooperative level to exclusive */
		if ((dfbres=this->dfblayer->SetCooperativeLevel(this->dfblayer, DLSCL_EXCLUSIVE)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBDisplayLayer::SetCooperativeLevel(DLSCL_EXCLUSIVE) failed");
			return false;
		}

		return true;
#endif
    }
    else
    if (mmsfb->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
    	return true;
#endif
    }
    else {
#ifdef __HAVE_XLIB__
    	return true;
#endif
    }

    return false;
}

bool MMSFBLayer::getConfiguration(MMSFBLayerConfig *config) {

    // check if initialized
    INITCHECK;

    if (this->config.avail) {
        // fill return config
        if (config)
            *config = this->config;
        return true;
    }

    if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
		DFBResult               dfbres;
		DFBDisplayLayerConfig   dlc;

		/* get configuration */
		if ((dfbres=this->dfblayer->GetConfiguration(this->dfblayer, &dlc)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBDisplayLayer::GetConfiguration() failed");
			return false;
		}

		// fill my config
		this->config.avail = true;
		this->config.w = dlc.width;
		this->config.h = dlc.height;
		this->config.pixelformat = getMMSFBPixelFormatFromDFBPixelFormat(dlc.pixelformat);
		this->config.buffermode = getDFBLayerBufferModeString(dlc.buffermode);
		this->config.options = getDFBLayerOptionsString(dlc.options);

#endif
    }
    else
    if (mmsfb->backend == MMSFB_BE_FBDEV) {
#ifdef  __HAVE_FBDEV__
		this->config.avail = true;
#endif
    }
    else {
		this->config.avail = true;
    }

    if (!config) {
    	DEBUGMSG("MMSGUI", "Layer properties:");

/*        if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
            DEBUGMSG("MMSGUI", " backend:     DFB");
#endif
        }
        else
        if (mmsfb->backend == MMSFB_BACKEND_FBDEV) {
#ifdef __HAVE_FBDEV__
            DEBUGMSG("MMSGUI", " backend:     FBDEV");
#endif
        }
        else {
#ifdef __HAVE_XLIB__
    		if (mmsfb->outputtype == MMS_OT_XSHM) {
    			// XSHM

    			DEBUGMSG("MMSGUI", " backend:     XSHM");
    		}
    		else {
    			// XVSHM

    			DEBUGMSG("MMSGUI", " backend:     XVSHM");
    		}
#endif
        }
*/

    	DEBUGMSG("MMSGUI", " backend:     " + getMMSFBBackendString(mmsfb->backend));

        DEBUGMSG("MMSGUI", " size:        " + iToStr(this->config.w) + "x" + iToStr(this->config.h));

    	DEBUGMSG("MMSGUI", " pixelformat: " + getMMSFBPixelFormatString(this->config.pixelformat));

	    if (this->config.buffermode!="")
	    	DEBUGMSG("MMSGUI", " buffermode:  " + this->config.buffermode);
	    else
	    	DEBUGMSG("MMSGUI", " buffermode:  NONE");

	    if (this->config.options!="")
	    	DEBUGMSG("MMSGUI", " options:     " + this->config.options);
	    else
	    	DEBUGMSG("MMSGUI", " options:     NONE");
    }

    // fill return config
    if (config)
        *config = this->config;

    return true;
}

bool MMSFBLayer::getResolution(int *w, int *h) {

    // check if initialized
    INITCHECK;

    /* get configuration */
	MMSFBLayerConfig config;
    if (!getConfiguration(&config))
        return false;

    /* fill return values */
    *w = this->config.w;
    *h = this->config.h;

    return true;
}

bool MMSFBLayer::getPixelFormat(MMSFBSurfacePixelFormat *pixelformat) {

    // check if initialized
    INITCHECK;

    /* get configuration */
	MMSFBLayerConfig config;
    if (!getConfiguration(&config))
        return false;

    /* fill return values */
    *pixelformat = this->config.pixelformat;

    return true;
}

bool MMSFBLayer::setConfiguration(int w, int h, MMSFBSurfacePixelFormat pixelformat, string buffermode, string options,
								  MMSFBSurfacePixelFormat window_pixelformat, MMSFBSurfacePixelFormat surface_pixelformat) {

    // check if initialized
    INITCHECK;

    if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
		// get configuration
		MMSFBLayerConfig config;
		if (!getConfiguration(&config))
			return false;

		// change the config of the layer only, if the attributes are changed
		if (config.w != w || config.h != h || config.pixelformat != pixelformat || config.buffermode != buffermode) {
			// change config data
			DFBResult dfbres;
			DFBDisplayLayerConfig dlc;
			dlc.flags = DLCONF_NONE;
			dlc.width = w;
			dlc.height = h;
			dlc.pixelformat = getDFBPixelFormatFromMMSFBPixelFormat(pixelformat);
			dlc.buffermode = getDFBLayerBufferModeFromString(buffermode);
			dlc.options = getDFBLayerOptionsFromString(options);

			if (dlc.width > 0)
				dlc.flags = (DFBDisplayLayerConfigFlags)(dlc.flags | DLCONF_WIDTH);
			if (dlc.height > 0)
				dlc.flags = (DFBDisplayLayerConfigFlags)(dlc.flags | DLCONF_HEIGHT);
			if (dlc.pixelformat != DSPF_UNKNOWN)
				dlc.flags = (DFBDisplayLayerConfigFlags)(dlc.flags | DLCONF_PIXELFORMAT);
			if (dlc.buffermode != DLBM_UNKNOWN)
				dlc.flags = (DFBDisplayLayerConfigFlags)(dlc.flags | DLCONF_BUFFERMODE);
		  //  if (dlc.options != DLOP_NONE) {
				DEBUGOUT("\nSET OPTIONS 0x%08x!!!!\n", dlc.options);
				dlc.flags = (DFBDisplayLayerConfigFlags)(dlc.flags | DLCONF_OPTIONS);
		  //  }

			// test configuration
			DFBDisplayLayerConfigFlags failedFlags;
			if ((dfbres=this->dfblayer->TestConfiguration(this->dfblayer, &dlc, &failedFlags)) != DFB_OK) {
				if(failedFlags & DLCONF_PIXELFORMAT) {
					MMSFB_SetError(dfbres, "IDirectFBDisplayLayer::TestConfiguration(" + iToStr(w) + "x" + iToStr(h) + "," + getMMSFBPixelFormatString(pixelformat) + "," + buffermode + "," + options + ") failed");
					DEBUGMSG("MMSGUI", "Your configuration contains a pixelformat that is not supported.");
					return false;
				}
				if(failedFlags & DLCONF_BUFFERMODE) {
					MMSFB_SetError(dfbres, "IDirectFBDisplayLayer::TestConfiguration(" + iToStr(w) + "x" + iToStr(h) + "," + getMMSFBPixelFormatString(pixelformat) + "," + buffermode + "," + options + ") failed");
					DEBUGMSG("MMSGUI", "Your configuration contains a buffermode that is not supported.");
					return false;
				}
				if(failedFlags & DLCONF_OPTIONS) {
					MMSFB_SetError(dfbres, "IDirectFBDisplayLayer::TestConfiguration(" + iToStr(w) + "x" + iToStr(h) + "," + getMMSFBPixelFormatString(pixelformat) + "," + buffermode + "," + options + ") failed");
					DEBUGMSG("MMSGUI", "Your configuration contains options that are not supported.");
					return false;
				}

				/* check if desired resolution is unsupported */
				if(failedFlags & DLCONF_WIDTH)
					dlc.flags = (DFBDisplayLayerConfigFlags)(dlc.flags & ~DLCONF_WIDTH);
				if(failedFlags & DLCONF_HEIGHT)
					dlc.flags = (DFBDisplayLayerConfigFlags)(dlc.flags & ~DLCONF_HEIGHT);
				if ((dfbres=this->dfblayer->TestConfiguration(this->dfblayer, &dlc, &failedFlags)) != DFB_OK) {
					MMSFB_SetError(dfbres, "IDirectFBDisplayLayer::TestConfiguration(" + iToStr(w) + "x" + iToStr(h) + "," + getMMSFBPixelFormatString(pixelformat) + "," + buffermode + "," + options + ") failed");
					return false;
				}
				DEBUGMSG("MMSGUI", "Your configuration contains a resolution that is not supported.");
			}

			// set configuration
			if((dfbres = this->dfblayer->SetConfiguration(this->dfblayer, &dlc)) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBDisplayLayer::SetConfiguration(" + iToStr(w) + "x" + iToStr(h) + "," + getMMSFBPixelFormatString(pixelformat) + "," + buffermode + "," + options + ") failed");
				return false;
			}
		}

		// get configuration
		this->config.avail = false;
		if (!getConfiguration())
			return false;

		// set background
		this->dfblayer->SetBackgroundMode(this->dfblayer, DLBM_COLOR);
		this->dfblayer->SetBackgroundColor(this->dfblayer, 0, 0, 0, 0);

		// set special config
		this->config.window_pixelformat = window_pixelformat;
		this->config.surface_pixelformat = surface_pixelformat;

		return true;
#endif
    }
    else
    if (mmsfb->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
        if (!mmsfb->mmsfbdev)
        	return false;

        // initializing layer
		if (!mmsfb->mmsfbdev->initLayer(this->config.id, w, h, pixelformat,
			(buffermode == MMSFB_BM_BACKVIDEO)?1:(buffermode == MMSFB_BM_TRIPLE)?2:0)) {
			MMSFB_SetError(0, "init layer " + iToStr(this->config.id) + " failed!");
			return false;
		}

		// get fb memory ptr
		MMSFBSurfacePlanesBuffer buffers;
		memset(&buffers, 0, sizeof(buffers));
		if (!mmsfb->mmsfbdev->getFrameBufferPtr(this->config.id, buffers, &this->config.w, &this->config.h)) {
			MMSFB_SetError(0, "getFrameBufferPtr() failed");
			return false;
		}
		mmsfb->mmsfbdev->getPixelFormat(this->config.id, &this->config.pixelformat);
		this->config.buffermode = buffermode;
		this->config.options = MMSFB_LO_NONE;

		// create a new surface instance for the framebuffer memory
		this->mmsfbdev_surface = new MMSFBSurface(this->config.w, this->config.h, this->config.pixelformat,
												  MMSFB_MAX_SURFACE_PLANES_BUFFERS - 1, buffers);
		if (!this->mmsfbdev_surface) {
			MMSFB_SetError(0, "cannot create new instance of MMSFBSurface");
			return false;
		}

		// we must switch extended accel on
		this->mmsfbdev_surface->setExtendedAcceleration(true);

    	// mark this surface as a layer surface
		this->mmsfbdev_surface->setLayerSurface();

		// get configuration
		this->config.avail = false;
		if (!getConfiguration())
			return false;

		// set special config
		this->config.window_pixelformat = window_pixelformat;
		this->config.surface_pixelformat = surface_pixelformat;

		return true;
#endif
    }
    else {
#ifdef __HAVE_XLIB__
		// get configuration
		this->config.avail = false;
		if (!getConfiguration())
			return false;

		// if we use XLIB, currently we cannot change the layer attributes
		this->config.window_pixelformat = window_pixelformat;
		this->config.surface_pixelformat = surface_pixelformat;

		return true;
#endif
    }

    return false;
}

bool MMSFBLayer::setOpacity(unsigned char opacity) {

    // check if initialized
    INITCHECK;

    if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
		DFBResult   dfbres;

		/* invert the opacity for inverted pixelformats */
		if (this->config.pixelformat == MMSFB_PF_AiRGB) {
			opacity = 255 - opacity;
		}

		/* set the opacity */
		if ((dfbres=this->dfblayer->SetOpacity(this->dfblayer, opacity)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBDisplayLayer::SetOpacity(" + iToStr(opacity) + ") failed");
			return false;
		}

		return true;
#endif
    }

    return false;
}

bool MMSFBLayer::setLevel(int level) {

    // check if initialized
    INITCHECK;

    if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
		DFBResult   dfbres;

		/* set the opacity */
		if ((dfbres=this->dfblayer->SetLevel(this->dfblayer, level)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBDisplayLayer::SetLevel(" + iToStr(level) + ") failed");
			return false;
		}
		return true;
#endif
    }

    return false;
}

bool MMSFBLayer::getSurface(MMSFBSurface **surface) {

	// check if initialized
    INITCHECK;

    if (this->surface) {
        // i have already a surface
        *surface = this->surface;
        DEBUGMSG("MMSGUI", "have already a surface");
        return true;
    }

    if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
		// get layers surface
		DFBResult           dfbres;
		IDirectFBSurface    *dfbsurface;
    	DEBUGMSG("MMSGUI", "calling DFB->GetSurface()");
		if ((dfbres=this->dfblayer->GetSurface(this->dfblayer, &dfbsurface)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBDisplayLayer::GetSurface() failed");
			return false;
		}
    	DEBUGMSG("MMSGUI", "setting blitting flags");
		dfbsurface->SetBlittingFlags(dfbsurface, DSBLIT_NOFX);

		// create a new surface instance
		*surface = new MMSFBSurface(dfbsurface);
		if (!*surface) {
			dfbsurface->Release(dfbsurface);
			MMSFB_SetError(0, "cannot create new instance of MMSFBSurface");
			return false;
		}
#endif
    }
    else
    if (mmsfb->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
    	if (this->config.buffermode == MMSFB_BM_FRONTONLY) {
    		// we have only the front buffer, no backbuffer to be allocated
    		*surface = this->mmsfbdev_surface;
			if (!*surface) {
				MMSFB_SetError(0, "layer surface is not initialized");
				return false;
			}
    	}
    	else
		if (this->config.buffermode == MMSFB_BM_BACKSYSTEM) {
			// create a new backbuffer surface instance
			*surface = new MMSFBSurface(this->config.w, this->config.h, this->config.pixelformat);
			if (!*surface) {
				MMSFB_SetError(0, "cannot create new instance of MMSFBSurface");
				return false;
			}

			// the surface has to know the fbdev surface, so surface->flip() can update the fbdev memory
			(*surface)->config.surface_buffer->mmsfbdev_surface = this->mmsfbdev_surface;

			// we must switch extended accel on
			(*surface)->setExtendedAcceleration(true);
		}
		else {
    		// we assume that we have at least one backbuffer in video memory
    		*surface = this->mmsfbdev_surface;
			if (!*surface) {
				MMSFB_SetError(0, "layer surface is not initialized");
				return false;
			}

			// the surface has to know the fbdev surface, so surface->flip() can update the fbdev memory
			// here we point to the same surface, in this case the flip can save the memcpy() and
			// can use the hardware panning instead
			(*surface)->config.surface_buffer->mmsfbdev_surface = this->mmsfbdev_surface;
    	}
#endif
    }
    else {
#ifdef __HAVE_XLIB__
		if (mmsfb->outputtype == MMSFB_OT_XSHM) {
			// XSHM
			if ((!this->x_image1)||(!this->x_image2)) {
				MMSFB_SetError(0, "x_image not available, cannot get surface");
				return false;
			}

			// create a new surface instance
			*surface = new MMSFBSurface(this->config.w, this->config.h, this->config.pixelformat,
										this->x_image1, this->x_image2, this->scaler);
			if (!*surface) {
				MMSFB_SetError(0, "cannot create new instance of MMSFBSurface");
				return false;
			}

			// we must switch extended accel on
			(*surface)->setExtendedAcceleration(true);
		}
		else {
			// XVSHM
			if ((!this->xv_image1)||(!this->xv_image2)) {
				MMSFB_SetError(0, "xv_image not available, cannot get surface");
				return false;
			}

			// create a new surface instance
			*surface = new MMSFBSurface(this->config.w, this->config.h, this->config.pixelformat,
										this->xv_image1, this->xv_image2);
			if (!*surface) {
				MMSFB_SetError(0, "cannot create new instance of MMSFBSurface");
				return false;
			}

			// we must switch extended accel on
			(*surface)->setExtendedAcceleration(true);
		}
#endif
    }

    // save this for the next call
    this->surface = *surface;

    if (this->surface) {
    	// mark this surface as a layer surface
    	this->surface->setLayerSurface();

		// clear all surface buffers
    	int bufnum = 0;
    	this->surface->getNumberOfBuffers(&bufnum);
    	this->surface->clear();
		this->surface->flip();
    	while (bufnum > 1) {
			this->surface->clear();
			this->surface->flip();
			bufnum--;
		}

	    // initialize the flip flags for the layer surface
	    this->surface->setFlipFlags(this->flipflags);

	    return true;
    }

    return false;
}

bool MMSFBLayer::setFlipFlags(MMSFBFlipFlags flags) {
	this->flipflags = flags;

	/* if the layer surface does exist, update it */
	if (this->surface)
	    this->surface->setFlipFlags(this->flipflags);

	return true;
}

bool MMSFBLayer::releaseLayer() {

	// check if initialized
    INITCHECK;

    if (mmsfb->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
        if (mmsfb->mmsfbdev) {
        	return mmsfb->mmsfbdev->releaseLayer(this->config.id);
        }
#endif
    }

    return false;
}

bool MMSFBLayer::restoreLayer() {

    // check if initialized
    INITCHECK;

    if (mmsfb->backend == MMSFB_BE_FBDEV) {
#ifdef __HAVE_FBDEV__
        if (mmsfb->mmsfbdev) {
        	return mmsfb->mmsfbdev->restoreLayer(this->config.id);
        }
#endif
    }

    return false;
}

bool MMSFBLayer::createSurface(MMSFBSurface **surface, int w, int h,
							   MMSFBSurfacePixelFormat pixelformat, int backbuffer) {

    // check if initialized
    INITCHECK;

    if (pixelformat == MMSFB_PF_NONE) {
    	pixelformat = this->config.surface_pixelformat;

/*toberemoved        pixelformat = this->config.pixelformat;
        if (!isAlphaPixelFormat(pixelformat)) {
        	// the gui internally needs surfaces with alpha channel
        	// now we have to decide if we are working in RGB or YUV color space
        	pixelformat = this->config.surface_pixelformat;
        	if ((pixelformat == MMSFB_PF_NONE)||((pixelformat != MMSFB_PF_ARGB)&&(pixelformat != MMSFB_PF_AiRGB)&&(pixelformat != MMSFB_PF_AYUV))) {
        		// use autodetection
	        	if (!isRGBPixelFormat(pixelformat))
		            // so switch all non-alpha pixelformats to AYUV
		            pixelformat = MMSFB_PF_AYUV;
	            else
		            // so switch all non-alpha pixelformats to ARGB
		            pixelformat = MMSFB_PF_ARGB;
        	}
        }
        else
        if (isIndexedPixelFormat(pixelformat))
            // the gui internally needs non-indexed surfaces
            // so switch all indexed pixelformats to ARGB
            pixelformat = MMSFB_PF_ARGB;
*/
    }

    if (firsttime_createsurface) {
    	printf("DISKO: Pixelformat %s is used for surfaces.\n", getMMSFBPixelFormatString(pixelformat).c_str());
    	firsttime_createsurface = false;
    }
    return mmsfb->createSurface(surface, w, h, pixelformat, backbuffer, (this->config.buffermode == MMSFB_BM_BACKSYSTEM));
}

bool MMSFBLayer::createWindow(MMSFBWindow **window, int x, int y, int w, int h,
							  MMSFBSurfacePixelFormat pixelformat, bool usealpha, int backbuffer) {

    // check if initialized
    INITCHECK;

    /* check if i am the right layer */
    MMSFBLayer *layer;
    mmsfbwindowmanager->getLayer(&layer);
    if (layer != this) {
        MMSFB_SetError(0, "not the right layer, cannot create MMSFBWindow");
        return false;
    }

    if (pixelformat == MMSFB_PF_NONE) {
    	if (usealpha) {
    		// use preset window pixelformat
    		pixelformat = this->config.window_pixelformat;
    	}
    	else {
    		// use layer pixelformat
    		pixelformat = this->config.pixelformat;
    	    if (isAlphaPixelFormat(pixelformat)) {
				// switch all alpha pixelformats to RGB32
				pixelformat = MMSFB_PF_RGB32;
    	    }
    	    else
    	    if (isIndexedPixelFormat(pixelformat)) {
				// switch all indexed pixelformats to RGB32
				pixelformat = MMSFB_PF_RGB32;
    	    }
    	}
    }


    if (usealpha) {
	    if (firsttime_createwindow_usealpha) {
	    	printf("DISKO: Pixelformat %s is used for windows with alphachannel.\n", getMMSFBPixelFormatString(pixelformat).c_str());
	    	firsttime_createwindow_usealpha = false;
	    }
    }
    else
	    if (firsttime_createwindow_noalpha) {
	    	printf("DISKO: Pixelformat %s is used for windows with no alphachannel.\n", getMMSFBPixelFormatString(pixelformat).c_str());
	    	firsttime_createwindow_noalpha = false;
	    }


#ifdef USE_DFB_WINMAN

    DFBResult               dfbres;
    IDirectFBWindow         *dfbwindow;
    DFBWindowDescription    window_desc;

    /* create window description */
    window_desc.flags = (DFBWindowDescriptionFlags)(DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_PIXELFORMAT | DWDESC_CAPS);
    window_desc.posx = x;
    window_desc.posy = y;
    window_desc.width = w;
    window_desc.height = h;
    window_desc.pixelformat = getDFBPixelFormatFromString(pixelformat);

    /* set caps - differs between alpha and non-alpha pixelformats */
    if (!isAlphaPixelFormat(pixelformat)) {
    	if (backbuffer)
    		window_desc.caps = (DFBWindowCapabilities)(DWCAPS_DOUBLEBUFFER);
    	else
    		window_desc.caps = (DFBWindowCapabilities)0;
    }
    else {
    	if (backbuffer)
    		window_desc.caps = (DFBWindowCapabilities)(DWCAPS_DOUBLEBUFFER | DWCAPS_ALPHACHANNEL);
    	else
    		window_desc.caps = (DFBWindowCapabilities)(DWCAPS_ALPHACHANNEL);
    }

    /* create the window */
    if ((dfbres=this->dfblayer->CreateWindow(this->dfblayer, &window_desc, &dfbwindow)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBDisplayLayer::CreateWindow(" + iToStr(w) + "x" + iToStr(h) + "," + getDFBPixelFormatString(window_desc.pixelformat) + ") failed");
        return false;
    }

    /* create a new window instance */
    *window = new MMSFBWindow(dfbwindow, window_desc.posx, window_desc.posy);
    if (!*window) {
        dfbwindow->Release(dfbwindow);
        MMSFB_SetError(0, "cannot create new instance of MMSFBWindow");
        return false;
    }

#endif

#ifdef USE_MMSFB_WINMAN

    // create a window surface
    MMSFBSurface *surface;
	if (!mmsfb->createSurface(&surface, w, h, pixelformat, backbuffer, (this->config.buffermode == MMSFB_BM_BACKSYSTEM)))
		return false;

    // create a new window instance
    *window = new MMSFBWindow(surface, x, y);
    if (!*window) {
        delete surface;
        MMSFB_SetError(0, "cannot create new instance of MMSFBWindow");
        return false;
    }

    // that is a window surface
    surface->setWinSurface();

    // inform the window manager
    mmsfbwindowmanager->addWindow(*window);

#endif

    return true;
}




