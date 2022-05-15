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

#include "mmsgui/mmswindow.h"
#include "mmsgui/mmschildwindow.h"
#include "mmsgui/mmswidgets.h"
#include "mmsgui/mmsborder.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


/* static variables */
IMMSWindowManager 	*MMSWindow::windowmanager = NULL;

MMSImageManager     *MMSWindow::im1 = NULL;
MMSFBLayer     		*MMSWindow::im1_layer = NULL;
MMSImageManager     *MMSWindow::im2 = NULL;
MMSFBLayer     		*MMSWindow::im2_layer = NULL;
MMSFontManager     	*MMSWindow::fm = NULL;

MMSFBWindow *MMSWindow::fullscreen_root_window			= NULL;
int			MMSWindow::fullscreen_root_window_use_count = 0;
MMSFBWindow *MMSWindow::fullscreen_main_window			= NULL;
int			MMSWindow::fullscreen_main_window_use_count	= 0;


MMSWindow::MMSWindow() {

    this->TID = 0;
    this->Lock_cnt = 0;

    this->baseWindowClass = NULL;
    this->windowClass = NULL;
    this->initialized = false;
    this->parent = NULL;
    this->toplevel_parent = NULL;
//this->im = NULL;
//    this->fm = NULL;
    this->window = NULL;
    this->layer = NULL;
    this->surface = NULL;
	this->flags = MMSW_NONE;
    this->bgimage = NULL;
    for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++)
        this->borderimages[i] = NULL;
    bordergeomset = false;

    this->draw_setgeom = true;

    this->dxpix = 0;
    this->dypix = 0;
    this->geom.x = 0;
    this->geom.y = 0;
    this->geom.w = 0;
    this->geom.h = 0;

    this->focusedChildWin = 0;

    this->upArrowWidget    = NULL;
    this->downArrowWidget  = NULL;
    this->leftArrowWidget  = NULL;
    this->rightArrowWidget = NULL;

    this->initialArrowsDrawn = false;

    /* initialize the callbacks */
    onBeforeShow  = new sigc::signal<bool, MMSWindow*>::accumulated<bool_accumulator>;
    onAfterShow   = new sigc::signal<void, MMSWindow*, bool>;
    onBeforeHide  = new sigc::signal<bool, MMSWindow*, bool>::accumulated<bool_accumulator>;
    onHide        = new sigc::signal<void, MMSWindow*, bool>;
    onHandleInput = new sigc::signal<bool, MMSWindow*, MMSInputEvent*>::accumulated<neg_bool_accumulator>;
}

MMSWindow::~MMSWindow() {
	/* wait until show/hide actions are finished */
	while(this->action->getAction() != MMSWACTION_NONE)
		msleep(100);

	// hide the window if shown
	instantHide();

	// delete the callbacks
    if (onBeforeShow) delete onBeforeShow;
    if (onAfterShow) delete onAfterShow;
    if (onBeforeHide) delete onBeforeHide;
    if (onHide) delete onHide;
    if (onHandleInput) delete onHandleInput;

	if (this->type != MMSWINDOWTYPE_CHILDWINDOW) {
		// remove normal window
	    if (this->windowmanager)
	        this->windowmanager->removeWindow(this);
	}
	else {
		// remove child window from parent
		if (this->parent)
			this->parent->removeChildWindow(this);
	}

    // delete children
	// i have to delete only the first widget because all others are children from it and will be implicitly deleted
	if (children.size()>0)
		delete children.at(0);

    // delete childwins
    for (unsigned int i = 0; i < childwins.size(); i++)
        delete childwins.at(i).window;

    // delete the rest :)
    delete this->action;
//delete this->im;
//delete this->fm;

    if (!((this->flags & MMSW_VIDEO)&&(!(this->flags & MMSW_USEGRAPHICSLAYER)))) {
		// surface is NOT the video layer surface
    	// so delete the window/surface memory
		if (this->window) {
			// delete the mmsfbwindow/surface
			bool os;
			getOwnSurface(os);
			if (os) {
				// own surface, so delete complete window
				delete this->window;
			}
			else {
				// delete sub-surface and decrease use counter
				if (this->surface)
					delete this->surface;
				if (this->type == MMSWINDOWTYPE_ROOTWINDOW) {
					if (this->fullscreen_root_window_use_count > 0)
						this->fullscreen_root_window_use_count--;
					if (this->fullscreen_root_window_use_count == 0)
						if (this->fullscreen_root_window) {
							// delete the fullscreen window for root window type because not used anymore
							delete this->fullscreen_root_window;
							this->fullscreen_root_window = NULL;
						}
				}
				if (this->type == MMSWINDOWTYPE_MAINWINDOW) {
					if (this->fullscreen_main_window_use_count > 0)
						this->fullscreen_main_window_use_count--;
					if (this->fullscreen_main_window_use_count == 0)
						if (this->fullscreen_main_window) {
							// delete the fullscreen window for main window type because not used anymore
							delete this->fullscreen_main_window;
							this->fullscreen_main_window = NULL;
						}
				}
			}
		}
		else {
			// delete surface (e.g. child window surface)
			if (this->surface)
				delete this->surface;
		}
    }
}

MMSWINDOWTYPE MMSWindow::getType() {
	return this->type;
}

bool MMSWindow::create(string dx, string dy, string w, string h, MMSALIGNMENT alignment, MMSWINDOW_FLAGS flags,
		               bool *own_surface) {
    /* save flags */
    this->flags = flags;

    /* set theme values */
    if (dx != "")
        setDx(dx, false, false);
    if (dy != "")
        setDy(dy, false, false);
    if (w != "")
        setWidth(w, false, false);
    if (h != "")
        setHeight(h, false, false);
    if (alignment != MMSALIGNMENT_NOTSET)
        setAlignment(alignment, false, false);
    if (own_surface)
    	setOwnSurface(*own_surface);

    this->action = new MMSWindowAction(this);
    this->firstfocusset = false;
    this->focusedwidget=NULL;

    this->navigateUpWindow    = NULL;
    this->navigateDownWindow  = NULL;
    this->navigateLeftWindow  = NULL;
    this->navigateRightWindow = NULL;

    this->buttonpress_widget = NULL;
    this->buttonpress_childwin = NULL;

    if (!this->parent) {
        /* normal parent window, check the flags and get the right layer */
        if ((mmsfbmanager.getLayerCount()<2)&&(this->flags & MMSW_VIDEO)) {
        	DEBUGMSG("MMSGUI", "use video window on graphics layer");
            this->flags = (MMSWINDOW_FLAGS)(this->flags | MMSW_USEGRAPHICSLAYER);
        }

        if ((this->flags & MMSW_VIDEO)&&(!(this->flags & MMSW_USEGRAPHICSLAYER))) {
        	DEBUGMSG("MMSGUI", "get the video layer");
        	this->layer = mmsfbmanager.getVideoLayer();
        } else {
        	DEBUGMSG("MMSGUI", "get the grapics layer");
        	this->layer = mmsfbmanager.getGraphicsLayer();
        }
    }
    else {
        /* child window, use the flags and the layer from my parent */
    	DEBUGMSG("MMSGUI", "use layer from parent window");
        this->flags = this->parent->flags;
        this->layer = this->parent->layer;

    	// searching the right toplevel parent
        this->toplevel_parent = this->getParent(true);
    }

    DEBUGMSG("MMSGUI", "got flags: ");
    if (this->flags) {
        if (this->flags & MMSW_VIDEO)
        	DEBUGMSG("MMSGUI", " MMSW_VIDEO");
        if(this->flags & MMSW_USEGRAPHICSLAYER)
        	DEBUGMSG("MMSGUI", " MMSW_USEGRAPHICSLAYER");
    }
    else
    	DEBUGMSG("MMSGUI", " MMSW_NONE");

    // create static image manager
    if (!this->im1_layer) {
    	this->im1 = new MMSImageManager(this->layer);
    	this->im1_layer = this->layer;
    	this->im = this->im1;
    }
    else {
        if (this->im1_layer == this->layer) {
        	this->im = this->im1;
        }
        else {
            if (this->im2_layer == this->layer) {
				this->im = this->im2;
            }
            else {
				this->im2 = new MMSImageManager(this->layer);
				this->im2_layer = this->layer;
				this->im = this->im2;
            }
        }
    }

    // create static font manager
    if (!this->fm)
    	this->fm = new MMSFontManager();

    /* set some attributes */
    this->shown=false;
    this->willshow=false;
    this->willhide=false;

    buffered_shown = false;

    /* load background image */
    string path, name;
    if (!getBgImagePath(path)) path = "";
    if (!getBgImageName(name)) name = "";
    this->bgimage = im->getImage(path, name);

    /* load border images */
    if (!getBorderImagePath(path)) path = "";
    for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++) {
        if (!getBorderImageNames((MMSBORDER_IMAGE_NUM)i, name)) name = "";
        this->borderimages[i] = im->getImage(path, name);
    }

    /* resize/create the window */
    if (this->windowmanager)
        resize();

    return true;
}

bool MMSWindow::create(string w, string h, MMSALIGNMENT alignment, MMSWINDOW_FLAGS flags, bool *own_surface) {
    return create("", "", w, h, alignment, flags, own_surface);
}



bool MMSWindow::resize(bool refresh) {
    int wdesc_posx, wdesc_posy, wdesc_width, wdesc_height;
    string dx, dy, width, height;


    DEBUGMSG("MMSGUI", "resize... flags: " + iToStr(this->flags));

	if (this->layer == NULL) {
		DEBUGMSG("MMSGUI", "have no layer... returning");
		return false;
	}

    if (!this->parent) {
        /* normal parent window */

    	/* check if we have a video window and should use the video layer */
        if ((this->flags & MMSW_VIDEO)&&(!(this->flags & MMSW_USEGRAPHICSLAYER))) {
            if (!this->surface) {
            	DEBUGMSG("MMSGUI", "have a video window, use the layer surface");
        		this->layer->getSurface(&this->surface);
        		DEBUGMSG("MMSGUI", "after layer->getSurface() (surface = 0x%x)", this->surface);
                if (this->windowmanager)
                    this->windowmanager->addWindow(this);

                this->geom.x = 0;
                this->geom.y = 0;
                this->layer->getResolution(&this->geom.w, &this->geom.h);
                this->innerGeom = this->geom;
                DEBUGMSG("MMSGUI", "resolution: %d x %d", this->geom.w, this->geom.h);
            }
            else
            	DEBUGMSG("MMSGUI", "cannot resize the layer surface");

            return true;
    	}

        if (!this->windowmanager) {
        	DEBUGMSG("MMSGUI", "have no windowmanager... returning");
            return false;
        }

        /* get the screen width and height */
        this->layer->getResolution(&this->vrect.w, &this->vrect.h);
        DEBUGMSG("MMSGUI", "got screen %dx%d", this->vrect.w, this->vrect.h);

        if (this->flags & MMSW_VIDEO) {
            /* for video windows use full screen */
            this->vrect.x = 0;
            this->vrect.y = 0;
        }
        else
            /* other windows uses visible rectangle settings from windowmanager */
            this->vrect = this->windowmanager->getVRect();

        DEBUGMSG("MMSGUI", "use screen area %d, %d, %d, %d", this->vrect.x, this->vrect.x, this->vrect.w, this->vrect.h);
    }
    else {
        /* child window */

        /* get the parent width and height */
        this->vrect.x = (this->parent->geom.w - this->parent->innerGeom.w) / 2;
        this->vrect.y = (this->parent->geom.h - this->parent->innerGeom.h) / 2;
        this->vrect.w = this->parent->innerGeom.w;
        this->vrect.h = this->parent->innerGeom.h;
//        logger.writeLog("got inner size from parent " + iToStr(vrect.w) + "x" + iToStr(vrect.h));
    }


    /* calculate the window position */
    /* first try with xpos */
    if (!getDx(dx)) dx = "";
    if (getPixelFromSizeHint(&wdesc_posx, dx, vrect.w, 0) == false) {
        if (getPixelFromSizeHint(&wdesc_posx, dx, 10000, 0) == false) {
        	DEBUGMSG("MMSGUI", "window dx %s is wrong, using 0px", dx.c_str());
            myWindowClass.setDx("0px");
            wdesc_posx = 0;
        }
    }
    /* ypos */
    if (!getDy(dy)) dy = "";
    if (getPixelFromSizeHint(&wdesc_posy, dy, vrect.h, wdesc_posx) == false) {
        if (getPixelFromSizeHint(&wdesc_posy, dy, 10000, wdesc_posx) == false) {
        	DEBUGMSG("MMSGUI", "window dy %s is wrong, using 0px", dy.c_str());
            myWindowClass.setDy("0px");
            wdesc_posy = 0;
        }
    }
    /* second try with xpos (because of "<factor>$") */
    if (!getDx(dx)) dx = "";
    if (getPixelFromSizeHint(&wdesc_posx, dx, vrect.w, wdesc_posy) == false) {
        if (getPixelFromSizeHint(&wdesc_posx, dx, 10000, wdesc_posy) == false) {
        	DEBUGMSG("MMSGUI", "window dx %s is wrong, using 0px", dx.c_str());
            myWindowClass.setDx("0px");
            wdesc_posx = 0;
        }
    }

    /* save the real dx / dy */
    this->dxpix = wdesc_posx;
    this->dypix = wdesc_posy;

    DEBUGMSG("MMSGUI", "dx: %d, dy: %d", this->dxpix, this->dypix);

    /* calculate the window size */
    if (!getWidth(width)) width = "";
    if (getPixelFromSizeHint(&wdesc_width, width, vrect.w, 0) == false) {
        if (getPixelFromSizeHint(&wdesc_width, width, 10000, 0) == false) {
        	DEBUGMSG("MMSGUI", "window width %s is wrong, using %d px", width.c_str(), vrect.w);
            myWindowClass.setWidth(iToStr(vrect.w) + "px");
            wdesc_width = vrect.w;
        }
    }
    if (!getHeight(height)) height = "";
    if (getPixelFromSizeHint(&wdesc_height, height, vrect.h, 0) == false) {
        if (getPixelFromSizeHint(&wdesc_height, height, 10000, 0) == false) {
        	DEBUGMSG("MMSGUI", "window height %s is wrong, using %d px", height.c_str(), vrect.h);
            myWindowClass.setHeight(iToStr(vrect.h) + "px");
            wdesc_height = vrect.h;
        }
    }

    DEBUGMSG("MMSGUI", "window resolution: %d x %d", wdesc_width, wdesc_height);

    if ((wdesc_width == 0)&&(wdesc_height == 0)) {
        /* bad values */
    	DEBUGMSG("MMSGUI", "window width " + width + " is wrong, using " + iToStr(vrect.w) + "px");
        myWindowClass.setWidth(iToStr(vrect.w) + "px");
        wdesc_width = vrect.w;
        DEBUGMSG("MMSGUI", "window height " + height + " is wrong, using " + iToStr(vrect.h) + "px");
        myWindowClass.setHeight(iToStr(vrect.h) + "px");
        wdesc_height = vrect.h;
    }
    else {
        if (wdesc_width == 0) {
            /* it seems that width should be a factor of height */
            getPixelFromSizeHint(&wdesc_width, width, vrect.w, wdesc_height);
        }
        else {
            /* it seems that height should be a factor of width */
            getPixelFromSizeHint(&wdesc_height, height, vrect.h, wdesc_width);
        }
    }

    /* adjust a little bit */
    unsigned int margin;
    if (!getMargin(margin))
    	margin = 0;
    wdesc_posx+= vrect.x;
    wdesc_posy+= vrect.y;
    wdesc_width-= margin*2;
    wdesc_height-= margin*2;

    /* work with alignment */
    MMSALIGNMENT alignment;
    if (!getAlignment(alignment)) alignment = MMSALIGNMENT_CENTER;
    switch (alignment) {
        case MMSALIGNMENT_CENTER:
            wdesc_posx+= (vrect.w - wdesc_width) / 2;
            wdesc_posy+= (vrect.h - wdesc_height) / 2;
            break;
        case MMSALIGNMENT_LEFT:
            wdesc_posy+= (vrect.h - wdesc_height) / 2;
            break;
        case MMSALIGNMENT_RIGHT:
            wdesc_posx+= (vrect.w - wdesc_width);
            wdesc_posy+= (vrect.h - wdesc_height) / 2;
            break;
        case MMSALIGNMENT_TOP_CENTER:
            wdesc_posx+= (vrect.w - wdesc_width) / 2;
            break;
        case MMSALIGNMENT_TOP_LEFT:
            break;
        case MMSALIGNMENT_TOP_RIGHT:
            wdesc_posx+= (vrect.w - wdesc_width);
            break;
        case MMSALIGNMENT_BOTTOM_CENTER:
            wdesc_posx+= (vrect.w - wdesc_width) / 2;
            wdesc_posy+= (vrect.h - wdesc_height);
            break;
        case MMSALIGNMENT_BOTTOM_LEFT:
            wdesc_posy+= (vrect.h - wdesc_height);
            break;
        case MMSALIGNMENT_BOTTOM_RIGHT:
            wdesc_posx+= (vrect.w - wdesc_width);
            wdesc_posy+= (vrect.h - wdesc_height);
            break;
        default:
            break;
    }

    // use even pos/size because of pixelformats like YV12
    wdesc_posx   &= 0xfffffffe;
    wdesc_posy   &= 0xfffffffe;
    wdesc_width  &= 0xfffffffe;
    wdesc_height &= 0xfffffffe;

    int oldx = this->geom.x;
    int oldy = this->geom.y;
    this->geom.x = wdesc_posx;
    this->geom.y = wdesc_posy;
    int oldw = this->geom.w;
    int oldh = this->geom.h;
    this->geom.w = wdesc_width;
    this->geom.h = wdesc_height;
    unsigned int borderMargin;
    if (!getBorderMargin(borderMargin))
    	borderMargin = 0;
    unsigned int borderThickness;
    if (!getBorderThickness(borderThickness))
    	borderThickness = 0;
    unsigned int dz = borderMargin + borderThickness;
    innerGeom.x = dz;
    innerGeom.y = dz;
    innerGeom.w = this->geom.w - 2*dz;
    innerGeom.h = this->geom.h - 2*dz;


    if (!this->parent) {
        // normal parent window
        if (!this->window) {
            // create window

        	// own surface?
			bool os;
			getOwnSurface(os);

			if (!(this->flags & MMSW_VIDEO)) {
                // no video window, use alpha
    			if ((os) || ((this->type == MMSWINDOWTYPE_ROOTWINDOW) && (!this->fullscreen_root_window))
    					 || ((this->type == MMSWINDOWTYPE_MAINWINDOW) && (!this->fullscreen_main_window))) {

    				if ((!os) && ((this->type == MMSWINDOWTYPE_ROOTWINDOW) || (this->type == MMSWINDOWTYPE_MAINWINDOW))) {
    					// create full screen window
    					wdesc_posx = 0;
    					wdesc_posy = 0;
    			        this->layer->getResolution(&wdesc_width, &wdesc_height);
    				}

    				// window should have own surface
                	DEBUGMSG("MMSGUI", "creating window (" + iToStr(wdesc_posx) + ","
                                                        + iToStr(wdesc_posy) + ","
                                                        + iToStr(wdesc_width) + ","
                                                        + iToStr(wdesc_height)
                                                        + "), alphachannel requested");
                    this->layer->createWindow(&(this->window),
                                              wdesc_posx, wdesc_posy, wdesc_width, wdesc_height,
                                              MMSFB_PF_NONE, true);
                    DEBUGMSG("MMSGUI", "window created (0x%x)", this->window);

                    // window should not be visible at this time
                    this->window->setOpacity(0);
    			}

				if (!os) {
					if (this->type == MMSWINDOWTYPE_ROOTWINDOW) {
						if (!this->fullscreen_root_window)
							this->fullscreen_root_window = this->window;
						else
							this->window = this->fullscreen_root_window;
						this->fullscreen_root_window_use_count++;
					}
					else
					if (this->type == MMSWINDOWTYPE_MAINWINDOW) {
						if (!this->fullscreen_main_window)
							this->fullscreen_main_window = this->window;
						else
							this->window = this->fullscreen_main_window;
						this->fullscreen_main_window_use_count++;
					}
				}
            }
            else {
                // video window, do not use alpha
            	DEBUGMSG("MMSGUI", "creating video window (" + iToStr(wdesc_posx) + ","
                                                          + iToStr(wdesc_posy) + ","
                                                          + iToStr(wdesc_width) + ","
                                                          + iToStr(wdesc_height)
                                                          + "), no alphachannel");
                this->layer->createWindow(&(this->window),
                                          wdesc_posx, wdesc_posy, wdesc_width, wdesc_height,
                                          MMSFB_PF_NONE, false);
                DEBUGMSG("MMSGUI", "video window created (0x%x)", this->window);

                // window should not be visible at this time
                this->window->setOpacity(0);

                // video windows should have own surfaces
                setOwnSurface(true);
            }

            // get window surface
        	this->window->getSurface(&(this->surface));

			if ((this->window == this->fullscreen_root_window) || (this->window == this->fullscreen_main_window)) {
				// get subsurface
                DEBUGMSG("MMSGUI", "window has no own surface, get subsurface of shared full screen window");
				this->surface = this->surface->getSubSurface(&this->geom);
			}


            // normal window
			DEBUGMSG("MMSGUI", "setting blitting flags for window");
			this->surface->setBlittingFlags(MMSFB_BLIT_BLEND_ALPHACHANNEL);

			/* set the window to bottom */
//            this->window->lowerToBottom();

			// add window to managers list
			if (this->windowmanager) {
				DEBUGMSG("MMSGUI", "adding window to window manager");
				this->windowmanager->addWindow(this);
			}
        }
        else {
            // change the window (new size/pos)
			bool os;
			getOwnSurface(os);
			if (os) {
				// own surface
				int px,py;
				this->window->getPosition(&px, &py);
				if ((this->geom.x != px)||(this->geom.y != py)) {
					DEBUGMSG("MMSGUI", "repositioning window (" + iToStr(this->geom.x) + "," + iToStr(this->geom.y) + ")");
					this->window->moveTo(this->geom.x, this->geom.y);
				}
				int w,h;
				this->window->getSize(&w, &h);
				if ((this->geom.w != w)||(this->geom.h != h)) {
					DEBUGMSG("MMSGUI", "resizing window (" + iToStr(this->geom.w) + "x" + iToStr(this->geom.h) + ")");
					this->window->resize(this->geom.w, this->geom.h);
				}
			}
			else {
				// working with subsurface
            	DEBUGMSG("MMSGUI", "re-positioning/-sizing window subsurface (" + iToStr(this->geom.x) + "," + iToStr(this->geom.y) + ","
                                                                        + iToStr(this->geom.w) + "," + iToStr(this->geom.h) + ")");
				this->surface->setSubSurface(&this->geom);
			}
        }
    }
    else {
        /* child window */
        if (!this->surface) {
            /* create surface for child window */
            /* get layers pixelformat */
        	MMSFBSurfacePixelFormat pixelformat;
            this->layer->getPixelFormat(&pixelformat);

            bool os;
            getOwnSurface(os);
        	if (os) {
        		DEBUGMSG("MMSGUI", "creating surface for child window (" + iToStr(wdesc_posx) + ","
	                                                                  + iToStr(wdesc_posy) + ","
	                                                                  + iToStr(wdesc_width) + ","
	                                                                  + iToStr(wdesc_height)
	                                                                  + ") with pixelformat " + getMMSFBPixelFormatString(pixelformat)
	                                                                  + " (use alpha)");

	            this->layer->createSurface(&(this->surface),
	                                      wdesc_width, wdesc_height, MMSFB_PF_NONE, 0);
	        }
	        else {
	        	DEBUGMSG("MMSGUI", "creating sub surface for child window (" + iToStr(wdesc_posx) + ","
	                                                                      + iToStr(wdesc_posy) + ","
	                                                                      + iToStr(wdesc_width) + ","
	                                                                      + iToStr(wdesc_height)
	                                                                      + ") with pixelformat " + getMMSFBPixelFormatString(pixelformat)
	                                                                      + " (use alpha)");

	            MMSFBRectangle rect;

	            rect.x = wdesc_posx;
	            rect.y = wdesc_posy;
	            rect.w = wdesc_width;
	            rect.h = wdesc_height;
	            this->surface = this->parent->surface->getSubSurface(&rect);
	        }

            this->surface->setBlittingFlags(MMSFB_BLIT_BLEND_ALPHACHANNEL);

            /* set the window to bottom */
//            this->window->lowerToBottom();

            /* add window to parents childwins list */
//printf("%x->resize(), this->%x->addChildwindow(%x)\n", this, this->parent, this);
            this->parent->addChildWindow(this);
        }
        else {
            /* change the surface (new size/pos) */
            if   ((this->geom.x != oldx)||(this->geom.y != oldy)
                ||(this->geom.w != oldw)||(this->geom.h != oldh)) {
            	DEBUGMSG("MMSGUI", "re-positioning/-sizing child window (" + iToStr(this->geom.x) + "," + iToStr(this->geom.y) + ","
                                                                        + iToStr(this->geom.w) + "," + iToStr(this->geom.h) + ")");
                this->parent->setChildWindowRegion(this, refresh);
            }
        }
    }

    DEBUGMSG("MMSGUI", "resizing done");

    return true;
}


void MMSWindow::lock() {
	// which window is to lock?
	MMSWindow *tolock = this;
	if (this->toplevel_parent)
		tolock = this->toplevel_parent;
	else
	if (this->parent)
		tolock = this->parent;

	if (tolock->surface)
		tolock->surface->lock();

/*    if (tolock->Lock.trylock() == 0) {
        // I have got the lock the first time
    	tolock->TID = pthread_self();
    	tolock->Lock_cnt = 1;
    }
    else {
        if ((tolock->TID == pthread_self())&&(tolock->Lock_cnt > 0)) {
            // I am the thread which has already locked this window
        	tolock->Lock_cnt++;
        }
        else {
            // another thread has already locked this window, waiting for...
        	tolock->Lock.lock();
        	tolock->TID = pthread_self();
        	tolock->Lock_cnt = 1;
        }
    }
	*/

/*
    if (this->Lock.trylock() == 0) {
        // I have got the lock the first time
        this->TID = pthread_self();
        this->Lock_cnt = 1;
    }
    else {
        if ((this->TID == pthread_self())&&(this->Lock_cnt > 0)) {
            // I am the thread which has already locked this window
            this->Lock_cnt++;
        }
        else {
            // another thread has already locked this window, waiting for...
            this->Lock.lock();
            this->TID = pthread_self();
            this->Lock_cnt = 1;
        }
    }
    */
}

void MMSWindow::unlock() {
	// which window is to lock?
	MMSWindow *tolock = this;
	if (this->toplevel_parent)
		tolock = this->toplevel_parent;
	else
	if (this->parent)
		tolock = this->parent;

	if (tolock->surface)
		tolock->surface->unlock();

/*	if (tolock->TID != pthread_self())
        return;
    if(tolock->Lock_cnt==0)
    	return;
    tolock->Lock_cnt--;
    if (tolock->Lock_cnt == 0)
    	tolock->Lock.unlock();
*/

/*
	if (this->TID != pthread_self())
        return;
    if(this->Lock_cnt==0)
    	return;
    this->Lock_cnt--;
    if (this->Lock_cnt == 0)
        this->Lock.unlock();
*/
}


string MMSWindow::getName() {
    return this->name;
}

void MMSWindow::setName(string name) {
    this->name = name;
}

MMSWindow* MMSWindow::searchForWindow(string name) {
    MMSWindow *window;

    if (name=="")
        return NULL;

    /* first, my own childwins */
    for (unsigned int i = 0; i < childwins.size(); i++)
        if (childwins.at(i).window->getName() == name)
            return childwins.at(i).window;

    /* second, call search method of my childwins */
    for (unsigned int i = 0; i < childwins.size(); i++)
        if ((window = childwins.at(i).window->searchForWindow(name)))
            return window;

    return NULL;
}

bool MMSWindow::addChildWindow(MMSWindow *childwin) {
    if (childwin->getType()!=MMSWINDOWTYPE_CHILDWINDOW)
        return false;

//printf("add child window %s, %x, %d\n", childwin->name.c_str(), childwin, this->childwins.size());

    /* per default childwins are not focused */
//    childwin->focused = false;

    CHILDWINS cw;
    cw.window = childwin;
    cw.region.x1 = childwin->geom.x;
    cw.region.y1 = childwin->geom.y;
    cw.region.x2 = childwin->geom.x + childwin->geom.w - 1;
    cw.region.y2 = childwin->geom.y + childwin->geom.h - 1;
    cw.opacity = 0;
    cw.oldopacity = 0;
    cw.focusedWidget = 0;

//printf("%x->childwins.push_back %x\n", this, cw.window);
//printf("  %s->childwins.push_back %s\n", this->name.c_str(), cw.window->name.c_str());

    lock();
    this->childwins.push_back(cw);
    unlock();

//printf("%x->childwins.push_back %x   <<\n", this, cw.window);
//printf("  %s->childwins.push_back %s   <<<\n", this->name.c_str(), cw.window->name.c_str());

    return true;
}



bool MMSWindow::removeChildWindow(MMSWindow *childwin) {
    if (childwin->getType()!=MMSWINDOWTYPE_CHILDWINDOW)
        return false;

    for (unsigned int i = 0; i < this->childwins.size(); i++)
    	if (childwins.at(i).window == childwin) {
            this->childwins.erase(this->childwins.begin()+i);
    		return true;
    	}

    return false;
}


bool MMSWindow::setChildWindowOpacity(MMSWindow *childwin, unsigned char opacity) {
    if (childwin->getType()!=MMSWINDOWTYPE_CHILDWINDOW)
        return false;

    for (unsigned int i = 0; i < this->childwins.size(); i++)
        if (this->childwins.at(i).window == childwin) {

//PUP            flipLock.lock();
        	lock();

            this->childwins.at(i).oldopacity = this->childwins.at(i).opacity;
            this->childwins.at(i).opacity = opacity;

            flipWindow(childwin, NULL, MMSFB_FLIP_NONE, false, true);

//PUP			flipLock.unlock();
            unlock();


            return true;
        }

    return false;
}

bool MMSWindow::setChildWindowRegion(MMSWindow *childwin, bool refresh) {

    if (childwin->getType()!=MMSWINDOWTYPE_CHILDWINDOW)
        return false;

    for (unsigned int i = 0; i < this->childwins.size(); i++)
        if (this->childwins.at(i).window == childwin) {
            /* get old region */
            MMSFBRegion *currregion = &this->childwins.at(i).region;
            MMSFBRegion oldregion = *currregion;

            if   ((oldregion.x1 != childwin->geom.x)
                ||(oldregion.y1 != childwin->geom.y)
                ||(oldregion.x2 - oldregion.x1 + 1 != childwin->geom.w)
                ||(oldregion.y2 - oldregion.y1 + 1 != childwin->geom.h)) {

                /* calc new pos */
                currregion->x1 = childwin->geom.x;
                currregion->y1 = childwin->geom.y;
                currregion->x2 = childwin->geom.x + childwin->geom.w - 1;
                currregion->y2 = childwin->geom.y + childwin->geom.h - 1;


                bool os;
                childwin->getOwnSurface(os);
            	if (os) {
            		if   ((oldregion.x2 - oldregion.x1 + 1 != childwin->geom.w)
	                    ||(oldregion.y2 - oldregion.y1 + 1 != childwin->geom.h)) {

	                    /* resize surface */
	                    childwin->surface->resize(childwin->geom.w, childwin->geom.h);
	                }
            	}
            	else {
            		/* working with sub surface */
					childwin->surface->setSubSurface(&(childwin->geom));
            	}

                /* call resize recursive for new regions of my child windows */
                for (unsigned int j = 0; j < childwin->childwins.size(); j++)
                    childwin->childwins.at(j).window->resize(false);

                /* recursive calls should stop here */
                if (!refresh)
                    return true;

                /* draw at new pos */
                flipWindow(childwin, NULL, MMSFB_FLIP_NONE, false, false);

                /* redraw the old rects */
                if (oldregion.y1 < currregion->y1) {
                    /* redraw above */
                    MMSFBRegion region;
                    region = oldregion;
                    if (region.y2 >= currregion->y1)
                        region.y2 = currregion->y1 - 1;

                    region.x1-= currregion->x1;
                    region.x2-= currregion->x1;
                    region.y1-=currregion->y1;
                    region.y2-=currregion->y1;
                    flipWindow(childwin, &region, MMSFB_FLIP_NONE, false, false);
                }
                if (oldregion.y2 > currregion->y2) {
                    /* redraw below */
                    MMSFBRegion region;
                    region = oldregion;
                    if (region.y1 <= currregion->y2)
                        region.y1 = currregion->y2 + 1;

                    region.x1-= currregion->x1;
                    region.x2-= currregion->x1;
                    region.y1-=currregion->y1;
                    region.y2-=currregion->y1;
                    flipWindow(childwin, &region, MMSFB_FLIP_NONE, false, false);
                }
                if (oldregion.x1 < currregion->x1) {
                    /* redraw left side */
                    MMSFBRegion region;
                    region = oldregion;
                    if  ((region.y2 >= currregion->y1)
                       &&(region.y1 <= currregion->y2)) {
                        if (region.x2 >= currregion->x1)
                            region.x2 = currregion->x1 - 1;

                        region.y1 = 0;
                        region.y2 = currregion->y2 - currregion->y1;
                        region.x1-=currregion->x1;
                        region.x2-=currregion->x1;
                        flipWindow(childwin, &region, MMSFB_FLIP_NONE, false, false);
                    }
                }
                if (oldregion.x2 > currregion->x2) {
                    /* redraw right side */
                    MMSFBRegion region;
                    region = oldregion;
                    if  ((region.y2 >= currregion->y1)
                       &&(region.y1 <= currregion->y2)) {
                        if (region.x1 <= currregion->x2)
                            region.x1 = currregion->x2 + 1;

                        region.y1 = 0;
                        region.y2 = currregion->y2 - currregion->y1;
                        region.x1-=currregion->x1;
                        region.x2-=currregion->x1;
                        flipWindow(childwin, &region, MMSFB_FLIP_NONE, false, false);
                    }
                }
            }

            return true;
        }

    return false;
}

bool MMSWindow::moveChildWindow(MMSWindow *childwin, int x, int y, bool refresh) {

    if (childwin->getType()!=MMSWINDOWTYPE_CHILDWINDOW)
        return false;

    childwin->geom.x = x;
	childwin->geom.y = y;

	return setChildWindowRegion(childwin, refresh);
}

void MMSWindow::drawChildWindows(MMSFBSurface *dst_surface, MMSFBRegion *region, int offsX, int offsY) {
    MMSFBRegion       pw_region;

    if (region == NULL) {
        /* complete surface */
        pw_region.x1 = 0;
        pw_region.y1 = 0;
        pw_region.x2 = this->geom.w - 1;
        pw_region.y2 = this->geom.h - 1;
    }
    else {
        /* only a region */
        pw_region = *region;
    }

    /* draw all affected child windows */
    for (unsigned int i = 0; i < this->childwins.size(); i++) {
        CHILDWINS *cw = &(this->childwins.at(i));
        MMSFBRegion *myregion = &(cw->region);

        // window pointer set?
        if (!cw->window)
        	continue;

        /* if the window has no opacity then continue */
        if (!cw->opacity)
            continue;

        if (!((myregion->x2 < pw_region.x1)||(myregion->y2 < pw_region.y1)
            ||(myregion->x1 > pw_region.x2)||(myregion->y1 > pw_region.y2))) {

            /* the window is affected */
            /* calc source and destination */
            MMSFBRectangle src_rect;
            int dst_x = pw_region.x1;
            int dst_y = pw_region.y1;

            src_rect.x = pw_region.x1 - myregion->x1;
            if (src_rect.x < 0) {
                dst_x-= src_rect.x;
                src_rect.x = 0;
            }

            src_rect.y = pw_region.y1 - myregion->y1;
            if (src_rect.y < 0) {
                dst_y-= src_rect.y;
                src_rect.y = 0;
            }

            src_rect.w = myregion->x2 - myregion->x1 + 1 - src_rect.x;
            if (myregion->x2 > pw_region.x2)
                src_rect.w-= myregion->x2 - pw_region.x2;

            src_rect.h = myregion->y2 - myregion->y1 + 1 - src_rect.y;
            if (myregion->y2 > pw_region.y2)
                src_rect.h-= myregion->y2 - pw_region.y2;


            bool os;
            cw->window->getOwnSurface(os);
        	if (os) {
                /* set the blitting flags and color */
                if (cw->opacity < 255) {
                    dst_surface->setBlittingFlags((MMSFBBlittingFlags) (MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA));
                    dst_surface->setColor(0, 0, 0, cw->opacity);
                }
                else
                    dst_surface->setBlittingFlags((MMSFBBlittingFlags) MMSFB_BLIT_BLEND_ALPHACHANNEL);

                /* blit window front buffer to destination surface */
        		dst_surface->blit(cw->window->surface, &src_rect, dst_x + offsX, dst_y + offsY);
        	}
        	else {
        		/* no own surface -> direct draw */
				MMSFBRectangle r = cw->window->geom;
				if ((src_rect.w == r.w)||(src_rect.h == r.h)) {
					/* draw all (e.g. border) */
					cw->window->draw(false, &src_rect, false);
				}
				else {
					/* minimal draw */
					cw->window->draw(true, &src_rect, false);
				}
        	}

            /* draw the children of this child */
            MMSFBRegion reg;
            reg.x1 = src_rect.x;
            reg.y1 = src_rect.y;
            reg.x2 = src_rect.x + src_rect.w - 1;
            reg.y2 = src_rect.y + src_rect.h - 1;
            if(cw->window)
            	cw->window->drawChildWindows(dst_surface, &reg, dst_x + offsX - reg.x1, dst_y + offsY - reg.y1);
        }
    }
}

bool MMSWindow::flipWindow(MMSWindow *win, MMSFBRegion *region, MMSFBFlipFlags flags,
                           bool flipChildSurface, bool locked) {
    MMSFBSurface    *pw_surface;
    MMSFBRegion       pw_region;


    /* stop parallel processing */
    if (!locked)
//PUP        flipLock.lock();
    	lock();


    if (!win) win = this;
    if (win->getType()!=MMSWINDOWTYPE_CHILDWINDOW) {
        /* normal parent window */
        pw_surface = win->surface;
        if (region == NULL) {
            /* complete surface */
        	pw_region.x1 = 0;
            pw_region.y1 = 0;
            pw_region.x2 = win->geom.w - 1;
            pw_region.y2 = win->geom.h - 1;
        }
        else {
            /* only a region */
            pw_region = *region;
        }
    }
    else {
        /* child window */
        int z = -1;
        for (unsigned int i = 0; i < this->childwins.size(); i++)
            if (this->childwins.at(i).window == win) {
                /* child window found, flip it */
                if (flipChildSurface)
                    win->surface->flip(region);

                /* return if parent window is not shown */
                if ((win->parent->isShown()==false)&&(win->parent->willshow==false)) {
                    /* unlock */
                    if (!locked)
//PUP                        flipLock.unlock();
                    	unlock();
                    return true;
                }

                /* return if opacity is zero */
                if ((this->childwins.at(i).opacity==0)&&(this->childwins.at(i).oldopacity==0)) {
                    /* unlock */
                    if (!locked)
//PUP                        flipLock.unlock();
                    	unlock();

                    return true;
                }

                this->childwins.at(i).oldopacity = this->childwins.at(i).opacity;

                /* get parents surface and break loop */
                pw_surface = win->parent->surface;
                z = i;
                break;
            }

        /* window found? */
        if (z < 0) {
            /* not found */
            if (!locked)
//PUP                flipLock.unlock();
            	unlock();

            return false;
        }

        /* calculate the affected region on the parent surface */
        MMSFBRegion *myregion = &(this->childwins.at(z).region);
        if (region == NULL) {
            /* complete surface */
            pw_region = *myregion;
        }
        else {
            /* only a region */
            pw_region.x1 = myregion->x1 + region->x1;
            pw_region.y1 = myregion->y1 + region->y1;
            pw_region.x2 = myregion->x1 + region->x2;
            pw_region.y2 = myregion->y1 + region->y2;
        }

        /* redraw the region within parent window */
        MMSFBRectangle rect;
        rect.x = pw_region.x1;
        rect.y = pw_region.y1;
        rect.w = pw_region.x2 - pw_region.x1 + 1;
        rect.h = pw_region.y2 - pw_region.y1 + 1;


        if (this->parent == NULL) {
            /* i am the root */
        	this->draw(true, &rect);
        } else {
            /* i am also a child, call recursive */
            bool ret = this->parent->flipWindow(win->parent, &pw_region, flags, false, false);

            /* unlock */
            if (!locked)
//PUP                flipLock.unlock();
            	unlock();

            return ret;
        }

    }

    /* lock */
//PUP    pw_surface->lock();
    lock();


	/* draw all affected child windows */
    drawChildWindows(pw_surface, &pw_region);

	/* do the flip */
    pw_surface->flip(&pw_region);

    /* unlock */
//PUP    pw_surface->unlock();
    unlock();

    /* unlock */
    if (!locked)
//PUP        flipLock.unlock();
    	unlock();

    return false;
}


void MMSWindow::removeFocusFromChildWindow() {
    /* check something */
    if (!this->parent) return;
    if ((this->parent->focusedChildWin < 0) || (this->parent->focusedChildWin >= this->parent->childwins.size())) return;
    if (this->parent->childwins.at(this->parent->focusedChildWin).window != this) return;

    /* searching for other childwin to get the focus */
//    for (unsigned int i = 0; i < this->parent->childwins.size(); i++) {
	for (int i = (int)this->parent->childwins.size()-1; i >= 0; i--) {
        if (i == (int)this->parent->focusedChildWin) continue;
        MMSWindow *w = this->parent->childwins.at(i).window;
        if (!w->isShown()) continue;
        if (!w->getNumberOfFocusableWidgets())
            if (!w->getNumberOfFocusableChildWins())
                continue;

        /* okay, i have the focus, remove it */
        this->parent->removeChildWinFocus();

        /* set the new focused childwin */
        this->parent->focusedChildWin = i;
        this->parent->restoreChildWinFocus();
        return;
    }

//TODO: it must be possible, that there is no focused childwin (-1)
}


void MMSWindow::loadArrowWidgets() {
    /* searching root parent */
    MMSWindow *tmp = this;
    while (tmp->parent) tmp = tmp->parent;

	bool  	b;
	string 	s;

    /* connect arrow widgets */
    if (!this->upArrowWidget)
    	if (getUpArrow(s))
    		if (s != "")
		        if ((this->upArrowWidget = tmp->searchForWidget(s))) {
		            if (!this->upArrowWidget->getSelectable(b))
		                this->upArrowWidget = NULL;
	                else
		                if (!b)
		                    this->upArrowWidget = NULL;
		        }

    if (!this->downArrowWidget)
    	if (getDownArrow(s))
    		if (s != "")
		        if ((this->downArrowWidget = tmp->searchForWidget(s))) {
		            if (!this->downArrowWidget->getSelectable(b))
		                this->downArrowWidget = NULL;
	                else
		                if (!b)
		                    this->downArrowWidget = NULL;
		        }

    if (!this->leftArrowWidget)
    	if (getLeftArrow(s))
    		if (s != "")
		        if ((this->leftArrowWidget = tmp->searchForWidget(s))) {
		            if (!this->leftArrowWidget->getSelectable(b))
		                this->leftArrowWidget = NULL;
	                else
		                if (!b)
		                    this->leftArrowWidget = NULL;
		        }

    if (!this->rightArrowWidget)
    	if (getRightArrow(s))
    		if (s != "")
		        if ((this->rightArrowWidget = tmp->searchForWidget(s))) {
		            if (!this->rightArrowWidget->getSelectable(b))
		                this->rightArrowWidget = NULL;
	                else
		                if (!b)
		                    this->rightArrowWidget = NULL;
		        }
}


void MMSWindow::getArrowWidgetStatus(ARROW_WIDGET_STATUS *setarrows) {

    if (this->focusedwidget) {
    	/* for my widgets */
        setarrows->up=(this->focusedwidget->canNavigateUp());
        setarrows->down=(this->focusedwidget->canNavigateDown());
        setarrows->left=(this->focusedwidget->canNavigateLeft());
        setarrows->right=(this->focusedwidget->canNavigateRight());

        /* check my window navigation */
        if (!setarrows->up)
            setarrows->up=(getNavigateUpWindow());
        if (!setarrows->down)
            setarrows->down=(getNavigateDownWindow());
        if (!setarrows->left)
            setarrows->left=(getNavigateLeftWindow());
        if (!setarrows->right)
            setarrows->right=(getNavigateRightWindow());
    }
    else {
        /* for my focused child window */
        if (!childwins.empty()) {
            MMSWindow *fWin = childwins.at(this->focusedChildWin).window;

            /* get all the states (my own and all children) */
            fWin->getArrowWidgetStatus(setarrows);

            /* check my window navigation */
            if (!setarrows->up)
                setarrows->up=(getNavigateUpWindow());
            if (!setarrows->down)
                setarrows->down=(getNavigateDownWindow());
            if (!setarrows->left)
                setarrows->left=(getNavigateLeftWindow());
            if (!setarrows->right)
                setarrows->right=(getNavigateRightWindow());
        }
    }
}

void MMSWindow::switchArrowWidgets() {
    preCalcNaviLock.lock();

    /* connect arrow widgets */
    loadArrowWidgets();

    /* get the new state of the arrows */
    ARROW_WIDGET_STATUS setarrows;
    memset(&setarrows, 0, sizeof(ARROW_WIDGET_STATUS));

    /* get all the states (my own and all children) */
    getArrowWidgetStatus(&setarrows);

    /* switch arrow widgets */
    if (this->upArrowWidget) {
        if (setarrows.up)
            this->upArrowWidget->setSelected(true);
        else
            this->upArrowWidget->setSelected(false);
    }

    if (this->downArrowWidget) {
        if (setarrows.down)
            this->downArrowWidget->setSelected(true);
        else
            this->downArrowWidget->setSelected(false);
    }

    if (this->leftArrowWidget) {
        if (setarrows.left)
            this->leftArrowWidget->setSelected(true);
        else
            this->leftArrowWidget->setSelected(false);
    }

    if (this->rightArrowWidget) {
        if (setarrows.right)
            this->rightArrowWidget->setSelected(true);
        else
            this->rightArrowWidget->setSelected(false);
    }

    /* recursive to my parents */
    if (this->parent)
        this->parent->switchArrowWidgets();

    preCalcNaviLock.unlock();
}


bool MMSWindow::flip(void) {
    if (getType()==MMSWINDOWTYPE_CHILDWINDOW)
    	this->parent->flipWindow(this);
    else
    	this->surface->flip();
    return true;
}

MMSFBLayer *MMSWindow::getLayer() {
    return this->layer;
}

MMSFBSurface *MMSWindow::getSurface() {
    return this->surface;
}

MMSWindow *MMSWindow::getParent(bool toplevel) {
	if (!toplevel)
		return this->parent;
	if (!this->parent)
		return NULL;
	MMSWindow *pw = this->parent->getParent(toplevel);
	if (pw)
		return pw;
	return this->parent;
}


void MMSWindow::recalculateChildren() {
    if(!this->children.empty()) {
        this->children.at(0)->setGeometry(this->innerGeom);
    }

    /* pre-calculate the navigation */
    preCalcNavigation();
}



bool MMSWindow::init() {

    /* get my four windows to which I have to navigate */
    if (this->parent) {
    	string s;
    	if (getNavigateUp(s))
    		this->navigateUpWindow = this->parent->searchForWindow(s);
    	if (getNavigateDown(s))
    		this->navigateDownWindow = this->parent->searchForWindow(s);
    	if (getNavigateRight(s))
    		this->navigateRightWindow = this->parent->searchForWindow(s);
    	if (getNavigateLeft(s))
    		this->navigateLeftWindow = this->parent->searchForWindow(s);
    }

    /* pre-calculate the navigation */
    preCalcNavigation();

    return true;
}



void MMSWindow::draw(bool toRedrawOnly, MMSFBRectangle *rect2update, bool clear) {

    /* lock */
//PUP    this->surface->lock();
	lock();

    if (rect2update) {
        /* use a small rectangle */
        MMSFBRegion clip;
        clip.x1 = rect2update->x;
        clip.y1 = rect2update->y;
        clip.x2 = rect2update->x + rect2update->w - 1;
        clip.y2 = rect2update->y + rect2update->h - 1;
        this->surface->setClip(&clip);
    }

	/* clear all or a part of the surface */
	if (clear)
		this->surface->clear();

	/* draw background */
    MMSFBColor bgcolor;
    getBgColor(bgcolor);
    if (this->bgimage) {
        /* prepare for blitting */
   		this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(255, (bgcolor.a)?bgcolor.a:255, 255);

        /* draw background with bgimage */
        this->surface->stretchBlit(this->bgimage, NULL, &(this->innerGeom));
    }
    else
    if (bgcolor.a) {
        /* prepare for drawing */
   		this->surface->setDrawingColorAndFlagsByBrightnessAndOpacity(bgcolor, 255, 255);

        /* draw window background */
        this->surface->fillRectangle(this->innerGeom.x, this->innerGeom.y, this->innerGeom.w, this->innerGeom.h);
    }


    /* draw children */
    bool backgroundFilled = true;
    if(!this->children.empty()) {
		if (this->draw_setgeom) {
			this->children.at(0)->setGeometry(this->innerGeom);
			this->draw_setgeom = false;
		}
        this->children.at(0)->drawchildren(toRedrawOnly, &backgroundFilled);
    }


	/* reset the clip */
    this->surface->setClip(NULL);

	/* unlock */
//PUP    this->surface->unlock();
    unlock();

	/* draw border */
    if (!toRedrawOnly)
        drawMyBorder();

}

void MMSWindow::drawMyBorder() {
	unsigned int borderThickness;
	if (!getBorderThickness(borderThickness))
		borderThickness = 0;
	bool borderRCorners;
	if (!getBorderRCorners(borderRCorners))
		borderRCorners = false;
	MMSFBColor c;

	getBorderColor(c);
    drawBorder(borderThickness, borderRCorners, this->borderimages,
               this->bordergeom, &(this->bordergeomset), this->surface,
               0, 0, this->geom.w, this->geom.h, c, this->im, 255, 255);
}


bool MMSWindow::show() {

    /* the window will be hidden in a few seconds (hideAction thread is running), wait for it */
    while (this->willhide)
        msleep(100);

    while(1) {
        /* check if the window is shown */
        if(this->shown) {
            /* call onAfterShow callback with already shown flag */
            this->onAfterShow->emit(this, true);
            return true;
        }
        /* check if the window will already be shown */
        if (this->willshow) {
            msleep(100);
            continue;
        }
        break;
    }

    /* start the show process */
    this->willshow = true;

    /* call onBeforeShow callback */
    if (!this->onBeforeShow->emit(this)) {
        /* a callback returns false, break the show process */
        this->willshow = false;
        return false;
    }


    if (getType() == MMSWINDOWTYPE_MAINWINDOW) {
        /* hide all main and popup windows */
        if (this->windowmanager) {
            this->windowmanager->hideAllPopupWindows(true);
            this->windowmanager->hideAllMainWindows();
        }
    }
//////////
    else
    if (getType() == MMSWINDOWTYPE_CHILDWINDOW) {
    	if (this->parent)
    		if (!this->parent->isShown(true)) {
    			// not really visible, break the show process
    			this->buffered_shown = true;
    		    this->setFirstFocus();
    	        this->shown = true;
    	        this->willshow = false;
    	        this->onAfterShow->emit(this, false);
    	        return true;
    		}
    }
///////////////

    if (this->action->isRunning())
        this->action->cancelBroadcast.emit(this->getType());
    this->action->setAction(MMSWACTION_SHOW);
    this->action->start();

    return true;
}


/////////////////////
void MMSWindow::showBufferedShown() {
	unsigned int cws = childwins.size();
	if (!cws) {
    	/* pre-calculate the navigation */
/*        if (this->parent) {
            this->parent->preCalcNavigation();
            this->switchArrowWidgets();
        }*/
	}

    for (unsigned int i = 0; i < childwins.size(); i++) {
		MMSWindow *w = childwins.at(i).window;

		if (w->shown) {
			lock();

		    if (!w->buffered_shown) {
		    	// it is not the first time, so one draw is enough
		    	// do it only for child windows with own surfaces
				bool os;
				w->getOwnSurface(os);
				if (os) {
					w->draw();
					w->parent->flipWindow(w);
				}
		    }
		    else {
		    	// buffered shown, first time called
				w->draw();
				w->draw();
				if (!w->initialized) {
					/* init window (e.g. pre-calc navigation ...) */
					w->init();
					w->initialized = true;
				}

				if (!w->initialArrowsDrawn) {
					/* set the arrow widgets */
					w->initialArrowsDrawn = true;
					w->switchArrowWidgets();
				}

				w->parent->flipWindow(w);

				if ((w->parent)||((!w->parent)&&(w->window))) {
					unsigned int opacity;
					if (!w->getOpacity(opacity)) opacity = 255;
					MMSFBRectangle rect = w->getGeometry();

					/* set final opacity */
					w->parent->setChildWindowOpacity(w, opacity);
				}

				w->buffered_shown = false;
				w->showBufferedShown();
		    }

			unlock();
		}
	}
}

//////////////////


bool MMSWindow::raiseToTop(int zlevel) {
//printf("ZZZ: raisetotop %s\n", name.c_str());
    if (!this->parent) {
        // normal parent window
        // set the window to top
        if (this->window)
            return this->window->raiseToTop(zlevel);
    }
    else {
    	// child window
    	// change the childwins vector
    	lock();
        for (unsigned int i = 0; i < this->parent->childwins.size(); i++)
        	if (this->parent->childwins.at(i).window == this) {
        		// child window found, move it to the end of the vector
        		if (i + 1 < this->parent->childwins.size()) {
        			// not at the end, moving it
        			CHILDWINS cw = this->parent->childwins.at(i);
					this->parent->childwins.erase(this->parent->childwins.begin()+i);
					this->parent->childwins.push_back(cw);
//printf("ZZZ: raisetotop2 0=%s 1=%s\n", this->parent->childwins.at(0).window->name.c_str(), this->parent->childwins.at(1).window->name.c_str());

//printf("QQQQQQQ1>>>>>>>>>>>>>>>>>>>>>>>>>>> %d, %d\n", this->parent->focusedChildWin, i);
					if (this->parent->focusedChildWin > i) {
						// the focused window will not be changed here!!!
						this->parent->focusedChildWin--;
//printf("QQQQQQQ2>>>>>>>>>>>>>>>>>>>>>>>>>>> %d\n", this->parent->focusedChildWin);
					}

					// the toplevel child window
					i = this->parent->childwins.size() - 1;

					// redraw the window stack if child window and parent is shown
					if ((this->parent->childwins.at(i).window->shown)&&(this->parent->shown)) {
						this->parent->flipWindow(this->parent->childwins.at(i).window, NULL, MMSFB_FLIP_NONE, false, true);
	      			}

        		}

/*
        		// change the focused child win if window is shown or will be shown
        		if (this->parent->childwins.at(i).window->shown) {// || this->parent->childwins.at(i).window->willshow) {
printf("ZZZ: raisetotop3 0=%s 1=%s\n", this->parent->childwins.at(0).window->name.c_str(), this->parent->childwins.at(1).window->name.c_str());

//this->parent->focusedChildWin = 1;
//this->parent->childwins.at(i).window->setFocus();
//this->setFocus();

					this->parent->focusedChildWin = i;
        		}

				// redraw the window stack if child window and parent is shown
				if ((this->parent->childwins.at(i).window->shown)&&(this->parent->shown)) {
					this->parent->flipWindow(this->parent->childwins.at(i).window, NULL, MMSFB_FLIP_NONE, false, true);
      			}
*/
                unlock();
        		return true;
        	}
        unlock();
    }
    return false;
}

bool MMSWindow::lowerToBottom() {
    if (!this->parent) {
        // normal parent window
		// set the window to bottom
		if (this->window)
			return this->window->lowerToBottom();
    }
    else {
    	// child window
    	// change the childwins vector
    	lock();
        for (unsigned int i = 0; i < this->parent->childwins.size(); i++)
        	if (this->parent->childwins.at(i).window == this) {
        		// child window found, move it to the begin of the vector
        		if (i > 0) {
					CHILDWINS cw = this->parent->childwins.at(i);
					this->parent->childwins.erase(this->parent->childwins.begin()+i);
					this->parent->childwins.insert(this->parent->childwins.begin(), cw);
        		}

        		// change the focused child win pointer
                for (int j = (int)this->parent->childwins.size()-1; j >= 0 ; j--)
                	if (this->parent->childwins.at(j).window->shown) {
						this->parent->focusedChildWin = j;
						break;
                	}

        		// redraw the window stack if parent is shown
        		if (this->parent->shown)
        			if (this->parent->childwins.at(0).window->shown)
        				this->parent->flipWindow(this->parent->childwins.at(0).window, NULL, MMSFB_FLIP_NONE, false, true);

        		unlock();
        		return true;
        	}
        unlock();
    }
    return false;
}


bool MMSWindow::showAction(bool *stopaction) {
    bool    saction = *stopaction;

//    printf("showAction %x\n", this);

    if(shown==true) {
        /* call onAfterShow callback with already shown flag */
        this->onAfterShow->emit(this, true);
        return !saction;
    }


    // set the first focused widget, if not set and if window can get the focus
    this->setFirstFocus();

//printf("showaction %s\n", name.c_str());

    if (getType() == MMSWINDOWTYPE_CHILDWINDOW) {
    	if ((int)this->parent->focusedChildWin >= 0) {
    		if (!this->parent->childwins.at(this->parent->focusedChildWin).window->isShown()) {
//printf("showaction2 %s\n", name.c_str());
    			// focused child window is not shown!
    			// so set focus to this window
    			setFocus();
    		}
    	}
    }


//printf("showaction3 %s\n", name.c_str());

/////////
//    if (getType() != MMSWINDOWTYPE_CHILDWINDOW) {
    	showBufferedShown();
//    }
///////////////////

//	printf("showAction2 %x\n", this);

    // check if all of its parents are shown
    bool really_shown = true;
    if (this->parent)
		really_shown = this->parent->isShown(true);

    /* set the first focused widget, if not set and if window can get the focus */
//    this->setFirstFocus();

//    printf("showAction3 %x\n", this);


    /* lock drawing */
//PUP    this->drawLock.lock();
    lock();

    if (getType() == MMSWINDOWTYPE_ROOTWINDOW) {
        // hide the current root window
        if (this->windowmanager) {
            this->windowmanager->hideAllRootWindows(true);
            this->windowmanager->lowerToBottom(this);
        }
        else {
        	lowerToBottom();
        }
    }
    else {
    	// bring all other windows in foreground
        if (!this->parent) {
            // normal parent window (main or popup)
            if (this->windowmanager) {
            	this->windowmanager->raiseToTop(this);
            }
            else {
            	raiseToTop();
            }
        }
        else {
        	// change the z-order of child windows?
        	bool staticzorder = false;
        	this->parent->getStaticZOrder(staticzorder);
        	if (!staticzorder)
        		raiseToTop();
        }
    }

    if ((getType() == MMSWINDOWTYPE_ROOTWINDOW) || (getType() == MMSWINDOWTYPE_MAINWINDOW)) {
		bool os;
		getOwnSurface(os);
		if (!os) {
			if (this->window) {
				// we are working with a subsurface of a fullscreen window
				this->window->setVisibleRectangle(&this->geom);
			}
		}
    }

//    printf("showAction4 %x\n", this);

    /* draw complete window two times!!! *********************************/
    /* two times are needed because if window is not shown (shown=false) */
    /* refreshFromChild does not work!!! -> but the second call to draw  */
    /* uses the current settings from all childs                         */
	draw();                                                            /**/
    draw();                                                            /**/
    /*********************************************************************/

//    printf("showAction5 %x\n", this);

    if (!this->initialized) {
        /* init window (e.g. pre-calc navigation ...) */
        init();
        this->initialized = true;
    }

    if (!this->initialArrowsDrawn) {
        /* set the arrow widgets */
        this->initialArrowsDrawn = true;
        switchArrowWidgets();
    }

    /* make it visible */
    if (!this->parent)
        flipWindow(this);
    else
        this->parent->flipWindow(this);

     /* unlock drawing */
//    this->drawLock.unlock();
    unlock();

    if (this->window)
        /* show window (normally the opacity is 0 here) */
        this->window->show();

	/* window is shown (important to set it before fading!!!) */
    shown=true;

    // per default only main or root windows can get inputs
    // popup windows can get inputs if the modal mode is set
    // child windows get the inputs from the parent main, root or popup window
    if (this->windowmanager) {
		switch (getType()) {
			case MMSWINDOWTYPE_MAINWINDOW:
			case MMSWINDOWTYPE_ROOTWINDOW:
				this->windowmanager->setToplevelWindow(this);
				break;
			case MMSWINDOWTYPE_POPUPWINDOW: {
				bool modal;
				if (getModal(modal)) {
					if (modal)
						this->windowmanager->setToplevelWindow(this);
				}
				break;
			}
			default:
				break;
		}
    }

    if ((this->parent)||((!this->parent)&&(this->window))) {
	    unsigned int opacity;
	    if (!getOpacity(opacity)) opacity = 255;
        MMSFBRectangle rect = getGeometry();

	    bool fadein;
	    if (!getFadeIn(fadein)) fadein = false;
	    MMSDIRECTION movein;
	    if (!getMoveIn(movein)) movein = MMSDIRECTION_NOTSET;

	    if ((really_shown)&&((fadein)||(movein!=MMSDIRECTION_NOTSET))) {
		    // little animation
    	    int steps = 3;
    	    unsigned int opacity_step;
    	    int move_step;

    	    switch (movein) {
    	    	case MMSDIRECTION_LEFT:
            	    move_step = (vrect.w-rect.x+vrect.x) / (steps+1);
    	    		break;
    	    	case MMSDIRECTION_RIGHT:
            	    move_step = (rect.w-vrect.x+rect.x) / (steps+1);
    	    		break;
    	    	case MMSDIRECTION_UP:
            	    move_step = (vrect.h-rect.y+vrect.y) / (steps+1);
            	    break;
    	    	case MMSDIRECTION_DOWN:
            	    move_step = (rect.h-vrect.y+rect.y) / (steps+1);
    	    		break;
    	    	default:
    	    		break;
    	    }

    	    if (fadein)
    	    	opacity_step = opacity / (steps+1);

    	    for (int i = steps; i > 0; i--) {
	            // start time stamp
    	    	unsigned int start_ts = getMTimeStamp();

        	    switch (movein) {
        	    	case MMSDIRECTION_LEFT:
        	    		if (!parent)
        	    			this->window->moveTo((rect.x + i * move_step) & ~0x01, rect.y);
        	    		else
        	    			this->parent->moveChildWindow(this, (rect.x + i * move_step) & ~0x01, rect.y);
        	    		break;
        	    	case MMSDIRECTION_RIGHT:
        	    		if (!parent)
        	    			this->window->moveTo((rect.x - i * move_step) & ~0x01, rect.y);
        	    		else
        	    			this->parent->moveChildWindow(this, (rect.x - i * move_step) & ~0x01, rect.y);
        	    		break;
        	    	case MMSDIRECTION_UP:
        	    		if (!parent)
        	    			this->window->moveTo(rect.x, (rect.y + i * move_step) & ~0x01);
        	    		else
        	    			this->parent->moveChildWindow(this, rect.x, (rect.y + i * move_step) & ~0x01);
        	    		break;
        	    	case MMSDIRECTION_DOWN:
        	    		if (!parent)
        	    			this->window->moveTo(rect.x, (rect.y - i * move_step) & ~0x01);
        	    		else
        	    			this->parent->moveChildWindow(this, rect.x, (rect.y - i * move_step) & ~0x01);
        	    		break;
        	    	default:
        	    		break;
        	    }

        	    if (fadein) {
    	    		if (!parent)
    	    			this->window->setOpacity(opacity - i * opacity_step);
    	    		else
    	    	        this->parent->setChildWindowOpacity(this, opacity - i * opacity_step);
        	    }
        	    else
        	    if (i == steps) {
    	    		if (!parent)
    	    			this->window->setOpacity(opacity);
    	    		else
    	    	        this->parent->setChildWindowOpacity(this, opacity);
        	    }

	            if (*stopaction) {
	                saction=true;
	                break;
	            }

	            // end time stamp
    	    	unsigned int end_ts = getMTimeStamp();

    	    	// sleeping a little...
    	    	msleep(getFrameDelay(start_ts, end_ts));
	        }
	    }

	    /* set final position */
	    if (movein!=MMSDIRECTION_NOTSET) {
    		if (!parent)
    			this->window->moveTo(rect.x, rect.y);
    		else
    			this->parent->moveChildWindow(this, rect.x, rect.y);
	    }

	    /* set final opacity */
		if (!parent)
			this->window->setOpacity(opacity);
		else
			this->parent->setChildWindowOpacity(this, opacity);
    }

    willshow=false;

    if (getType() == MMSWINDOWTYPE_CHILDWINDOW) {


//TODO: if no focused childwin, then i should get the focused back


    	/* pre-calculate the navigation */
        if (this->parent) {
            this->parent->preCalcNavigation();
            this->switchArrowWidgets();
        }
    }

    *stopaction=false;

    /* call onAfterShow callback without already shown flag */
    this->onAfterShow->emit(this, false);

    return !saction;
}

bool MMSWindow::hide(bool goback, bool wait) {

    /* the window will be shown in a few seconds (showAction thread is running), wait for it */
    while (this->willshow)
        msleep(100);

    while(1) {
        /* check if the window is shown */
        if(!this->shown)
            return true;
        /* check if the window will already be hidden */
        if (this->willhide) {
            msleep(100);
            continue;
        }
        break;
    }

    /* starting hide process */
    this->willhide = true;

    /* call onBeforeHide callback */
    if (!this->onBeforeHide->emit(this, goback)) {
        /* a callback returns false, break the hide process */
        this->willhide = false;
        return false;
    }

    if (this->action->isRunning())
        this->action->cancelBroadcast.emit(this->getType());
    this->action->setAction(MMSWACTION_HIDE);
    this->action->start();

    if (wait) {
    	int c = 0;
        while ((this->action->getAction()==MMSWACTION_HIDE) && c < 20) { msleep(100); c++; }
    }

    /* call onHide callback */
    this->onHide->emit(this, goback);

    return true;
}

bool MMSWindow::hideAction(bool *stopaction) {
    bool    saction = *stopaction;

    if(shown==false)
        return !saction;

    // check if all of its parents are shown
   	bool really_shown = this->isShown(true);

    if (!this->parent)
        if (this->windowmanager)
            this->windowmanager->removeWindowFromToplevel(this);

    if (getType() == MMSWINDOWTYPE_CHILDWINDOW) {
        /* remove the focus from me */
    	removeFocusFromChildWindow();
    }

    if ((this->parent)||((!this->parent)&&(this->window))) {
	    unsigned int opacity;
	    if (!getOpacity(opacity)) opacity = 255;
        MMSFBRectangle rect = getGeometry();

	    bool fadeout;
	    if (!getFadeOut(fadeout)) fadeout = false;
	    MMSDIRECTION moveout;
	    if (!getMoveOut(moveout)) moveout = MMSDIRECTION_NOTSET;

	    if ((really_shown)&&((fadeout)||(moveout!=MMSDIRECTION_NOTSET))) {
		    // little animation
    	    int steps = 3;
    	    unsigned int opacity_step;
    	    int move_step;

    	    switch (moveout) {
    	    	case MMSDIRECTION_LEFT:
            	    move_step = (rect.w-vrect.x+rect.x) / (steps+1);
    	    		break;
    	    	case MMSDIRECTION_RIGHT:
            	    move_step = (vrect.w-rect.x+vrect.x) / (steps+1);
    	    		break;
    	    	case MMSDIRECTION_UP:
            	    move_step = (rect.h-vrect.y+rect.y) / (steps+1);
            	    break;
    	    	case MMSDIRECTION_DOWN:
            	    move_step = (vrect.h-rect.y+vrect.y) / (steps+1);
    	    		break;
    	    	default:
    	    		break;
    	    }

    	    if (fadeout)
    	    	opacity_step = opacity / (steps+1);

       	    for (int i = 1; i <= steps; i++) {

	            // start time stamp
    	    	unsigned int start_ts = getMTimeStamp();

    	    	switch (moveout) {
        	    	case MMSDIRECTION_LEFT:
        	    		if (!parent)
        	    			this->window->moveTo((rect.x - i * move_step) & ~0x01, rect.y);
        	    		else
        	    			this->parent->moveChildWindow(this, (rect.x - i * move_step) & ~0x01, rect.y);
        	    		break;
        	    	case MMSDIRECTION_RIGHT:
        	    		if (!parent)
        	    			this->window->moveTo((rect.x + i * move_step) & ~0x01, rect.y);
        	    		else
        	    			this->parent->moveChildWindow(this, (rect.x + i * move_step) & ~0x01, rect.y);
        	    		break;
        	    	case MMSDIRECTION_UP:
        	    		if (!parent)
        	    			this->window->moveTo(rect.x, (rect.y - i * move_step) & ~0x01);
        	    		else
        	    			this->parent->moveChildWindow(this, rect.x, (rect.y - i * move_step) & ~0x01);
        	    		break;
        	    	case MMSDIRECTION_DOWN:
        	    		if (!parent)
        	    			this->window->moveTo(rect.x, (rect.y + i * move_step) & ~0x01);
        	    		else
        	    			this->parent->moveChildWindow(this, rect.x, (rect.y + i * move_step) & ~0x01);
        	    		break;
        	    	default:
        	    		break;
        	    }

        	    if (fadeout) {
    	    		if (!parent)
    	    			this->window->setOpacity(opacity - i * opacity_step);
    	    		else
    	    	        this->parent->setChildWindowOpacity(this, opacity - i * opacity_step);
        	    }

	            if (*stopaction) {
	                saction=true;
	                break;
	            }

	            // end time stamp
    	    	unsigned int end_ts = getMTimeStamp();

    	    	// sleeping a little...
    	    	msleep(getFrameDelay(start_ts, end_ts));

	        }
	    }

		if (!parent) {
		    /* set final opacity */
			this->window->setOpacity(0);
        	this->window->hide();

    	    /* restore position */
    	    if (moveout!=MMSDIRECTION_NOTSET)
    	    	this->window->moveTo(rect.x, rect.y);
		}
		else {
		    /* set final opacity */
	        this->parent->setChildWindowOpacity(this, 0);

	        /* restore position */
    	    if (moveout!=MMSDIRECTION_NOTSET)
    			this->parent->moveChildWindow(this, rect.x, rect.y);
		}

    }
    else {
        /* check if i have the surface from layer */
        if (this->surface) {
            /* clear it */
            this->surface->clear();
            this->surface->flip();
        }
    }


    shown=false;
    willhide=false;

    if (getType() == MMSWINDOWTYPE_CHILDWINDOW) {
        /* pre-calculate the navigation */
        if (this->parent) {
            this->parent->preCalcNavigation();
            switchArrowWidgets();
        }
    }

    *stopaction=false;

    return !saction;
}

void MMSWindow::waitUntilShown() {
	while ((!isShown())||(willshow))
		msleep(10);
}

void MMSWindow::waitUntilHidden() {
	while ((isShown())||(willhide))
		msleep(10);
}

void MMSWindow::add(MMSWidget *child) {

    /* prevent duplicate items */
    for(unsigned int i = 0; i < this->children.size(); i++) {
        if(children.at(i)->getId() == child->getId())
            return;
    }

    /* add to the children vector */
    lock();
    this->children.push_back(child);
    unlock();
}

void MMSWindow::remove(MMSWidget *child) {
    /* remove from children vector */
    for(unsigned int i = 0; i < this->children.size(); i++) {
        if(children.at(i) == child) {
            this->children.erase(this->children.begin()+i);
            return;
        }
    }
}


void MMSWindow::refreshFromChild(MMSWidget *child, MMSFBRectangle *rect2update, bool check_shown) {
    MMSFBRegion  	region;

    // use own surface?
    // note: os=false must ONLY be set, if this window is a child window!!!
	bool os = true;
	if (this->type == MMSWINDOWTYPE_CHILDWINDOW)
		getOwnSurface(os);

	if (check_shown) {
	    // it makes sense that we skip all drawing requests here, if this window OR one of its parents are not shown
	    if (!isShown(true)) {
	    	DEBUGMSG("MMSGUI", "MMSWindow->refreshFromChild() skipped because window is not shown");
	        return;
	    }
	}

    /* lock drawing */
//PUP    this->drawLock.lock();
	lock();


    // calculate region
    MMSFBRectangle rect;
    MMSWidget *c = child;
    if ((!c)&&(!children.empty()))
	    c = children.at(0);
    if (c) {
        if (!rect2update) {
            rect = c->getGeometry();
            if (c->isDrawable()) {
                unsigned int childmargin;
                if (!c->getMargin(childmargin))
                	childmargin = 0;
                rect.x+=childmargin;
                rect.y+=childmargin;
                rect.w-=2*childmargin;
                rect.h-=2*childmargin;
            }
        }
        else
            rect = *rect2update;

        // check x/y
        if (rect.x < this->innerGeom.x) {
        	rect.w-= this->innerGeom.x - rect.x;
        	rect.x = this->innerGeom.x;
        }
        if (rect.y < this->innerGeom.y) {
        	rect.h-= this->innerGeom.y - rect.y;
        	rect.y = this->innerGeom.y;
        }

        // valid rectangle?
        if ((rect.w <= 0)||(rect.h <= 0)) {
        	unlock();
        	return;
        }

        // check width/height
        if (rect.x + rect.w > this->innerGeom.x + this->innerGeom.w)
        	rect.w = this->innerGeom.w - rect.x;
        if (rect.y + rect.h > this->innerGeom.y + this->innerGeom.h)
        	rect.h = this->innerGeom.h - rect.y;

        // valid rectangle?
        if ((rect.w <= 0)||(rect.h <= 0)) {
        	unlock();
        	return;
        }
    }
    else {
        rect = this->innerGeom;
    }

    if(child) {
    	/* draw only childs of this child */
		if (os)
			child->drawchildren();
    }
    else {
        /* draw only some parts of the window */
    	if (os)
    		draw(true, (rect2update)?&rect:NULL);
    }

    // set region
    region.x1 = rect.x;
    region.x2 = rect.x+rect.w-1;
    region.y1 = rect.y;
    region.y2 = rect.y+rect.h-1;

//    logger.writeLog("flip the region (x1,y1,x2,y2) (" +
//        iToStr(region.x1) + "," + iToStr(region.y1) + "," + iToStr(region.x2) + "," + iToStr(region.y2) +")");

    if (!bordergeomset)
        /* border geom is not set -> draw the border */
        drawMyBorder();
    else {
        /* draw the border if it is in the flipping region */
        bool htdb = false;

        /* check if border should be drawn */
        if (!htdb)
            htdb = ((bordergeom[0].x + bordergeom[0].w > region.x1)
                  &&(bordergeom[0].y + bordergeom[0].h > region.y1));
        if (!htdb)
            htdb = (bordergeom[1].y + bordergeom[1].h > region.y1);
        if (!htdb)
            htdb = ((bordergeom[2].x <= region.x2)
                  &&(bordergeom[2].y + bordergeom[2].h > region.y1));
        if (!htdb)
            htdb = (bordergeom[3].x <= region.x2);
        if (!htdb)
            htdb = ((bordergeom[4].x <= region.x2)
                  &&(bordergeom[4].y <= region.y2));
        if (!htdb)
            htdb = (bordergeom[5].y <= region.y2);
        if (!htdb)
            htdb = ((bordergeom[6].x + bordergeom[6].w > region.x1)
                  &&(bordergeom[6].y <= region.y2));
        if (!htdb)
            htdb = (bordergeom[7].x + bordergeom[7].w > region.x1);

        if (htdb) {
            /* I have to draw the border */
        	DEBUGMSG("MMSGUI", "draw window border");
            drawMyBorder();
        }
    }

    /* flip region */
    if (!this->parent)
        flipWindow(this, &region, MMSFB_FLIP_ONSYNC);
    else
        this->parent->flipWindow(this, &region, MMSFB_FLIP_ONSYNC);

    /* unlock drawing */
//PUP    this->drawLock.unlock();
    unlock();
}

void MMSWindow::refresh() {

    if(shown==false) {
//        logger.writeLog("drawing skipped because window is not shown");
        return;
    }

    /* lock drawing */
//PUP    this->drawLock.lock();
    lock();

    /* draw complete window */
    this->draw_setgeom = true;
    draw();

    /* make it visible */
    if (!this->parent)
        flipWindow(this);
    else
        this->parent->flipWindow(this);

    /* unlock drawing */
//PUP    this->drawLock.unlock();
    unlock();
}


void MMSWindow::setFocusedWidget(MMSWidget *child, bool set, bool switchfocus) {
//printf("XXX: setFocusedWidget for window %s %x set = %d, switchfocus = %d\n", name.c_str(), this, set, switchfocus);
	if (set) {
    	if (switchfocus) {
    		if (this->focusedwidget)
    			this->focusedwidget->setFocus(false);
    		if (child)
    			child->setFocus(true);
    	}
        this->focusedwidget = child;
        this->firstfocusset = true;
    }
    else {
        if (child)
            if (child->isFocused()) {
            	if (switchfocus)
            		child->setFocus(false);
                this->focusedwidget = NULL;
                this->firstfocusset = false;
            }
    }

    switchArrowWidgets();
}

bool MMSWindow::setFirstFocus(bool cw) {
//printf("XXX: setFirstFocus1 to %s %x\n", name.c_str(), this);


    // per default only main or root windows can get inputs
    // popup windows can get inputs if the modal mode is set
    // child windows get the inputs from the parent main, root or popup window
    switch (getType()) {
        case MMSWINDOWTYPE_MAINWINDOW:
        case MMSWINDOWTYPE_ROOTWINDOW:
            break;
		case MMSWINDOWTYPE_POPUPWINDOW: {
			bool modal;
			if (getModal(modal)) {
				if (modal)
					this->windowmanager->setToplevelWindow(this);
			}
			break;
		}
        case MMSWINDOWTYPE_CHILDWINDOW:
            if (!cw) return false;
//printf ("parent->focusedChildWin %d\n", parent->focusedChildWin);
//printf ("parent->focusedwidget %d\n", this->focusedwidget);
            break;
        default:
            return false;
    }

//printf("XXX: setFirstFocus2 to %s\n", name.c_str());
    DEBUGMSG("MMSGUI", "MMSWindow: setFirstFocus to " + getName());

    if (this->firstfocusset) {
//printf("XXX: setFirstFocus2.2 to %s\n", name.c_str());
    	DEBUGMSG("MMSGUI", "MMSWindow: focus already set");
        return true;
    }
    this->firstfocusset = true;
    bool b;

//printf("XXX: setFirstFocus3 to %s\n", name.c_str());

    if(this->children.empty()) {
        bool found = false;

        for (unsigned int j = 0; j < this->childwins.size(); j++) {
            MMSWindow *w = this->childwins.at(j).window;

//printf("XXX: setFirstFocus4 to %s -> childwin %s\n", name.c_str(), w->name.c_str());

			if (w->getNumberOfFocusableWidgets()) {
//printf("XXX: setFirstFocus5 to %s -> childwin %s %d\n", name.c_str(), w->name.c_str(), j);
                /* widgets found which can be focused */
                this->focusedChildWin = j;
                found = true;
                if (!w->firstfocusset) {
//printf("XXX: setFirstFocus6 to %s -> childwin %s\n", name.c_str(), w->name.c_str());
                    for(unsigned int i=0;i<w->children.size();i++) {
                        if(w->children.at(i)->getFocusable(b))
                        	if (b) {
                        		DEBUGMSG("MMSGUI", "MMSWindow: set focus to child nr " + iToStr(i));
                        		string inputmode = "";
                        		w->children.at(i)->getInputModeEx(inputmode);
                        		if (strToUpr(inputmode) != "CLICK") {
//printf("XXX: setFirstFocus7 to %s -> childwin %s\n", name.c_str(), w->name.c_str());
                        			w->children.at(i)->setFocus(true, false);
                        		}
								else {
									w->children.at(i)->setFocus(false, false);
								}
	                            w->firstfocusset = true;
	                            this->childwins.at(j).focusedWidget = i;
	                            return true;
	                        }
                    }
                }
                break;
            }
            else
            if (w->getNumberOfFocusableChildWins()) {
                /* child windows found which have focusable widgets */
                this->focusedChildWin = j;
                found = true;
                if (!w->firstfocusset) {
                    for(unsigned int i=0;i<w->childwins.size();i++) {
                        if (w->childwins.at(i).window->setFirstFocus(true))
                            return true;
                    }
                }
                break;
            }
        }

        if (found) {
            static bool again;

            if (!again) {
                 int fd = open( "/dev/kmsg", O_WRONLY );

                 if (fd >= 0) {
                      char msg[] = "MMSWindow::setFirstFocus() found first focus\n";

                      write( fd, msg, sizeof(msg) );
                      close( fd );
                 }

                 if (getenv( "MMS_EXIT_ON_FIRST_FOCUS" ))
                     exit(0);

                 again = true;
            }
            return true;
        }

        DEBUGMSG("MMSGUI", "MMSWindow: no children to focus for window " + getName());
        return false;
    }


    for(unsigned int i=0;i<this->children.size();i++) {
        if(this->children.at(i)->getFocusable(b))
        	if (b) {
        		DEBUGMSG("MMSGUI", "MMSWindow: set focus to child nr " + iToStr(i));
        		string inputmode = "";
        		this->children.at(i)->getInputModeEx(inputmode);
        		if (strToUpr(inputmode) != "CLICK")
        			this->children.at(i)->setFocus(true);
	            return true;
	        }
    }

    DEBUGMSG("MMSGUI", "MMSWindow: no children to focus for window " + getName());
    return false;
}



#define MAXDGCODE   999999

/* a lower return value is better than an higher value */
double MMSWindow::calculateDistGradCode_Up(MMSFBRectangle currPos, MMSFBRectangle candPos) {

    MMSFB_BREAK();

    /* check if candidate is over the current widget */
    if (candPos.y >= currPos.y)
        /* no */
        return MAXDGCODE;

    /* create some vector points */
    double w1 = (double)candPos.x;                          // left border to left border
           w1-= (double)currPos.x;
    double w2 = (double)candPos.x + (double)candPos.w - 1;  // right border to right border
           w2-= (double)currPos.x + (double)currPos.w - 1;
    double w3 = (double)candPos.x + ((double)candPos.w / 2);// middle to middle
           w3-= (double)currPos.x + ((double)currPos.w / 2);
    double h  = (double)candPos.y + ((double)candPos.h / 2);// height
           h -= (double)currPos.y + ((double)currPos.h / 2);

    /* if the candidate is direct over the current widget and candidate width is equal or greater */
    if ((w1 - (double)currPos.w / 2 <= 0) && (w2 + (double)currPos.w / 2 >= 0)) {
        /* then set the gradient of the middle vector to zero */
        w3 = 0;

        /* set smallest possible height */
        h = 0 - ((double)currPos.y - ((double)candPos.y + (double)candPos.h - 1));
    }

    /* check if correct quadrant */
    if (h >= 0)
        return MAXDGCODE;

    /* get absolute values */
    w1 = fabs(w1);
    w2 = fabs(w2);
    w3 = fabs(w3);
    h  = fabs(h);

    /* temporary storage for distance */
    double dist;

    /* temporary result storage */
    double dgcode1, dgcode2;

    /* work with left vector */
    dist = sqrt(w1*w1 + h*h);
    if (w1 <= h) {
        /* calc the gradient and the result code */
        double grad = w1 / h;
        dgcode1 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode1 = MAXDGCODE-MAXDGCODE/2+dist;

    /* work with right vector */
    dist = sqrt(w2*w2 + h*h);
    if (w2 <= h) {
        /* calc the gradient and the result code */
        double grad = w2 / h;
        dgcode2 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode2 = MAXDGCODE-MAXDGCODE/2+dist;

    /* take the smallest value */
    if (dgcode1 > dgcode2)
        dgcode1 = dgcode2;

    /* work with middle vector */
    dist = sqrt(w3*w3 + h*h);
    if (w3 <= h) {
        /* calc the gradient and the result code */
        double grad = w3 / h;
        dgcode2 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode2 = MAXDGCODE-MAXDGCODE/2+dist;

    /* return with the smallest value */
    if (dgcode1 < dgcode2)
        return dgcode1;

    /* return the result */
    return dgcode2;
}


/* a lower return value is better than an higher value */
double MMSWindow::calculateDistGradCode_Down(MMSFBRectangle currPos, MMSFBRectangle candPos) {

    /* check if candidate is under the current widget */
    if (candPos.y + candPos.h - 1 <= currPos.y + currPos.h - 1)
        /* no */
        return MAXDGCODE;

    /* create some vector points */
    double w1 = (double)candPos.x;                          // left border to left border
           w1-= (double)currPos.x;
    double w2 = (double)candPos.x + (double)candPos.w - 1;  // right border to right border
           w2-= (double)currPos.x + (double)currPos.w - 1;
    double w3 = (double)candPos.x + ((double)candPos.w / 2);// middle to middle
           w3-= (double)currPos.x + ((double)currPos.w / 2);
    double h  = (double)candPos.y + ((double)candPos.h / 2);// height
           h -= (double)currPos.y + ((double)currPos.h / 2);

    /* if the candidate is direct under the current widget and candidate width is equal or greater */
    if ((w1 - (double)currPos.w / 2 <= 0) && (w2 + (double)currPos.w / 2 >= 0)) {
        /* then set the gradient of the middle vector to zero */
        w3 = 0;

        /* set smallest possible height */
        h = (double)candPos.y - ((double)currPos.y + (double)currPos.h - 1);
    }

    /* check if correct quadrant */
    if (h <= 0)
        return MAXDGCODE;

    /* get absolute values */
    w1 = fabs(w1);
    w2 = fabs(w2);
    w3 = fabs(w3);
    h  = fabs(h);

    /* temporary storage for distance */
    double dist;

    /* temporary result storage */
    double dgcode1, dgcode2;

    /* work with left vector */
    dist = sqrt(w1*w1 + h*h);
    if (w1 <= h) {
        /* calc the gradient and the result code */
        double grad = w1 / h;
        dgcode1 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode1 = MAXDGCODE-MAXDGCODE/2+dist;

    /* work with right vector */
    dist = sqrt(w2*w2 + h*h);
    if (w2 <= h) {
        /* calc the gradient and the result code */
        double grad = w2 / h;
        dgcode2 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode2 = MAXDGCODE-MAXDGCODE/2+dist;

    /* take the smallest value */
    if (dgcode1 > dgcode2)
        dgcode1 = dgcode2;

    /* work with middle vector */
    dist = sqrt(w3*w3 + h*h);
    if (w3 <= h) {
        /* calc the gradient and the result code */
        double grad = w3 / h;
        dgcode2 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode2 = MAXDGCODE-MAXDGCODE/2+dist;

    /* return with the smallest value */
    if (dgcode1 < dgcode2)
        return dgcode1;

    /* return the result */
    return dgcode2;
}


/* a lower return value is better than an higher value */
double MMSWindow::calculateDistGradCode_Left(MMSFBRectangle currPos, MMSFBRectangle candPos) {

    /* check if candidate is left of the current widget */
    if (candPos.x >= currPos.x)
        return MAXDGCODE;

    /* create some vector points */
    double h1 = (double)candPos.y;                          // top border to top border
           h1-= (double)currPos.y;
    double h2 = (double)candPos.y + (double)candPos.h - 1;  // bottom border to bottom border
           h2-= (double)currPos.y + (double)currPos.h - 1;
    double h3 = (double)candPos.y + ((double)candPos.h / 2);// middle to middle
           h3-= (double)currPos.y + ((double)currPos.h / 2);
    double w  = (double)candPos.x + ((double)candPos.w / 2);// width
           w -= (double)currPos.x + ((double)currPos.w / 2);

    /* if the candidate is direct left of the current widget and candidate width is equal or greater */
    if ((h1 - (double)currPos.h / 2 <= 0) && (h2 + (double)currPos.h / 2 >= 0)) {
        /* then set the gradient of the middle vector to zero */
        h3 = 0;

        /* set smallest possible width */
        w = 0-((double)currPos.x - ((double)candPos.x + (double)candPos.w - 1));
    }

    /* check if correct quadrant */
    if (w >= 0)
        return MAXDGCODE;

    /* get absolute values */
    h1 = fabs(h1);
    h2 = fabs(h2);
    h3 = fabs(h3);
    w  = fabs(w);

    /* temporary storage for distance */
    double dist;

    /* temporary result storage */
    double dgcode1, dgcode2;

    /* work with top vector */
    dist = sqrt(h1*h1 + w*w);
    if (h1 <= w) {
        /* calc the gradient and the result code */
        double grad = h1 / w;
        dgcode1 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode1 = MAXDGCODE-MAXDGCODE/2+dist;

    /* work with bottom vector */
    dist = sqrt(h2*h2 + w*w);
    if (h2 <= w) {
        /* calc the gradient and the result code */
        double grad = h2 / w;
        dgcode2 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode2 = MAXDGCODE-MAXDGCODE/2+dist;

    /* take the smallest value */
    if (dgcode1 > dgcode2)
        dgcode1 = dgcode2;

    /* work with middle vector */
    dist = sqrt(h3*h3 + w*w);
    if (h3 <= w) {
        /* calc the gradient and the result code */
        double grad = h3 / w;
        dgcode2 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode2 = MAXDGCODE-MAXDGCODE/2+dist;

    /* return with the smallest value */
    if (dgcode1 < dgcode2)
        return dgcode1;

    /* return the result */
    return dgcode2;
}


/* a lower return value is better than an higher value */
double MMSWindow::calculateDistGradCode_Right(MMSFBRectangle currPos, MMSFBRectangle candPos) {

    /* check if candidate is right of the current widget */
    if (candPos.x + candPos.w - 1 <= currPos.x + currPos.w - 1)
        return MAXDGCODE;

    /* create some vector points */
    double h1 = (double)candPos.y;                          // top border to top border
           h1-= (double)currPos.y;
    double h2 = (double)candPos.y + (double)candPos.h - 1;  // bottom border to bottom border
           h2-= (double)currPos.y + (double)currPos.h - 1;
    double h3 = (double)candPos.y + ((double)candPos.h / 2);// middle to middle
           h3-= (double)currPos.y + ((double)currPos.h / 2);
    double w  = (double)candPos.x + ((double)candPos.w / 2);// width
           w -= (double)currPos.x + ((double)currPos.w / 2);

    /* if the candidate is direct right of the current widget and candidate width is equal or greater */
    if ((h1 - (double)currPos.h / 2 <= 0) && (h2 + (double)currPos.h / 2 >= 0)) {
        /* then set the gradient of the middle vector to zero */
        h3 = 0;

        /* set smallest possible width */
        w = (double)candPos.x - ((double)currPos.x + (double)currPos.w - 1);
    }

    /* check if correct quadrant */
    if (w <= 0)
        return MAXDGCODE;

    /* get absolute values */
    h1 = fabs(h1);
    h2 = fabs(h2);
    h3 = fabs(h3);
    w  = fabs(w);

    /* temporary storage for distance */
    double dist;

    /* temporary result storage */
    double dgcode1, dgcode2;

    /* work with top vector */
    dist = sqrt(h1*h1 + w*w);
    if (h1 <= w) {
        /* calc the gradient and the result code */
        double grad = h1 / w;
        dgcode1 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode1 = MAXDGCODE-MAXDGCODE/2+dist;

    /* work with bottom vector */
    dist = sqrt(h2*h2 + w*w);
    if (h2 <= w) {
        /* calc the gradient and the result code */
        double grad = h2 / w;
        dgcode2 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode2 = MAXDGCODE-MAXDGCODE/2+dist;

    /* take the smallest value */
    if (dgcode1 > dgcode2)
        dgcode1 = dgcode2;

    /* work with middle vector */
    dist = sqrt(h3*h3 + w*w);
    if (h3 <= w) {
        /* calc the gradient and the result code */
        double grad = h3 / w;
        dgcode2 = dist / (1 - grad);
    }
    else
        /* the grad is higher than 1 and will be ignored */
        dgcode2 = MAXDGCODE-MAXDGCODE/2+dist;

    /* return with the smallest value */
    if (dgcode1 < dgcode2)
        return dgcode1;

    /* return the result */
    return dgcode2;
}


bool MMSWindow::handleNavigationForWidgets(MMSInputEvent *inputevent) {
    MMSWidget *candidate = NULL;

    /* if no focused widget then return */
    if (!this->focusedwidget)
        return false;

	if (inputevent->type == MMSINPUTEVENTTYPE_KEYPRESS) {
		/* keyboard inputs */

		/* check if widget names for navigation are set to the focused widget */
		switch (inputevent->key) {
			case MMSKEY_CURSOR_DOWN:
		        candidate = this->focusedwidget->getNavigateDownWidget();
		        break;
			case MMSKEY_CURSOR_UP:
		        candidate = this->focusedwidget->getNavigateUpWidget();
		        break;
			case MMSKEY_CURSOR_RIGHT:
		        candidate = this->focusedwidget->getNavigateRightWidget();
		        break;
			case MMSKEY_CURSOR_LEFT:
		        candidate = this->focusedwidget->getNavigateLeftWidget();
		        break;

            default:
                break;
		}
	}
	else {
		/* other inputs */
	}

    if (candidate) {
        /* i found a new widget */
        if (candidate->getId() != this->focusedwidget->getId()) {
            /* different from current focus */
            this->focusedwidget->setFocus(false);
            candidate->setFocus(true, true, inputevent);
            return true;
        }
    }

    return false;
}



void MMSWindow::removeChildWinFocus() {
    if (this->childwins.size() > this->focusedChildWin) {

        /* get the focused childwin */
        MMSWindow *fWin = this->childwins.at(this->focusedChildWin).window;

        if (!fWin->children.empty()) {
            /* save focused widget from current window and remove the focus */
            for(unsigned int i=0;i<fWin->children.size();i++) {
                if(fWin->children.at(i)->isFocused()) {
                    childwins.at(this->focusedChildWin).focusedWidget = i;
                    fWin->children.at(i)->setFocus(false);

                    /* set the arrow widgets */
                    fWin->switchArrowWidgets();

                    break;
                }
            }
        }
        else {
            /* recursive to my focused childwin */
            fWin->removeChildWinFocus();
        }
    }
}

bool MMSWindow::restoreChildWinFocus(MMSInputEvent *inputevent) {

    if (this->childwins.size() > this->focusedChildWin) {

        /* get the focused childwin */
        MMSWindow *fWin = this->childwins.at(this->focusedChildWin).window;

        /* check if something to focus */
        if (!fWin->getNumberOfFocusableWidgets())
            if (!fWin->getNumberOfFocusableChildWins())
                return false;

        if (!fWin->children.empty()) {
            /* restore focused widget to candidate window */
        	bool b;
            if (!fWin->children.at(this->childwins.at(this->focusedChildWin).focusedWidget)->getFocusable(b))
            	b = false;

            if (b) {
        		string inputmode = "";
        		fWin->children.at(this->childwins.at(this->focusedChildWin).focusedWidget)->getInputModeEx(inputmode);
        		if (strToUpr(inputmode) != "CLICK") {
        			fWin->children.at(this->childwins.at(this->focusedChildWin).focusedWidget)->setFocus(true, true, inputevent);
        		}
            }
            else {
                /* last focusable widget is not focusable anymore, search other widget to focus */
                for(unsigned int i=0;i<fWin->children.size();i++) {
                    if(fWin->children.at(i)->getFocusable(b))
                    	if (b) {
		                    this->childwins.at(this->focusedChildWin).focusedWidget = i;

		            		string inputmode = "";
		            		fWin->children.at(i)->getInputModeEx(inputmode);
		            		if (strToUpr(inputmode) != "CLICK") {
		            			fWin->children.at(i)->setFocus(true, true, inputevent);
		            		}

		                    break;
		                }
                }
            }

            /* set the arrow widgets */
            fWin->switchArrowWidgets();
        }
        else {
            /* recursive to my focused childwin */
            if (!fWin->restoreChildWinFocus(inputevent)) {

                /* nothing to focus, searching for other childwin */
                for(unsigned int i = 0; i < fWin->childwins.size(); i++) {
                    if (i == fWin->focusedChildWin)
                        continue;

                    /* found */
                    fWin->focusedChildWin = i;

                    /* next try */
                    if (fWin->restoreChildWinFocus(inputevent))
                        /* okay */
                        return true;
                    else
                        /* try with next */
                        continue;
                }

                /* no childwin found, reset the focus to the first childwin */
                fWin->focusedChildWin = 0;
                return false;
            }
        }
    }

    return true;
}


void MMSWindow::setFocus() {

//printf("setFocus %s\n", name.c_str());

    /* i do only work for child windows */
    if (!this->parent) return;

    /* searching me */
    int me = -1;
    for (unsigned int i = 0; i < this->parent->childwins.size(); i++)
        if (this->parent->childwins.at(i).window == this) {
            me = i;
            break;
        }

//printf("setFocus2 %s\n", name.c_str());

    /* i have found me within my parents list */
    if (me < 0) return;

//printf("setFocus3 %s, %d, %d, parent = %s\n", name.c_str(), this->parent->focusedChildWin, me, this->parent->name.c_str());

    /* i am the currently focused child window */
    if ((int)this->parent->focusedChildWin == me) return;

//printf("setFocus4 %s\n", name.c_str());

    /* save focused widget from current window and remove the focus */
    this->parent->removeChildWinFocus();

    /* i am the new focused window */
    this->parent->focusedChildWin = me;

    /* restore focused widget to candidate window */
    this->parent->restoreChildWinFocus();

	// change the z-order of child windows?
	bool staticzorder = false;
	this->parent->getStaticZOrder(staticzorder);
	if (!staticzorder)
		raiseToTop();
}

bool MMSWindow::getFocus() {
	/* check if i am a child window */
	if (!this->parent) {
    	if (windowmanager->getToplevelWindow() == this)
    		return true;
    	else
    		return false;
	}

    /* search me */
    int me = -1;
    for (unsigned int i = 0; i < this->parent->childwins.size(); i++)
        if (this->parent->childwins.at(i).window == this) {
            me = i;
            break;
        }

    /* i have found me within my parents list */
    if (me < 0) return false;

    /* i am the currently focused child window? */
    if ((int)this->parent->focusedChildWin == me)
    	return true;
    else
    	return false;
}

bool MMSWindow::handleNavigationForChildWins(MMSInputEvent *inputevent) {
    MMSWindow *candidate = NULL;
    int cand=-1;

    /* check if I have child windows */
    if (!(this->childwins.size() > this->focusedChildWin))
        return false;

    /* get access to the focused child window */
    MMSWindow *fWin = this->childwins.at(this->focusedChildWin).window;

	if (inputevent->type == MMSINPUTEVENTTYPE_KEYPRESS) {
		/* keyboard inputs */

	    /* check if window names for navigation are set to the focused child window */
		switch (inputevent->key) {
			case MMSKEY_CURSOR_DOWN:
			    candidate = fWin->getNavigateDownWindow();
			    break;
			case MMSKEY_CURSOR_UP:
		        candidate = fWin->getNavigateUpWindow();
			    break;
			case MMSKEY_CURSOR_RIGHT:
		        candidate = fWin->getNavigateRightWindow();
			    break;
			case MMSKEY_CURSOR_LEFT:
		        candidate = fWin->getNavigateLeftWindow();
		        break;

            default:
                break;
		}
	}
	else {
		/* other inputs */

	}

    if (candidate) {
        /* check if candidate has something to focus */
        if (!candidate->getNumberOfFocusableWidgets())
            if (!candidate->getNumberOfFocusableChildWins())
                return false;

        /* i found a new window */
        if (candidate != fWin) {
            /* different from current focus */
            for(unsigned int i = 0; i < this->childwins.size(); i++) {
                if (childwins.at(i).window == candidate) {
                    cand = i;
                    break;
                }
            }
            if (cand < 0)
                return false;

            /* save focused widget from current window and remove the focus */
            removeChildWinFocus();

            /* new focused window */
            this->focusedChildWin = cand;

            /* restore focused widget to candidate window */
            restoreChildWinFocus(inputevent);

            return true;
        }
    }

    return false;
}


void MMSWindow::preCalcNavigation() {

    preCalcNaviLock.lock();

    if (!this->children.empty()) {
        /* for each focusable widget */
        for(unsigned int k = 0; k < this->children.size(); k++) {
            /* simulate that this widget has the focus */
            MMSWidget *fwidget = children.at(k);
            bool 	b;
            string 	s;

            if (!fwidget->getFocusable(b))
                continue;
            if (!b)
            	continue;

            /* for up, down, left right keys */
            for(unsigned int j = 0; j < 4; j++) {
                MMSKeySymbol key = MMSKEY_NULL;
                switch (j) {
                    case 0:
                        key = MMSKEY_CURSOR_UP;
                        break;
                    case 1:
                        key = MMSKEY_CURSOR_DOWN;
                        break;
                    case 2:
                        key = MMSKEY_CURSOR_LEFT;
                        break;
                    case 3:
                        key = MMSKEY_CURSOR_RIGHT;
                        break;
                }

                /* searching for next widget to become the focus */
                MMSFBRectangle fGeom = fwidget->getGeometry();
                MMSWidget *candidate = NULL;
                double dgcode = MAXDGCODE;

                for(unsigned int i = 0; i < this->children.size(); i++) {
                    /* get widget */
                    MMSWidget *widget = children.at(i);

                    /* not for already focused widget */
                    if (i != k) {
                        /* its not the already focused one */
                        if (widget->getFocusable(b))
                        	if (b) {
	                            /* basically it can be focused */
	                            MMSFBRectangle wGeom = widget->getGeometry();
	                            double cand_dgcode = MAXDGCODE;

	                            if (key == MMSKEY_CURSOR_DOWN)
	                                cand_dgcode = calculateDistGradCode_Down(fGeom, wGeom);
	                            else
	                            if (key == MMSKEY_CURSOR_UP)
	                                cand_dgcode = calculateDistGradCode_Up(fGeom, wGeom);
	                            else
	                            if (key == MMSKEY_CURSOR_RIGHT)
	                                cand_dgcode = calculateDistGradCode_Right(fGeom, wGeom);
	                            else
	                            if (key == MMSKEY_CURSOR_LEFT)
	                                cand_dgcode = calculateDistGradCode_Left(fGeom, wGeom);

	                            /* new candidate? */
	                            if (cand_dgcode < dgcode) {
	                                /* yes, make it to my new candidate */
	                                candidate = widget;
	                                dgcode = cand_dgcode;
	                            }
	                        }
                    }
                }

                if (candidate) {
                    /* i found a new widget */
                    if (key == MMSKEY_CURSOR_DOWN) {
                      	if (!fwidget->getNavigateDown(s))
                            fwidget->setNavigateDownWidget(candidate);
						else
							if (s == "")
	                            fwidget->setNavigateDownWidget(candidate);
                    } else
                    if (key == MMSKEY_CURSOR_UP) {
                        if (!fwidget->getNavigateUp(s))
                            fwidget->setNavigateUpWidget(candidate);
                        else
                        	if (s == "")
                        		fwidget->setNavigateUpWidget(candidate);
                    } else
                    if (key == MMSKEY_CURSOR_RIGHT) {
                        if (!fwidget->getNavigateRight(s))
                            fwidget->setNavigateRightWidget(candidate);
                        else
                        	if (s == "")
                        		fwidget->setNavigateRightWidget(candidate);
                    } else
                    if (key == MMSKEY_CURSOR_LEFT) {
                        if (!fwidget->getNavigateLeft(s))
                            fwidget->setNavigateLeftWidget(candidate);
                        else
                        	if (s == "")
                        		fwidget->setNavigateLeftWidget(candidate);
                    }
                }
            }
        }
    }
    else {
        /* no widgets, work for child windows */
        /* for each child window which have focusable widgets or child windows */
        for(unsigned int k = 0; k < this->childwins.size(); k++) {
            /* simulate that this window has the focus */
            MMSWindow *fWin = this->childwins.at(k).window;

            /* only for visible windows */
            if (!fWin->isShown())
                continue;

            /* search for shown parent */
            MMSWindow *p = this->parent;
            while (p) {
                if (!p->parent) {
                    p = NULL;
                    break;
                }
                if (!p->isShown())
                    break;
                p = p->parent;
            }
            if (p)
                continue;

            /* check if i have something to focus */
            if (!fWin->getNumberOfFocusableWidgets())
                if (!fWin->getNumberOfFocusableChildWins())
                    continue;

            /* for up, down, left right keys */
            for(unsigned int j = 0; j < 4; j++) {
                MMSKeySymbol key = MMSKEY_NULL;
                switch (j) {
                    case 0:
                        key = MMSKEY_CURSOR_UP;
                        break;
                    case 1:
                        key = MMSKEY_CURSOR_DOWN;
                        break;
                    case 2:
                        key = MMSKEY_CURSOR_LEFT;
                        break;
                    case 3:
                        key = MMSKEY_CURSOR_RIGHT;
                        break;
                }


                /* searching for child window to become the focus */
                MMSFBRectangle fGeom;
                fGeom.x = fWin->geom.x;
                fGeom.y = fWin->geom.y;
                fGeom.w = fWin->geom.w;
                fGeom.h = fWin->geom.h;
                MMSWindow *candidate = NULL;
                double dgcode = MAXDGCODE;

                for(unsigned int i = 0; i < this->childwins.size(); i++) {
                    /* get window */
                    MMSWindow *window = childwins.at(i).window;

                    /* only for visible windows */
                    if (!window->isShown())
                        continue;

                    /* not for already focused window */
                    if (i != k) {
                        /* its not the already focused one */
                        int fwd = window->getNumberOfFocusableWidgets();
                        int fwn = window->getNumberOfFocusableChildWins();
                        if ((fwd>0)||(fwn>0)) {
                            /* basically it can be focused */
                            MMSFBRectangle wGeom;
                            wGeom.x = window->geom.x;
                            wGeom.y = window->geom.y;
                            wGeom.w = window->geom.w;
                            wGeom.h = window->geom.h;
                            double cand_dgcode = MAXDGCODE;

                            if (key == MMSKEY_CURSOR_DOWN)
                                cand_dgcode = calculateDistGradCode_Down(fGeom, wGeom);
                            else
                            if (key == MMSKEY_CURSOR_UP)
                                cand_dgcode = calculateDistGradCode_Up(fGeom, wGeom);
                            else
                            if (key == MMSKEY_CURSOR_RIGHT)
                                cand_dgcode = calculateDistGradCode_Right(fGeom, wGeom);
                            else
                            if (key == MMSKEY_CURSOR_LEFT)
                                cand_dgcode = calculateDistGradCode_Left(fGeom, wGeom);

                            /* new candidate? */
                            if (cand_dgcode < dgcode) {
                                /* yes, make it to my new candidate */
                                candidate = window;
                                dgcode = cand_dgcode;

                                if (fwn>0)
                                    window->preCalcNavigation();
                            }
                        }
                    }
                }

                /* i found a new window */
                if (key == MMSKEY_CURSOR_DOWN) {
                	string s;
                	if (!fWin->getNavigateDown(s)) s = "";
                    if (s == "")
                        fWin->setNavigateDownWindow(candidate);
                } else
                if (key == MMSKEY_CURSOR_UP) {
                	string s;
                	if (!fWin->getNavigateUp(s)) s = "";
                    if (s == "")
                        fWin->setNavigateUpWindow(candidate);
                } else
                if (key == MMSKEY_CURSOR_RIGHT) {
                	string s;
                	if (!fWin->getNavigateRight(s)) s = "";
                    if (s == "")
                        fWin->setNavigateRightWindow(candidate);
                } else
                if (key == MMSKEY_CURSOR_LEFT) {
                	string s;
                	if (!fWin->getNavigateLeft(s)) s = "";
                    if (s == "")
                        fWin->setNavigateLeftWindow(candidate);
                }
            }
        }
    }

    preCalcNaviLock.unlock();
}


bool MMSWindow::handleInput(vector<MMSInputEvent> *inputeventset) {
    bool ret = true;

//printf("YYY: input to window %s\n", name.c_str());

    if (this->shown == false)
        return false;


    for(unsigned int i=0; i < inputeventset->size();i++) {

    	if (inputeventset->at(i).type == MMSINPUTEVENTTYPE_KEYPRESS) {
    		/* keyboard inputs */
	        try {
	            if(this->focusedwidget != NULL) {
//printf("YYY: input3 to window %s\n", name.c_str());
	                this->focusedwidget->handleInput(&(inputeventset->at(i)));

	                switch(inputeventset->at(i).key) {
	                    case MMSKEY_CURSOR_DOWN:
	                    case MMSKEY_CURSOR_LEFT:
	                    case MMSKEY_CURSOR_RIGHT:
	                    case MMSKEY_CURSOR_UP:
	                        /* set the arrow widgets */
	                        switchArrowWidgets();

                        default:
                            break;
	                }

	                return true;
	            }
	            else
	            if (this->childwins.size() > this->focusedChildWin) {
//printf("YYY: input2 to window %s, this->focusedChildWin = %d, %s\n", name.c_str(), this->focusedChildWin, this->childwins.at(this->focusedChildWin).window->name.c_str());
//printf("YYY: input3 to window %s, this->focusedChildWin = %d, %s %s\n", name.c_str(), this->focusedChildWin, this->childwins.at(this->focusedChildWin).window->name.c_str(), this->childwins.at(1).window->name.c_str());
	                /* get the focus to my focused child window */
//	                logger.writeLog("try to execute input on childwindow");
	                if (!this->childwins.at(this->focusedChildWin).window->handleInput(inputeventset)) {
	                    /* childwin cannot navigate further, so try to find the next childwin */
						bool modal = false;
        				((MMSChildWindow*)this->childwins.at(this->focusedChildWin).window)->getModal(modal);
        				if (!modal)
        					// currently focused child window is NOT marked as modal, so try to change the focus
        					this->handleNavigationForChildWins(&(inputeventset->at(i)));

	                    return false;
	                }

	                /* set the arrow widgets */
	                switchArrowWidgets();

	                return true;
	            }
	            else {
	                throw new MMSWidgetError(1,"navigate");
	            }

	        } catch (MMSWidgetError *err) {
	            if(err->getCode() == 1) {
	                /* test if navigation must be done */
	                ret = true;
	                switch(inputeventset->at(i).key) {
	                    /* handle navigation */
	                    case MMSKEY_CURSOR_DOWN:
	                    case MMSKEY_CURSOR_LEFT:
	                    case MMSKEY_CURSOR_RIGHT:
	                    case MMSKEY_CURSOR_UP:
//	                        logger.writeLog("widget threw a exception so try to navigate");
	                        ret = this->handleNavigationForWidgets(&(inputeventset->at(i)));

	                        /* set the arrow widgets */
	                        switchArrowWidgets();

	                        break;
	                    default:
	                        /* input is no navigation */
	                        ret = false;
	                        break;
	                }

	                /* call handle input callback */
	                onHandleInput->emit(this, &(inputeventset->at(i)));
	            }
	        }
    	}
    	else
    	if (inputeventset->at(i).type == MMSINPUTEVENTTYPE_KEYRELEASE) {
            /* call handle input callback */
            onHandleInput->emit(this, &(inputeventset->at(i)));
    	}
    	else
    	if (inputeventset->at(i).type == MMSINPUTEVENTTYPE_BUTTONPRESS) {
    		/* button pressed */
	        try {
	            if (this->children.size()) {
	            	/* searching for the right widget to get the focus */
	            	int posx = inputeventset->at(i).posx;
	            	int posy = inputeventset->at(i).posy;
	            	bool b;
	            	for (unsigned int j = 0; j < this->children.size(); j++) {
	            		MMSWidget *w = this->children.at(j);
	            		if (!w->getClickable(b))
	            			continue;
	            		if (!b)
	            			continue;
	            		if (!w->isActivated())
	            			continue;

	            		MMSFBRectangle rect = this->children.at(j)->getGeometry();
	            		if ((posx >= rect.x)&&(posy >= rect.y)
	            		  &&(posx < rect.x + rect.w)&&(posy < rect.y + rect.h)) {
	            			// this is the widget under the pointer
		            		w->getFocusable(b);
	            			if ((b)&&(w != this->focusedwidget)) {
	            				// set focus to this widget
	            				DEBUGMSG("MMSGUI", "try to change focus");
	            				setFocusedWidget(w, true, true);
	            			}

	            			DEBUGMSG("MMSGUI", "try to execute input on widget");
	            	        this->buttonpress_widget = w;
	            	        this->buttonpress_widget->handleInput(&(inputeventset->at(i)));

	                        /* set the arrow widgets */
	                        switchArrowWidgets();

	    	                return true;
	            		}
	            	}

	            	/* no widget found */
        	        this->buttonpress_widget = NULL;
	                throw new MMSWidgetError(1,"no focusable widget found");
	            }
	            else
	            if (this->childwins.size() > this->focusedChildWin) {
					bool modal = false;
					if (this->childwins.at(this->focusedChildWin).window->isShown())
						this->childwins.at(this->focusedChildWin).window->getModal(modal);
       				if (!modal) {
						/* searching for the right childwin to get the focus */
						int posx = inputeventset->at(i).posx;
						int posy = inputeventset->at(i).posy;
	//	            	for (unsigned int j = 0; j < this->childwins.size(); j++) {
						for (int j = (int)this->childwins.size()-1; j >= 0; j--) {
							if (!this->childwins.at(j).window->isShown())
								continue;
	/*	            		if (!this->childwins.at(j).window->getNumberOfFocusableWidgets())
								continue;*/
							MMSFBRectangle rect = this->childwins.at(j).window->getGeometry();
							if ((posx >= rect.x)&&(posy >= rect.y)
							  &&(posx < rect.x + rect.w)&&(posy < rect.y + rect.h)) {
								/* this is the childwin under the pointer */
								if (!this->childwins.at(j).window->getFocus()) {
	//								bool modal = false;
	//		        				((MMSChildWindow*)this->childwins.at(this->focusedChildWin).window)->getModal(modal);
	//	            				if (modal)
										// currently focused child window is marked as modal, so do not change the focus
	//									continue;

									if (this->childwins.at(j).window->getNumberOfFocusableWidgets(true)>0)
									{
										/* set focus to this childwin */
										DEBUGMSG("MMSGUI", "try to change focus");
										this->childwins.at(j).window->setFocus();
									}
								}

								/* normalize the pointer position */
								for (unsigned int k = 0; k < inputeventset->size(); k++) {
									inputeventset->at(k).posx-=rect.x;
									inputeventset->at(k).posy-=rect.y;
								}

								DEBUGMSG("MMSGUI", "try to execute input on childwin");
								this->buttonpress_childwin = this->childwins.at(j).window;
								this->childwins.at(j).window->handleInput(inputeventset);

								/* set the arrow widgets */
								switchArrowWidgets();

								return true;
							}
						}

						/* no childwin found */
						this->buttonpress_childwin = NULL;
						throw new MMSWidgetError(1,"no focusable childwin found");
       				}
       				else {
       					// modal window is active
						//int posx = inputeventset->at(i).posx;
						//int posy = inputeventset->at(i).posy;
						MMSFBRectangle rect = this->childwins.at(this->focusedChildWin).window->getGeometry();

						// normalize the pointer position
						for (unsigned int k = 0; k < inputeventset->size(); k++) {
							inputeventset->at(k).posx-=rect.x;
							inputeventset->at(k).posy-=rect.y;
						}

						DEBUGMSG("MMSGUI", "try to execute input on childwin");
						this->buttonpress_childwin = this->childwins.at(this->focusedChildWin).window;
						this->childwins.at(this->focusedChildWin).window->handleInput(inputeventset);

						/* set the arrow widgets */
						switchArrowWidgets();

						return true;
       				}
	            }
	            else {
	                throw new MMSWidgetError(1,"navigate, buttonpress");
	            }

	        } catch (MMSWidgetError *err) {
	            if(err->getCode() == 1) {
	                /* test if navigation must be done */
	                ret = true;

	                /* call handle input callback */
	                onHandleInput->emit(this, &(inputeventset->at(i)));
	            }
	        }
    	}
    	else
    	if   ((inputeventset->at(i).type == MMSINPUTEVENTTYPE_BUTTONRELEASE)
    		||(inputeventset->at(i).type == MMSINPUTEVENTTYPE_AXISMOTION)) {
    		/* button released */
    		try {
	            if (this->children.size()) {
	            	// window with widgets
	            	if (this->buttonpress_widget) {
	            		DEBUGMSG("MMSGUI", "try to execute input on widget");
            	        this->buttonpress_widget->handleInput(&(inputeventset->at(i)));

            	        if (inputeventset->at(i).type == MMSINPUTEVENTTYPE_BUTTONRELEASE)
            	        	this->buttonpress_widget = NULL;

                        /* set the arrow widgets */
                        switchArrowWidgets();

                        return true;
	            	}
	            	else
	            		return false;
	            }
	            else
	            if (this->childwins.size() > this->focusedChildWin) {
	            	// window with childwindows
	            	if (this->buttonpress_childwin) {
              			/* normalize the pointer position */
	            		MMSFBRectangle rect = this->buttonpress_childwin->getGeometry();
            			for (unsigned int k = 0; k < inputeventset->size(); k++) {
            				inputeventset->at(k).posx-=rect.x;
            				inputeventset->at(k).posy-=rect.y;
            			}

            			DEBUGMSG("MMSGUI", "try to execute input on childwin");
            	        bool rc = this->buttonpress_childwin->handleInput(inputeventset);

            	        if (inputeventset->at(i).type == MMSINPUTEVENTTYPE_BUTTONRELEASE)
            	        	this->buttonpress_childwin = NULL;

                        /* set the arrow widgets */
                        switchArrowWidgets();

                        return rc;
	            	}
	            	else
	            		return false;
	            }
	            else {
//	                throw new MMSWidgetError(1,"navigate, buttonrelease");

					// window without widgets and childwindows, e.g. video/flash windows

					// call handle input callback
					return onHandleInput->emit(this, &(inputeventset->at(i)));
	            }

	        } catch (MMSWidgetError *err) {
	            if(err->getCode() == 1) {
	                /* test if navigation must be done */
	                ret = true;

	                /* call handle input callback */
	                onHandleInput->emit(this, &(inputeventset->at(i)));
	            }
	        }
    	}
    }

    return ret;
}

MMSFBRectangle MMSWindow::getGeometry() {
	return this->geom;
}

MMSFBRectangle MMSWindow::getRealGeometry() {
	/* childwin? */
	if (!this->parent)
		return this->geom;

	/* yes */
	MMSFBRectangle r1,r2;
	r1 = this->geom;
	r2 = this->parent->getRealGeometry();
	r1.x+=r2.x;
	r1.y+=r2.y;
	return r1;
}


MMSWidget *MMSWindow::getFocusedWidget() {
    return this->focusedwidget;
}

int MMSWindow::getNumberOfFocusableWidgets(bool cw) {
    int		cnt = 0;
    bool 	b;

    if (!children.empty()) {
		for (unsigned int i = 0; i < children.size(); i++)
			if (children.at(i)->getFocusable(b))
				if (b)
					cnt++;
    }
    else {
    	if (cw) {
			for (unsigned int i = 0; i < childwins.size(); i++)
				cnt += childwins.at(i).window->getNumberOfFocusableWidgets(cw);
    	}
    }

    return cnt;
}

int MMSWindow::getNumberOfFocusableChildWins() {
    int cnt = 0;

    for (unsigned int i = 0; i < childwins.size(); i++)
        if (childwins.at(i).window->getNumberOfFocusableWidgets()>0)
            cnt++;
        else
            cnt+=childwins.at(i).window->getNumberOfFocusableChildWins();

    return cnt;
}


void MMSWindow::setWindowManager(IMMSWindowManager *wm) {
    if (this->windowmanager != wm) {
    	DEBUGMSG("MMSGUI", "windowmanager != wm");
        /* set new window manager */
        if (this->windowmanager != NULL) {
        	DEBUGMSG("MMSGUI", "windowmanager != NULL");
            this->windowmanager = wm;
            /* and add the window to it */
            if (this->windowmanager) {
            	DEBUGMSG("MMSGUI", "windowmanager->addWindow");
                this->windowmanager->addWindow(this);
            }
        }
        else {
            this->windowmanager = wm;
        	DEBUGMSG("MMSGUI", "resize");
            this->resize();
        }
    }
}

bool MMSWindow::isShown(bool checkparents) {
	if (!this->shown) return false;
	if (this->buffered_shown) return false;
	if ((checkparents)&&(this->parent)) return this->parent->isShown(true);
	return true;
}



bool MMSWindow::willHide() {
    return this->willhide;
}

void MMSWindow::instantShow() {
    unsigned int opacity;
    if (!getOpacity(opacity)) opacity = 255;

    if (!parent) {
    	/* normal window */
	    if (this->window) {
	        this->window->show();
	        this->window->setOpacity(opacity);
	    }
    }
	else {
		/* child window */
        this->parent->setChildWindowOpacity(this, opacity);
	}
}

void MMSWindow::instantHide() {
	if (!parent) {
    	// normal window
	    if (this->windowmanager)
	        this->windowmanager->removeWindowFromToplevel(this);
    	if (isShown())
    	    if (this->window) {
		        this->window->setOpacity(0);
		        this->window->hide();
	    	}
	}
	else {
		// child window
    	if (isShown()) {
			removeFocusFromChildWindow();
		    this->parent->setChildWindowOpacity(this, 0);
    	}
	}
}



MMSWidget* MMSWindow::searchForWidget(string name) {
    MMSWidget *widget;

	if (name=="")
	    return NULL;

    /* for all child windows */
    for (unsigned int i = 0; i < childwins.size(); i++)
        if ((widget = childwins.at(i).window->searchForWidget(name)))
            return widget;

    /* for my own children (widgets) */
    for (unsigned int i = 0; i < children.size(); i++)
        if (children.at(i)->getName() == name)
            return children.at(i);

    return NULL;
}

MMSWidget* MMSWindow::searchForWidgetType(MMSWIDGETTYPE type) {
    MMSWidget *widget;

    /* for all child windows */
    for (unsigned int i = 0; i < childwins.size(); i++)
        if ((widget = childwins.at(i).window->searchForWidgetType(type)))
            return widget;

    /* first, my own children */
    for (unsigned int i = 0; i < children.size(); i++)
        if (children.at(i)->getType() == type)
            return children.at(i);

    /* second, call search method of my children */
    for (unsigned int i = 0; i < children.size(); i++)
        if ((widget = children.at(i)->searchForWidgetType(type)))
            return widget;

    return NULL;
}

MMSWidget* MMSWindow::operator[](string name) {
    MMSWidget *widget;

    if ((widget = searchForWidget(name)))
        return widget;
    throw new MMSWidgetError(1, "widget " + name + " not found");
}


MMSWindow *MMSWindow::getNavigateUpWindow() {
    return navigateUpWindow;
}

MMSWindow *MMSWindow::getNavigateDownWindow() {
    return navigateDownWindow;
}

MMSWindow *MMSWindow::getNavigateLeftWindow() {
    return navigateLeftWindow;
}

MMSWindow *MMSWindow::getNavigateRightWindow() {
    return navigateRightWindow;
}

void MMSWindow::setNavigateUpWindow(MMSWindow *upWindow) {
    navigateUpWindow = upWindow;
}

void MMSWindow::setNavigateDownWindow(MMSWindow *downWindow) {
    navigateDownWindow = downWindow;
}

void MMSWindow::setNavigateRightWindow(MMSWindow *rightWindow) {
    navigateRightWindow = rightWindow;
}

void MMSWindow::setNavigateLeftWindow(MMSWindow *leftWindow) {
    navigateLeftWindow = leftWindow;
}


void MMSWindow::targetLangChanged(MMS_LANGUAGE_TYPE lang, bool refresh) {
    // for all child windows
    for (unsigned int i = 0; i < childwins.size(); i++)
        childwins.at(i).window->targetLangChanged(lang, false);

    // for my own children (widgets)
    for (unsigned int i = 0; i < children.size(); i++)
        switch (children.at(i)->getType()) {
        case MMSWIDGETTYPE_LABEL:
        	((MMSLabelWidget *)children.at(i))->targetLangChanged(lang);
        	break;
        case MMSWIDGETTYPE_TEXTBOX:
        	((MMSTextBoxWidget *)children.at(i))->targetLangChanged(lang);
        	break;
        default:
        	break;
        }

    // refresh it
    if (refresh)
    	this->refresh();
}


/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETWINDOW(x,y) \
    if (this->myWindowClass.is##x()) return myWindowClass.get##x(y); \
    else if ((windowClass)&&(windowClass->is##x())) return windowClass->get##x(y); \
    else return baseWindowClass->get##x(y);


bool MMSWindow::getAlignment(MMSALIGNMENT &alignment) {
    GETWINDOW(Alignment, alignment);
}

bool MMSWindow::getDx(string &dx) {
    GETWINDOW(Dx, dx);
}

int MMSWindow::getDxPix() {
    return this->dxpix;
}

bool MMSWindow::getDy(string &dy) {
    GETWINDOW(Dy, dy);
}

int MMSWindow::getDyPix() {
    return this->dypix;
}

bool MMSWindow::getWidth(string &width) {
    GETWINDOW(Width, width);
}

bool MMSWindow::getHeight(string &height) {
    GETWINDOW(Height, height);
}

bool MMSWindow::getBgColor(MMSFBColor &bgcolor) {
    GETWINDOW(BgColor, bgcolor);
}

bool MMSWindow::getBgImagePath(string &bgimagepath) {
    GETWINDOW(BgImagePath, bgimagepath);
}

bool MMSWindow::getBgImageName(string &bgimagename) {
    GETWINDOW(BgImageName, bgimagename);
}

bool MMSWindow::getOpacity(unsigned int &opacity) {
    GETWINDOW(Opacity, opacity);
}

bool MMSWindow::getFadeIn(bool &fadein) {
    GETWINDOW(FadeIn, fadein);
}

bool MMSWindow::getFadeOut(bool &fadeout) {
    GETWINDOW(FadeOut, fadeout);
}

bool MMSWindow::getDebug(bool &debug) {
    GETWINDOW(Debug, debug);
}

bool MMSWindow::getMargin(unsigned int &margin) {
    GETWINDOW(Margin, margin);
}

bool MMSWindow::getUpArrow(string &uparrow) {
    GETWINDOW(UpArrow, uparrow);
}

bool MMSWindow::getDownArrow(string &downarrow) {
    GETWINDOW(DownArrow, downarrow);
}

bool MMSWindow::getLeftArrow(string &leftarrow) {
    GETWINDOW(LeftArrow, leftarrow);
}

bool MMSWindow::getRightArrow(string &rightarrow) {
    GETWINDOW(RightArrow, rightarrow);
}

bool MMSWindow::getNavigateUp(string &navigateup) {
    GETWINDOW(NavigateUp, navigateup);
}

bool MMSWindow::getNavigateDown(string &navigatedown) {
    GETWINDOW(NavigateDown, navigatedown);
}

bool MMSWindow::getNavigateLeft(string &navigateleft) {
    GETWINDOW(NavigateLeft, navigateleft);
}

bool MMSWindow::getNavigateRight(string &navigateright) {
    GETWINDOW(NavigateRight, navigateright);
}

bool MMSWindow::getOwnSurface(bool &ownsurface) {
    GETWINDOW(OwnSurface, ownsurface);
}

bool MMSWindow::getMoveIn(MMSDIRECTION &movein) {
    GETWINDOW(MoveIn, movein);
}

bool MMSWindow::getMoveOut(MMSDIRECTION &moveout) {
    GETWINDOW(MoveOut, moveout);
}

bool MMSWindow::getModal(bool &modal) {
    GETWINDOW(Modal, modal);
}

bool MMSWindow::getStaticZOrder(bool &staticzorder) {
    GETWINDOW(StaticZOrder, staticzorder);
}


#define GETBORDER(x,y) \
    if (this->myWindowClass.border.is##x()) return myWindowClass.border.get##x(y); \
    else if ((windowClass)&&(windowClass->border.is##x())) return windowClass->border.get##x(y); \
    else return baseWindowClass->border.get##x(y);

#define GETBORDER_IMAGES(x,p,y) \
    if (this->myWindowClass.border.is##x()) return myWindowClass.border.get##x(p,y); \
    else if ((windowClass)&&(windowClass->border.is##x())) return windowClass->border.get##x(p,y); \
    else return baseWindowClass->border.get##x(p,y);


bool MMSWindow::getBorderColor(MMSFBColor &color) {
    GETBORDER(Color, color);
}

bool MMSWindow::getBorderImagePath(string &imagepath) {
    GETBORDER(ImagePath, imagepath);
}

bool MMSWindow::getBorderImageNames(MMSBORDER_IMAGE_NUM num, string &imagename) {
    GETBORDER_IMAGES(ImageNames, num, imagename);
}

bool MMSWindow::getBorderThickness(unsigned int &thickness) {
    GETBORDER(Thickness, thickness);
}

bool MMSWindow::getBorderMargin(unsigned int &margin) {
    GETBORDER(Margin, margin);
}

bool MMSWindow::getBorderRCorners(bool &rcorners) {
    GETBORDER(RCorners, rcorners);
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSWindow::setAlignment(MMSALIGNMENT alignment, bool refresh, bool resize) {
    myWindowClass.setAlignment(alignment);
    if (resize)
        this->resize();
    if (refresh)
        this->refresh();
}

void MMSWindow::setDx(string dx, bool refresh, bool resize) {
    myWindowClass.setDx(dx);
    if (resize)
        this->resize();
    if (refresh)
        this->refresh();
}

void MMSWindow::setDxPix(int dx, bool refresh, bool resize) {
    string s = iToStr(dx) + "px";
    setDx(s, refresh, resize);
}

void MMSWindow::setDy(string dy, bool refresh, bool resize) {
    myWindowClass.setDy(dy);
    if (resize)
        this->resize();
    if (refresh)
        this->refresh();
}

void MMSWindow::setDyPix(int dy, bool refresh, bool resize) {
    string s = iToStr(dy) + "px";
    setDy(s, refresh, resize);
}

void MMSWindow::setWidth(string width, bool refresh, bool resize) {
    myWindowClass.setWidth(width);
    if (resize)
        this->resize();
    if (refresh)
        this->refresh();
}

void MMSWindow::setHeight(string height, bool refresh, bool resize) {
    myWindowClass.setHeight(height);
    if (resize)
        this->resize();
    if (refresh)
        this->refresh();
}

void MMSWindow::setBgColor(MMSFBColor bgcolor, bool refresh) {
    myWindowClass.setBgColor(bgcolor);
    if (refresh)
        this->refresh();
}

void MMSWindow::setBgImagePath(string bgimagepath, bool load, bool refresh) {
    myWindowClass.setBgImagePath(bgimagepath);
    if (load) {
        im->releaseImage(this->bgimage);
        string path, name;
        if (!getBgImagePath(path)) path = "";
        if (!getBgImageName(name)) name = "";
        this->bgimage = im->getImage(path, name);
    }
    if (refresh)
        this->refresh();
}

void MMSWindow::setBgImageName(string bgimagename, bool load, bool refresh) {
    myWindowClass.setBgImageName(bgimagename);
    if (load) {
        im->releaseImage(this->bgimage);
        string path, name;
        if (!getBgImagePath(path)) path = "";
        if (!getBgImageName(name)) name = "";
        this->bgimage = im->getImage(path, name);
    }
    if (refresh)
        this->refresh();
}

void MMSWindow::setOpacity(unsigned int opacity, bool refresh) {
    myWindowClass.setOpacity(opacity);
    if (refresh) {
        if (!this->parent) {
            if (this->window)
                this->window->setOpacity(opacity);
        }
        else
            this->parent->setChildWindowOpacity(this, opacity);
    }
}

void MMSWindow::setFadeIn(bool fadein) {
    myWindowClass.setFadeIn(fadein);
}

void MMSWindow::setFadeOut(bool fadeout) {
    myWindowClass.setFadeOut(fadeout);
}

void MMSWindow::setDebug(bool debug, bool refresh) {
    myWindowClass.setDebug(debug);
    if (refresh)
        this->refresh();
}

void MMSWindow::setMargin(unsigned int margin, bool refresh, bool resize) {
    myWindowClass.setMargin(margin);
    if (resize)
        this->resize();
    if (refresh)
        this->refresh();
}

void MMSWindow::setUpArrow(string uparrow, bool refresh) {
    myWindowClass.setUpArrow(uparrow);
    upArrowWidget = NULL;
    if (refresh)
        this->refresh();
}

void MMSWindow::setDownArrow(string downarrow, bool refresh) {
    myWindowClass.setDownArrow(downarrow);
    downArrowWidget = NULL;
    if (refresh)
        this->refresh();
}

void MMSWindow::setLeftArrow(string leftarrow, bool refresh) {
    myWindowClass.setLeftArrow(leftarrow);
    leftArrowWidget = NULL;
    if (refresh)
        this->refresh();
}

void MMSWindow::setRightArrow(string rightarrow, bool refresh) {
    myWindowClass.setRightArrow(rightarrow);
    rightArrowWidget = NULL;
    if (refresh)
        this->refresh();
}

void MMSWindow::setNavigateUp(string navigateup) {
    myWindowClass.setNavigateUp(navigateup);
    this->navigateUpWindow = NULL;
    if ((this->parent)&&(navigateup!=""))
        this->navigateUpWindow = this->parent->searchForWindow(navigateup);
}

void MMSWindow::setNavigateDown(string navigatedown) {
    myWindowClass.setNavigateDown(navigatedown);
    this->navigateDownWindow = NULL;
    if ((this->parent)&&(navigatedown!=""))
        this->navigateDownWindow = this->parent->searchForWindow(navigatedown);
}

void MMSWindow::setNavigateLeft(string navigateleft) {
    myWindowClass.setNavigateLeft(navigateleft);
    this->navigateLeftWindow = NULL;
    if ((this->parent)&&(navigateleft!=""))
        this->navigateLeftWindow = this->parent->searchForWindow(navigateleft);
}

void MMSWindow::setNavigateRight(string navigateright) {
    myWindowClass.setNavigateRight(navigateright);
    this->navigateRightWindow = NULL;
    if ((this->parent)&&(navigateright!=""))
        this->navigateRightWindow = this->parent->searchForWindow(navigateright);
}

void MMSWindow::setOwnSurface(bool ownsurface) {
    myWindowClass.setOwnSurface(ownsurface);
}

void MMSWindow::setMoveIn(MMSDIRECTION movein) {
    myWindowClass.setMoveIn(movein);
}

void MMSWindow::setMoveOut(MMSDIRECTION moveout) {
    myWindowClass.setMoveOut(moveout);
}

void MMSWindow::setModal(bool modal) {
    myWindowClass.setModal(modal);
}

void MMSWindow::setStaticZOrder(bool staticzorder) {
    myWindowClass.setStaticZOrder(staticzorder);
}


void MMSWindow::setBorderColor(MMSFBColor color, bool refresh) {
    myWindowClass.border.setColor(color);
    if (refresh)
        this->refresh();
}

void MMSWindow::setBorderImagePath(string imagepath, bool load, bool refresh) {
    myWindowClass.border.setImagePath(imagepath);
    if (load) {
        string path, name;
        if (!getBorderImagePath(path)) path = "";
        for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++) {
            im->releaseImage(this->borderimages[i]);
            if (!getBorderImageNames((MMSBORDER_IMAGE_NUM)i, name)) name = "";
            this->borderimages[i] = im->getImage(path, name);
        }
    }
    if (refresh)
        this->refresh();
}

void MMSWindow::setBorderImageNames(string imagename_1, string imagename_2, string imagename_3, string imagename_4,
                                    string imagename_5, string imagename_6, string imagename_7, string imagename_8,
                                    bool load, bool refresh) {
    myWindowClass.border.setImageNames(imagename_1, imagename_2, imagename_3, imagename_4,
                                       imagename_5, imagename_6, imagename_7, imagename_8);
    if (load) {
        string path, name;
        if (!getBorderImagePath(path)) path = "";
        for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++) {
            im->releaseImage(this->borderimages[i]);
            if (!getBorderImageNames((MMSBORDER_IMAGE_NUM)i, name)) name = "";
            this->borderimages[i] = im->getImage(path, name);
        }
    }
    if (refresh)
        this->refresh();
}

void MMSWindow::setBorderThickness(unsigned int thickness, bool refresh, bool resize) {
    myWindowClass.border.setThickness(thickness);
    if (resize)
        this->resize();
    if (refresh)
        this->refresh();
}

void MMSWindow::setBorderMargin(unsigned int margin, bool refresh, bool resize) {
    myWindowClass.border.setMargin(margin);
    if (resize)
        this->resize();
    if (refresh)
        this->refresh();
}

void MMSWindow::setBorderRCorners(bool rcorners, bool refresh) {
    myWindowClass.border.setRCorners(rcorners);
    if (refresh)
        this->refresh();
}

void MMSWindow::updateFromThemeClass(MMSWindowClass *themeClass) {

	MMSALIGNMENT	a;
	bool 			b;
	MMSFBColor		c;
	MMSDIRECTION	d;
	string 			s;
	unsigned int	u;

	if (themeClass->getAlignment(a))
        setAlignment(a, false, false);
    if (themeClass->getDx(s))
        setDx(s, false, false);
    if (themeClass->getDy(s))
        setDy(s, false, false);
    if (themeClass->getWidth(s))
        setWidth(s, false, false);
    if (themeClass->getHeight(s))
        setHeight(s, false, false);
    if (themeClass->getBgColor(c))
        setBgColor(c, false);
    if (themeClass->getBgImagePath(s))
        setBgImagePath(s, true, false);
    if (themeClass->getBgImageName(s))
        setBgImageName(s, true, false);
    if (themeClass->getOpacity(u))
        setOpacity(u, false);
    if (themeClass->getFadeIn(b))
        setFadeIn(b);
    if (themeClass->getFadeOut(b))
        setFadeOut(b);
    if (themeClass->getDebug(b))
        setDebug(b, false);
    if (themeClass->getMargin(u))
        setMargin(u, false, false);
    if (themeClass->getUpArrow(s))
        setUpArrow(s, false);
    if (themeClass->getDownArrow(s))
        setDownArrow(s, false);
    if (themeClass->getLeftArrow(s))
        setLeftArrow(s, false);
    if (themeClass->getRightArrow(s))
        setRightArrow(s, false);
    if (themeClass->getNavigateUp(s))
        setNavigateUp(s);
    if (themeClass->getNavigateDown(s))
        setNavigateDown(s);
    if (themeClass->getNavigateLeft(s))
        setNavigateLeft(s);
    if (themeClass->getNavigateRight(s))
        setNavigateRight(s);
    if (themeClass->getOwnSurface(b))
        setOwnSurface(b);
    if (themeClass->getMoveIn(d))
        setMoveIn(d);
    if (themeClass->getMoveOut(d))
        setMoveOut(d);
	if (themeClass->getModal(b))
        setModal(b);
	if (themeClass->getStaticZOrder(b))
        setStaticZOrder(b);
    if (themeClass->border.getColor(c))
        setBorderColor(c, false);
    if (themeClass->border.getImagePath(s))
        setBorderImagePath(s, true, false);
    if (themeClass->border.isImageNames()) {
    	string s[8];
    	themeClass->border.getImageNames(MMSBORDER_IMAGE_NUM_TOP_LEFT, s[0]);
    	themeClass->border.getImageNames(MMSBORDER_IMAGE_NUM_TOP, s[1]);
    	themeClass->border.getImageNames(MMSBORDER_IMAGE_NUM_TOP_RIGHT, s[2]);
    	themeClass->border.getImageNames(MMSBORDER_IMAGE_NUM_RIGHT, s[3]);
    	themeClass->border.getImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_RIGHT, s[4]);
    	themeClass->border.getImageNames(MMSBORDER_IMAGE_NUM_BOTTOM, s[5]);
    	themeClass->border.getImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_LEFT, s[6]);
    	themeClass->border.getImageNames(MMSBORDER_IMAGE_NUM_LEFT, s[7]);
        setBorderImageNames(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], true, false);
    }
    if (themeClass->border.getThickness(u))
        setBorderThickness(u, false, false);
    if (themeClass->border.getMargin(u))
        setBorderMargin(u, false, false);
    if (themeClass->border.getRCorners(b))
        setBorderRCorners(b, false);

    /* resize window and refresh */
    resize();
    refresh();
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
