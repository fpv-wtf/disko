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

#ifndef MMSFB_H_
#define MMSFB_H_

#include "mmstools/mmstypes.h"
#include "mmstools/mmslogger.h"

#include "mmsgui/fb/mmsfbdev.h"
#include "mmsgui/fb/mmsfbdevmatrox.h"
#include "mmsgui/fb/mmsfbdevdavinci.h"
#include "mmsgui/fb/mmsfbdevomap.h"
#include "mmsgui/fb/mmsfblayer.h"
#include "mmsgui/fb/mmsfbwindowmanager.h"
#include "mmsgui/fb/mmsfbfont.h"

#define MMSFBLAYER_MAXNUM 32

//! The lowest layer to the backends like DFB, X11(XSHM/XVSHM) or FBDEV.
/*!
\author Jens Schneider
*/
class MMSFB {
    private:
        int             argc;       /* commandline arguments */
        char            **argv;

    	//! is initialized?
    	bool initialized;

#ifdef  __HAVE_DIRECTFB__
        // interface to dfb
        IDirectFB       *dfb;
#endif

#ifdef  __HAVE_FBDEV__
        // interface to own FB device
        MMSFBDev		*mmsfbdev;
#endif

#ifdef __HAVE_XLIB__
        Display 		*x_display;
        int				x_screen;
        Window 			x_window;
        GC 				x_gc;
        Visual			*x_visual;
        int				x_depth;
        int 			xv_port;
        MMSMutex		xlock;
        int				display_w;
        int				display_h;
        int				target_window_w;
        int				target_window_h;
        MMSFBFullScreenMode fullscreen;
        bool            resized;
        bool 			resizeWindow();
#endif

        MMSFBLayer 		*layer[MMSFBLAYER_MAXNUM];

        MMSFBBackend	backend;
        MMSFBOutputType	outputtype;
        MMSFBRectangle  x11_win_rect;

    public:
        MMSFB();
        virtual ~MMSFB();

        bool init(int argc, char **argv, MMSFBBackend backend, MMSFBOutputType outputtype, MMSFBRectangle x11_win_rect,
        		  bool extendedaccel, MMSFBFullScreenMode fullscreen, MMSFBPointerMode pointer,
				  string appl_name = "Disko Application", string appl_icon_name = "Disko Application", bool hidden=false);
        bool release();
        bool isInitialized();

        MMSFBBackend getBackend();

        bool getLayer(int id, MMSFBLayer **layer);

        void *getX11Window();
        void *getX11Display();
        bool refresh();

        bool createSurface(MMSFBSurface **surface, int w, int h, MMSFBSurfacePixelFormat pixelformat, int backbuffer = 0, bool systemonly = false);

#ifdef  __HAVE_DIRECTFB__
        bool createImageProvider(IDirectFBImageProvider **provider, string filename);
#endif
        bool createFont(MMSFBFont **font, string filename, int width = 0, int height = 0);

    friend class MMSFBLayer;
    friend class MMSFBSurface;
    friend class MMSFBFont;
    friend class MMSInputX11Handler;
    friend class MMSInputLISHandler;
};

/* access to global mmsfb */
extern MMSFB *mmsfb;

#endif /*MMSFB_H_*/
