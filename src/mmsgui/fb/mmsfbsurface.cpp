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

#include "mmsgui/fb/mmsfbsurface.h"
#include "mmsgui/fb/mmsfb.h"
#include "mmsgui/fb/mmsfbsurfacemanager.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifdef __HAVE_XLIB__
#include <ft2build.h>
#include FT_GLYPH_H
#endif


#ifdef  __HAVE_DIRECTFB__
D_DEBUG_DOMAIN( MMS_Surface, "MMS/Surface", "MMS FB Surface" );
#endif

// static variables
bool MMSFBSurface::extendedaccel								= false;
MMSFBSurfaceAllocMethod MMSFBSurface::allocmethod				= MMSFBSurfaceAllocMethod_malloc;

#define INITCHECK  if((!mmsfb->isInitialized())||(!this->llsurface)){MMSFB_SetError(0,"MMSFBSurface is not initialized");return false;}

#define CLIPSUBSURFACE \
	MMSFBRegion reg, tmp; \
	bool tmpset; \
	if (clipSubSurface(&reg, false, &tmp, &tmpset)) {

#define UNCLIPSUBSURFACE \
	clipSubSurface(NULL, false, &tmp, &tmpset); }

#define SETSUBSURFACE_DRAWINGFLAGS \
	MMSFBColor ccc = this->config.color; \
	this->llsurface->SetColor(this->llsurface, ccc.r, ccc.g, ccc.b, ccc.a); \
	this->llsurface->SetDrawingFlags(this->llsurface, getDFBSurfaceDrawingFlagsFromMMSFBDrawingFlags(this->config.drawingflags));

#define RESETSUBSURFACE_DRAWINGFLAGS \
    ccc = this->root_parent->config.color; \
    this->llsurface->SetColor(this->llsurface, ccc.r, ccc.g, ccc.b, ccc.a); \
    this->llsurface->SetDrawingFlags(this->llsurface, getDFBSurfaceDrawingFlagsFromMMSFBDrawingFlags(this->root_parent->config.drawingflags));

#define SETSUBSURFACE_BLITTINGFLAGS \
	MMSFBColor ccc = this->config.color; \
	this->llsurface->SetColor(this->llsurface, ccc.r, ccc.g, ccc.b, ccc.a); \
	this->llsurface->SetBlittingFlags(this->llsurface, getDFBSurfaceBlittingFlagsFromMMSFBBlittingFlags(this->config.blittingflags));

#define RESETSUBSURFACE_BLITTINGFLAGS \
    ccc = this->root_parent->config.color; \
    this->llsurface->SetColor(this->llsurface, ccc.r, ccc.g, ccc.b, ccc.a); \
    this->llsurface->SetBlittingFlags(this->llsurface, getDFBSurfaceBlittingFlagsFromMMSFBBlittingFlags(this->root_parent->config.blittingflags));


/*
#define MMSFBSurface_RGB2Y(r,g,b) ((((66*r+129*g+25*b+128)>>8)+16) & 0xff)
#define MMSFBSurface_RGB2U(r,g,b) ((((-38*r-74*g+112*b+128)>>8)+128) & 0xff)
#define MMSFBSurface_RGB2V(r,g,b) ((((112*r-94*g-18*b+128)>>8)+128) & 0xff)
*/


//#define MMSFBSurface_PREPARE_YUV2RGB(y,u,v)  y=(int)y-16;u=(int)u-128;v=(int)v-128;
//#define MMSFBSurface_PREPARE_YUVBLEND(y,u,v) y=(int)y-16;u=(int)u-128;v=(int)v-128;
//#define MMSFBSurface_RESET_YUVBLEND(y,u,v) y=(int)y+16;u=(int)u+128;v=(int)v+128;
/*#define MMSFBSurface_YUV2R(y,u,v,r) if ((r=((298*(int)y+409*(int)v+128)>>8)&0xffff)>>8) r=0xff;
#define MMSFBSurface_YUV2G(y,u,v,g) if ((g=((298*(int)y-100*(int)u-208*(int)v+128)>>8)&0xffff)>>8) g=0xff;
#define MMSFBSurface_YUV2B(y,u,v,b) if ((b=((298*(int)y+516*(int)u+128)>>8)&0xffff)>>8) b=0xff;
#define MMSFBSurface_YUV2RX(y,u,v,r) if ((r=(298*(int)y+409*(int)v+128)&0xffff00)>>16) r=0xff00;
#define MMSFBSurface_YUV2GX(y,u,v,g) if ((g=(298*(int)y-100*(int)u-208*(int)v+128)&0xffff00)>>16) g=0xff00;
#define MMSFBSurface_YUV2BX(y,u,v,b) if ((b=(298*(int)y+516*(int)u+128)&0xffff00)>>16) b=0xff00;*/


//#define MMSFB_CONV_YUV2R(y,u,v,r) if ((r=(298*(int)y+409*(int)v+128+0x200)>>8)>>8) r=0xff;
//#define MMSFB_CONV_YUV2G(y,u,v,g) if ((g=(298*(int)y-100*(int)u-208*(int)v+128+0x200)>>8)>>8) g=0xff;
//#define MMSFB_CONV_YUV2B(y,u,v,b) if ((b=(298*(int)y+516*(int)u+128+0x200)>>8)>>8) b=0xff;
/*#define MMSFBSurface_YUV2RX(y,u,v,r) if ((r=(298*(int)y+409*(int)v+128+0x200)&0xffff00)>>16) r=0xff00;
#define MMSFBSurface_YUV2GX(y,u,v,g) if ((g=(298*(int)y-100*(int)u-208*(int)v+128+0x200)&0xffff00)>>16) g=0xff00;
#define MMSFBSurface_YUV2BX(y,u,v,b) if ((b=(298*(int)y+516*(int)u+128+0x200)&0xffff00)>>16) b=0xff00;*/
//#define MMSFB_CONV_YUV2RX(y,u,v,r) if ((r=298*(int)y+409*(int)v+128+0x200)>>16) r=0xff00;
//#define MMSFB_CONV_YUV2GX(y,u,v,g) if ((g=298*(int)y-100*(int)u-208*(int)v+128+0x200)>>16) g=0xff00;
//#define MMSFB_CONV_YUV2BX(y,u,v,b) if ((b=298*(int)y+516*(int)u+128+0x200)>>16) b=0xff00;


/*
int c=(int)D[0].YUV.y-16;
int d=(int)D[0].YUV.u-128;
int e=(int)D[0].YUV.v-128;

if ((D[0].RGB.r=((298*c+409*e+128)>>8)&0xffff)>0xff) D[0].RGB.r=0xff;
if ((D[0].RGB.g=((298*c-100*d-208*e+128)>>8)&0xffff)>0xff) D[0].RGB.g=0xff;
if ((D[0].RGB.b=((298*c+516*d+128)>>8)&0xffff)>0xff) D[0].RGB.b=0xff;
*/


MMSFBSurface::MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, int backbuffer, bool systemonly) {
    // init me
    this->llsurface = NULL;
    this->surface_read_locked = false;
    this->surface_read_lock_cnt = 0;
    this->surface_write_locked = false;
    this->surface_write_lock_cnt = 0;
    this->surface_invert_lock = false;
#ifdef __HAVE_XLIB__
    this->scaler = NULL;
#endif
    this->config.surface_buffer = new MMSFBSurfaceBuffer;
	this->config.surface_buffer->numbuffers = 0;
	this->config.surface_buffer->external_buffer = false;
#ifdef __HAVE_FBDEV__
    this->config.surface_buffer->mmsfbdev_surface = NULL;
#endif
#ifdef __HAVE_XLIB__
    this->config.surface_buffer->x_image[0] = NULL;
    this->config.surface_buffer->xv_image[0] = NULL;
#endif
	this->use_own_alloc = (this->allocmethod != MMSFBSurfaceAllocMethod_dfb);

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
		// create surface description
		DFBSurfaceDescription   surface_desc;
		surface_desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
		surface_desc.width = w;
		surface_desc.height = h;
		surface_desc.pixelformat = getDFBPixelFormatFromMMSFBPixelFormat(pixelformat);

		if (surface_desc.pixelformat==DSPF_UNKNOWN)
			surface_desc.flags = (DFBSurfaceDescriptionFlags)(surface_desc.flags & ~DSDESC_PIXELFORMAT);

		// we use premultiplied surfaces because of alphachannel blitting with better performance
		surface_desc.flags = (DFBSurfaceDescriptionFlags)(surface_desc.flags | DSDESC_CAPS);
		surface_desc.caps = DSCAPS_PREMULTIPLIED;

		switch (backbuffer) {
			case 1: // front + one back buffer (double)
				surface_desc.caps = (DFBSurfaceCapabilities)(surface_desc.caps | DSCAPS_DOUBLE);
				break;
			case 2: // front + two back buffer (triple)
				surface_desc.caps = (DFBSurfaceCapabilities)(surface_desc.caps | DSCAPS_TRIPLE);
				break;
		}

		// surface should stored in system memory only?
		if (systemonly)
			surface_desc.caps = (DFBSurfaceCapabilities)(surface_desc.caps | DSCAPS_SYSTEMONLY);

		// create the surface
		DFBResult			dfbres;
		IDirectFBSurface	*dfbsurface;
		if ((dfbres=mmsfb->dfb->CreateSurface(mmsfb->dfb, &surface_desc, &dfbsurface)) != DFB_OK) {
			this->llsurface = NULL;
			DEBUGMSG("MMSGUI", "ERROR");
			MMSFB_SetError(dfbres, "IDirectFB::CreateSurface(" + iToStr(w) + "x" + iToStr(h) + ") failed");
			return;
		}

		init(dfbsurface, NULL, NULL);
#endif
	}
	else {
		// setup surface attributes
		MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
		this->config.w = sb->sbw = w;
		this->config.h = sb->sbh = h;
		sb->pixelformat = pixelformat;
		sb->alphachannel = isAlphaPixelFormat(sb->pixelformat);
		sb->premultiplied = true;
		sb->backbuffer = backbuffer;
		sb->systemonly = systemonly;

		// allocate my surface buffers
		sb->numbuffers = backbuffer + 1;
		if (sb->numbuffers > MMSFBSurfaceMaxBuffers) {
			sb->numbuffers = MMSFBSurfaceMaxBuffers;
			sb->backbuffer = sb->numbuffers - 1;
		}
		sb->currbuffer_read = 0;
		if (sb->numbuffers > 1)
			// using backbuffer(s)
			sb->currbuffer_write = 1;
		else
			// using only a single buffer for read/write
			sb->currbuffer_write = 0;
		DEBUGMSG("MMSGUI", "start allocating surface buffer");
		memset(sb->buffers, 0, sizeof(sb->buffers));
		for (int i = 0; i < sb->numbuffers; i++) {
			sb->buffers[i].pitch = calcPitch(w);
			int size = calcSize(sb->buffers[i].pitch, sb->sbh);
			DEBUGMSG("MMSGUI", ">allocating surface buffer #%d, %d bytes (pitch=%d, h=%d)", i, size, sb->buffers[i].pitch, sb->sbh);
			sb->buffers[i].ptr = malloc(size);
			sb->buffers[i].hwbuffer = false;

			// few internally pixelformats supports planes and therefore we must init the pointers
			initPlanePointers(&sb->buffers[i], sb->sbh);
		}
		DEBUGMSG("MMSGUI", "allocating surface buffer finished");

		init((void*)1, NULL, NULL);
	}
}


MMSFBSurface::MMSFBSurface(void *llsurface,
	        		       MMSFBSurface *parent,
						   MMSFBRectangle *sub_surface_rect) {
    // init me
#ifdef __HAVE_XLIB__
    this->scaler = NULL;
#endif
	if (llsurface > (void *)1)
		this->use_own_alloc = false;
	else
		this->use_own_alloc = (this->allocmethod != MMSFBSurfaceAllocMethod_dfb);

	if ((!parent)||(!this->use_own_alloc))
    	this->config.surface_buffer = new MMSFBSurfaceBuffer;
    else
    	this->config.surface_buffer = NULL;

   if (this->config.surface_buffer) {
	   memset(this->config.surface_buffer->buffers, 0, sizeof(this->config.surface_buffer->buffers));
	   this->config.surface_buffer->numbuffers = 0;
	   this->config.surface_buffer->external_buffer = false;
   }
#ifdef __HAVE_FBDEV__
    if (this->config.surface_buffer)
    	this->config.surface_buffer->mmsfbdev_surface = NULL;
#endif
#ifdef __HAVE_XLIB__
    if (this->config.surface_buffer) {
    	this->config.surface_buffer->x_image[0] = NULL;
    	this->config.surface_buffer->xv_image[0] = NULL;
    }
#endif

	init(llsurface, parent, sub_surface_rect);
}

MMSFBSurface::MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, int backbuffer, MMSFBSurfacePlanes *planes) {
    // init me
    this->llsurface = NULL;
    this->surface_read_locked = false;
    this->surface_read_lock_cnt = 0;
    this->surface_write_locked = false;
    this->surface_write_lock_cnt = 0;
    this->surface_invert_lock = false;
#ifdef __HAVE_XLIB__
    this->scaler = NULL;
#endif
	this->use_own_alloc = true;

    // setup surface attributes
	this->config.surface_buffer = new MMSFBSurfaceBuffer;
	MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
	this->config.w = sb->sbw = w;
	this->config.h = sb->sbh = h;
	sb->pixelformat = pixelformat;
	sb->alphachannel = isAlphaPixelFormat(sb->pixelformat);
	sb->premultiplied = true;
	sb->backbuffer = backbuffer;
	sb->systemonly = true;

	// set the surface buffer
	memset(sb->buffers, 0, sizeof(sb->buffers));
	sb->numbuffers = backbuffer+1;
	if (sb->numbuffers > MMSFBSurfaceMaxBuffers) sb->numbuffers = MMSFBSurfaceMaxBuffers;
	sb->buffers[0] = *planes;
	if (sb->numbuffers >= 2) {
		if (planes[1].ptr)
			sb->buffers[1] = planes[1];
		else
			sb->numbuffers = 1;
	}
	if (sb->numbuffers >= 3) {
		if (planes[2].ptr)
			sb->buffers[2] = planes[2];
		else
			sb->numbuffers = 2;
	}
	sb->backbuffer = sb->numbuffers - 1;
	sb->currbuffer_read = 0;
	if (sb->numbuffers <= 1)
		sb->currbuffer_write = 0;
	else
		sb->currbuffer_write = 1;
	sb->external_buffer = true;

#ifdef __HAVE_FBDEV__
    this->config.surface_buffer->mmsfbdev_surface = NULL;
#endif
#ifdef __HAVE_XLIB__
	this->config.surface_buffer->x_image[0] = NULL;
	this->config.surface_buffer->xv_image[0] = NULL;
#endif

	init((void*)1, NULL, NULL);
}

MMSFBSurface::MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, MMSFBSurfacePlanes *planes) {
	MMSFBSurface(w, h, pixelformat, 0, planes);
}


#ifdef __HAVE_XLIB__
MMSFBSurface::MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, XvImage *xv_image1, XvImage *xv_image2) {
    // init me
    this->llsurface = NULL;
    this->surface_read_locked = false;
    this->surface_read_lock_cnt = 0;
    this->surface_write_locked = false;
    this->surface_write_lock_cnt = 0;
    this->surface_invert_lock = false;
    this->scaler = NULL;
	this->use_own_alloc = true;

    // setup surface attributes
	this->config.surface_buffer = new MMSFBSurfaceBuffer;
	MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
	this->config.w = sb->sbw = w;
	this->config.h = sb->sbh = h;
	sb->pixelformat = pixelformat;
	sb->alphachannel = isAlphaPixelFormat(sb->pixelformat);
	sb->premultiplied = true;
	sb->backbuffer = 1;
	sb->systemonly = true;

	// set the surface buffer
	memset(sb->buffers, 0, sizeof(sb->buffers));
	sb->numbuffers = 2;
	sb->xv_image[0] = xv_image1;
	sb->buffers[0].ptr = sb->xv_image[0]->data;
	sb->buffers[0].pitch = *(sb->xv_image[0]->pitches);
	sb->buffers[0].hwbuffer = false;
	sb->xv_image[1] = xv_image2;
	sb->buffers[1].ptr = sb->xv_image[1]->data;
	sb->buffers[1].pitch = *(sb->xv_image[1]->pitches);
	sb->buffers[1].hwbuffer = false;
	sb->currbuffer_read = 0;
	sb->currbuffer_write = 1;
	sb->external_buffer = true;

	this->config.surface_buffer->x_image[0] = NULL;

#ifdef __HAVE_FBDEV__
    this->config.surface_buffer->mmsfbdev_surface = NULL;
#endif

	init((void*)1, NULL, NULL);
}

MMSFBSurface::MMSFBSurface(int w, int h, MMSFBSurfacePixelFormat pixelformat, XImage *x_image1, XImage *x_image2, MMSFBSurface *scaler) {
    // init me
    this->llsurface = NULL;
    this->surface_read_locked = false;
    this->surface_read_lock_cnt = 0;
    this->surface_write_locked = false;
    this->surface_write_lock_cnt = 0;
    this->surface_invert_lock = false;
	this->scaler = scaler;
	this->use_own_alloc = true;

    // setup surface attributes
	this->config.surface_buffer = new MMSFBSurfaceBuffer;
	MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
	this->config.w = sb->sbw = w;
	this->config.h = sb->sbh = h;
	sb->pixelformat = pixelformat;
	sb->alphachannel = isAlphaPixelFormat(sb->pixelformat);
	sb->premultiplied = true;
	sb->backbuffer = 0;
	sb->systemonly = true;

	// set the surface buffer
	memset(sb->buffers, 0, sizeof(sb->buffers));
	if (x_image2) {
		// two ximages
		sb->backbuffer = 1;
		sb->numbuffers = 2;
		sb->x_image[0] = x_image1;
		sb->buffers[0].ptr = sb->x_image[0]->data;
		sb->buffers[0].pitch = sb->x_image[0]->bytes_per_line;
		sb->buffers[0].hwbuffer = false;
		sb->x_image[1] = x_image2;
		sb->buffers[1].ptr = sb->x_image[1]->data;
		sb->buffers[1].pitch = sb->x_image[1]->bytes_per_line;
		sb->buffers[1].hwbuffer = false;
		sb->currbuffer_read = 0;
		sb->currbuffer_write = 1;
		sb->external_buffer = true;
	}
	else {
		// only one buffer
		sb->backbuffer = 0;
		sb->numbuffers = 1;
		sb->x_image[0] = x_image1;
		sb->buffers[0].ptr = sb->x_image[0]->data;
		sb->buffers[0].pitch = sb->x_image[0]->bytes_per_line;
		sb->buffers[0].hwbuffer = false;
		sb->x_image[1] = NULL;
		sb->buffers[1].ptr = NULL;
		sb->buffers[1].hwbuffer = false;
		sb->currbuffer_read = 0;
		sb->currbuffer_write = 0;
		sb->external_buffer = true;
	}

	this->config.surface_buffer->xv_image[0] = NULL;

#ifdef __HAVE_FBDEV__
    this->config.surface_buffer->mmsfbdev_surface = NULL;
#endif

	init((void*)1, NULL, NULL);
}
#endif

MMSFBSurface::~MMSFBSurface() {

    if (!mmsfb->isInitialized()) return;

    // release memory - only if not the layer surface
    if (this->llsurface) {
		if (!this->is_sub_surface) {
#ifndef USE_DFB_SUBSURFACE
			// delete all sub surfaces
			deleteSubSurface(NULL);
#endif
			mmsfbsurfacemanager->releaseSurface(this);
		}
		else {
#ifdef USE_DFB_SUBSURFACE
			this->llsurface->Release(this->llsurface);
#endif

			if (this->parent)
				this->parent->deleteSubSurface(this);
		}
	}
}


void MMSFBSurface::init(void *llsurface,
	        		    MMSFBSurface *parent,
						MMSFBRectangle *sub_surface_rect) {
    /* init me */
#ifdef  __HAVE_DIRECTFB__
    this->llsurface = (IDirectFBSurface*)llsurface;
#else
    this->llsurface = llsurface;
#endif
    this->surface_read_locked = false;
    this->surface_read_lock_cnt = 0;
    this->surface_write_locked = false;
    this->surface_write_lock_cnt = 0;
    this->surface_invert_lock = false;
    this->flipflags = MMSFB_FLIP_NONE;
    this->TID = 0;
    this->Lock_cnt = 0;

    /* init subsurface */
    this->parent = parent;
    this->root_parent =  NULL;
    this->sub_surface_xoff = 0;
    this->sub_surface_yoff = 0;
    if (this->parent) {
    	if (!this->parent->is_sub_surface)
			this->root_parent = this->parent;
    	else
    		this->root_parent = this->parent->root_parent;

    	this->is_sub_surface = true;

    	this->sub_surface_rect = *sub_surface_rect;

		this->config.surface_buffer = this->root_parent->config.surface_buffer;

#ifndef USE_DFB_SUBSURFACE

    	this->llsurface = this->root_parent->llsurface;

    	getRealSubSurfacePos();
#endif

    }
    else {
    	this->is_sub_surface = false;
    	this->sub_surface_rect.x = 0;
    	this->sub_surface_rect.y = 0;
    	this->sub_surface_rect.w = 0;
    	this->sub_surface_rect.h = 0;
    }


    /* get the current config */
    if (this->llsurface) {
        getConfiguration();

        /* init the color */
        this->config.color.r = 0;
        this->config.color.g = 0;
        this->config.color.b = 0;
        this->config.color.a = 0;
        this->config.clipped = false;
        this->config.iswinsurface = false;
        this->config.islayersurface = (this->parent && this->parent->isLayerSurface());
        this->config.drawingflags = MMSFB_DRAW_NOFX;
        this->config.blittingflags = MMSFB_BLIT_NOFX;
        this->config.font = NULL;
    }
}



bool MMSFBSurface::isInitialized() {
	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
		return (this->llsurface != NULL);
#else
		return false;
#endif
	}
	else {
		//TODO
		return true;
	}
}

void MMSFBSurface::freeSurfaceBuffer() {

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
		if (this->llsurface > (IDirectFBSurface*)1)
			this->llsurface->Release(this->llsurface);
#endif
	}
	else {
		//free my surface buffers
		MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
		if (!sb->external_buffer) {
			// buffer which is internally allocated
			if (!this->is_sub_surface) {
				// no subsurface
				// free all buffers (front and back buffers)
				for (int i = 0; i < sb->numbuffers; i++) {
					// free only first plane of each buffer, because it points to memory for all used planes
					if (sb->buffers[i].ptr) {
						free(sb->buffers[i].ptr);
						sb->buffers[i].ptr = NULL;
					}
				}
				delete sb;
				sb=NULL;
			}
		}
		if(sb) {
			sb->numbuffers = 0;
		}
	}
	this->llsurface = NULL;
}

void MMSFBSurface::deleteSubSurface(MMSFBSurface *surface) {
	if (surface) {
		/* remove a sub surface from the list */
		for (unsigned int i = 0; i < this->children.size(); i++)
			if (this->children.at(i) == surface) {
	            this->children.erase(this->children.begin()+i);
				break;
			}
	}
	else {
		/* delete all sub surfaces */
		for (unsigned int i = 0; i < this->children.size(); i++) {
			this->children.at(i)->deleteSubSurface(NULL);
			delete this->children.at(i);
		}
	}
}

int MMSFBSurface::calcPitch(int width) {

	MMSFBSurfacePixelFormat pf = this->config.surface_buffer->pixelformat;
    int    pitch = width;

    switch (pf) {
    case MMSFB_PF_ARGB1555:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_RGB16:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_RGB24:
    	pitch = width * 3;
    	break;
    case MMSFB_PF_RGB32:
    	pitch = width * 4;
    	break;
    case MMSFB_PF_ARGB:
    	pitch = width * 4;
    	break;
    case MMSFB_PF_A8:
    	pitch = width;
    	break;
    case MMSFB_PF_YUY2:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_RGB332:
    	pitch = width;
    	break;
    case MMSFB_PF_UYVY:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_I420:
    	pitch = width;
    	break;
    case MMSFB_PF_YV12:
    	pitch = width;
    	break;
    case MMSFB_PF_LUT8:
    	pitch = width;
    	break;
    case MMSFB_PF_ALUT44:
    	pitch = width;
    	break;
    case MMSFB_PF_AiRGB:
    	pitch = width * 4;
    	break;
    case MMSFB_PF_A1:
    	pitch = width / 8;
    	break;
    case MMSFB_PF_NV12:
    	pitch = width;
    	break;
    case MMSFB_PF_NV16:
    	pitch = width;
    	break;
    case MMSFB_PF_ARGB2554:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_ARGB4444:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_NV21:
    	pitch = width;
    	break;
    case MMSFB_PF_AYUV:
    	pitch = width * 4;
    	break;
    case MMSFB_PF_ARGB3565:
    	pitch = width * 2;
    	break;
    case MMSFB_PF_BGR24:
    	pitch = width * 3;
    	break;
    case MMSFB_PF_BGR555:
    	pitch = width * 2;
    	break;
    default:
    	break;
    }

    if (pitch <= 0) pitch = 1;
    if (pitch % 4)
    	pitch += 4 - pitch % 4;

    return pitch;
}

int MMSFBSurface::calcSize(int pitch, int height) {

	MMSFBSurfacePixelFormat pf = this->config.surface_buffer->pixelformat;
    int size = pitch * height;
    int diff;

    if (pf == MMSFB_PF_I420) {
    	// increase size for U/V planes
    	size += size / 2;
    	if ((diff = size % pitch))
    		size += pitch - diff;
    }
    else
    if (pf == MMSFB_PF_YV12) {
    	// increase size for U/V planes
    	size += size / 2;
    	if ((diff = size % pitch))
    		size += pitch - diff;
    }
    else
    if (pf == MMSFB_PF_ARGB3565) {
    	// increase size for alpha plane (4 bit for each pixel)
    	size += size / 4;
    	if ((diff = size % pitch))
    		size += pitch - diff;
    }

    return size;
}

void MMSFBSurface::initPlanePointers(MMSFBSurfacePlanes *planes, int height) {

	MMSFBSurfacePixelFormat pf = this->config.surface_buffer->pixelformat;

	switch (pf) {
	case MMSFB_PF_YV12:
    	planes->ptr3 = ((unsigned char *)planes->ptr) + planes->pitch * height;
    	planes->pitch3 = planes->pitch / 4;
    	planes->ptr2 = ((unsigned char *)planes->ptr3) + planes->pitch3 * height;
    	planes->pitch2 = planes->pitch3;
    	break;
	case MMSFB_PF_ARGB3565:
    	planes->ptr2 = ((unsigned char *)planes->ptr) + planes->pitch * height;
    	planes->pitch2 = planes->pitch / 4;
    	planes->ptr3 = NULL;
    	planes->pitch3 = NULL;
    	break;
    default:
    	break;
	}
}

void MMSFBSurface::getRealSubSurfacePos(MMSFBSurface *surface, bool refreshChilds) {
	if (this->is_sub_surface) {
		this->sub_surface_xoff = this->sub_surface_rect.x + this->parent->sub_surface_xoff;
		this->sub_surface_yoff = this->sub_surface_rect.y + this->parent->sub_surface_yoff;

		if (refreshChilds)
			for (unsigned int i = 0; i < this->children.size(); i++)
				this->children.at(i)->getRealSubSurfacePos(NULL, refreshChilds);
	}
	else {
		this->sub_surface_xoff = 0;
		this->sub_surface_yoff = 0;
	}
}


bool MMSFBSurface::clipSubSurface(MMSFBRegion *region, bool regionset, MMSFBRegion *tmp, bool *tmpset) {
	MMSFBRegion myregion;

	if (!region) {
		if (*tmpset)
			this->root_parent->setClip(tmp);
		else
			this->root_parent->setClip(NULL);
		this->root_parent->unlock();
		return true;
	}

	/* get my region */
	getClip(&myregion);
	if (this->is_sub_surface) {
	    myregion.x1+=sub_surface_xoff;
	    myregion.y1+=sub_surface_yoff;
	    myregion.x2+=sub_surface_xoff;
	    myregion.y2+=sub_surface_yoff;
	}

	if (!regionset) {
		/* init region */
		*region = myregion;
		if(this->parent)
    		return this->parent->clipSubSurface(region, true, tmp, tmpset);
	}

    /* check if input region is within my region */
    if (region->x1 < myregion.x1)
    	region->x1 = myregion.x1;
    else
    if (region->x1 > myregion.x2)
    	return false;

    if (region->y1 < myregion.y1)
    	region->y1 = myregion.y1;
    else
    if (region->y1 > myregion.y2)
    	return false;

    if (region->x2 > myregion.x2)
    	region->x2 = myregion.x2;
    else
    if (region->x2 < myregion.x1)
    	return false;

    if (region->y2 > myregion.y2)
    	region->y2 = myregion.y2;
    else
    if (region->y2 < myregion.y1)
    	return false;

    /* have a parent, call recursive */
	if (this->is_sub_surface)
		return this->parent->clipSubSurface(region, true, tmp, tmpset);

	/* i am the root, set clip now */
	lock();
	if (this->config.clipped) {
		getClip(tmp);
		*tmpset=true;
	}
	else
		*tmpset=false;
	setClip(region);
	return true;
}

void *MMSFBSurface::getDFBSurface() {
#ifdef  __HAVE_DIRECTFB__
	if (!this->use_own_alloc)
		return this->llsurface;
#endif
	return NULL;
}

bool MMSFBSurface::getConfiguration(MMSFBSurfaceConfig *config) {

    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
		DFBSurfaceCapabilities  caps;
		DFBResult               dfbres;
		DFBSurfacePixelFormat   mypf;

		/* get size */
		if (!this->is_sub_surface) {
			if ((dfbres=this->llsurface->GetSize(this->llsurface, &(this->config.w), &(this->config.h))) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::GetSize() failed");
				return false;
			}
			this->config.surface_buffer->sbw = this->config.w;
			this->config.surface_buffer->sbh = this->config.h;
		}
		else {
#ifdef USE_DFB_SUBSURFACE
			if ((dfbres=this->llsurface->GetSize(this->llsurface, &(this->config.w), &(this->config.h))) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::GetSize() failed");
				return false;
			}
#else
			this->config.w = this->sub_surface_rect.w;
			this->config.h = this->sub_surface_rect.h;
#endif
		}

		// get the surface pitch
		void *ptr;
		if (this->llsurface->Lock(this->llsurface, DSLF_READ, &ptr, &this->config.surface_buffer->buffers[0].pitch) == DFB_OK) {
			this->llsurface->Unlock(this->llsurface);
		}

		/* get pixelformat */
		if ((dfbres=this->llsurface->GetPixelFormat(this->llsurface, &mypf)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::GetPixelFormat() failed");
			return false;
		}

		/* build a format string */
		this->config.surface_buffer->pixelformat = getMMSFBPixelFormatFromDFBPixelFormat(mypf);
		this->config.surface_buffer->alphachannel = isAlphaPixelFormat(this->config.surface_buffer->pixelformat);

		/* get capabilities */
		if ((dfbres=this->llsurface->GetCapabilities(this->llsurface, &caps)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::GetCapabilities() failed");
			return false;
		}

	    /* is it a premultiplied surface? */
		this->config.surface_buffer->premultiplied = caps & DSCAPS_PREMULTIPLIED;

	    /* get the buffer mode */
		this->config.surface_buffer->backbuffer = 0;
	    if (caps & DSCAPS_DOUBLE)
	    	this->config.surface_buffer->backbuffer = 1;
	    else
	    if (caps & DSCAPS_TRIPLE)
	    	this->config.surface_buffer->backbuffer = 2;

	    /* system only? */
	    this->config.surface_buffer->systemonly = false;
	    if (caps & DSCAPS_SYSTEMONLY)
	    	this->config.surface_buffer->systemonly = true;

	    /* fill return config */
	    if (config)
	        *config = this->config;

	    /* log some infos */
	    if ((!config)&&(!this->is_sub_surface)) {
	    	DEBUGMSG("MMSGUI", "Surface properties:");

	    	DEBUGMSG("MMSGUI", " size:         " + iToStr(this->config.w) + "x" + iToStr(this->config.h));
			DEBUGMSG("MMSGUI", " pitch:        " + iToStr(this->config.surface_buffer->buffers[0].pitch));

		    if (this->config.surface_buffer->alphachannel)
		    	DEBUGMSG("MMSGUI", " pixelformat:  " + getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat) + ",ALPHACHANNEL");
		    else
		    	DEBUGMSG("MMSGUI", " pixelformat:  " + getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat));

		    DEBUGMSG("MMSGUI", " capabilities:");

		    if (caps & DSCAPS_PRIMARY)
		    	DEBUGMSG("MMSGUI", "  PRIMARY");
		    if (caps & DSCAPS_SYSTEMONLY)
		    	DEBUGMSG("MMSGUI", "  SYSTEMONLY");
		    if (caps & DSCAPS_VIDEOONLY)
		    	DEBUGMSG("MMSGUI", "  VIDEOONLY");
		    if (caps & DSCAPS_DOUBLE)
		    	DEBUGMSG("MMSGUI", "  DOUBLE");
		    if (caps & DSCAPS_TRIPLE)
		    	DEBUGMSG("MMSGUI", "  TRIPLE");
		    if (caps & DSCAPS_PREMULTIPLIED)
		    	DEBUGMSG("MMSGUI", "  PREMULTIPLIED");
	    }

	    return true;
#endif
	}
	else {
		/* get size */
		if (this->is_sub_surface) {
			this->config.w = this->sub_surface_rect.w;
			this->config.h = this->sub_surface_rect.h;
		}

		/* fill return config */
		if (config)
			*config = this->config;

		/* log some infos */
		if ((!config)&&(!this->is_sub_surface)) {
			DEBUGMSG("MMSGUI", "Surface properties:");

			DEBUGMSG("MMSGUI", " size:         " + iToStr(this->config.w) + "x" + iToStr(this->config.h));
			DEBUGMSG("MMSGUI", " pitch:        " + iToStr(this->config.surface_buffer->buffers[0].pitch));

			if (this->config.surface_buffer->alphachannel)
				DEBUGMSG("MMSGUI", " pixelformat:  " + getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat) + ",ALPHACHANNEL");
			else
				DEBUGMSG("MMSGUI", " pixelformat:  " + getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat));

			DEBUGMSG("MMSGUI", " capabilities:");

			if (this->config.surface_buffer->systemonly)
				DEBUGMSG("MMSGUI", "  SYSTEMONLY");
			if (this->config.surface_buffer->backbuffer == 1)
				DEBUGMSG("MMSGUI", "  DOUBLE");
			if (this->config.surface_buffer->backbuffer == 2)
				DEBUGMSG("MMSGUI", "  TRIPLE");
			if (this->config.surface_buffer->premultiplied)
				DEBUGMSG("MMSGUI", "  PREMULTIPLIED");
		}
	    return true;
	}
	return false;
}

void MMSFBSurface::setExtendedAcceleration(bool extendedaccel) {
	this->extendedaccel = extendedaccel;
}

bool MMSFBSurface::getExtendedAcceleration() {
	return this->extendedaccel;
}

void MMSFBSurface::setAllocMethod(MMSFBSurfaceAllocMethod allocmethod, bool reset) {
	this->allocmethod = allocmethod;
	if (reset) {
		if (this->use_own_alloc != (this->allocmethod != MMSFBSurfaceAllocMethod_dfb)) {
			this->use_own_alloc = (this->allocmethod != MMSFBSurfaceAllocMethod_dfb);
			getConfiguration();
		}
	}
	else
		if (this->allocmethod != MMSFBSurfaceAllocMethod_dfb)
			printf("DISKO: Using own surface memory management.\n");
}

MMSFBSurfaceAllocMethod MMSFBSurface::getAllocMethod() {
	return this->allocmethod;
}

bool MMSFBSurface::isWinSurface() {
    return this->config.iswinsurface;
}

bool MMSFBSurface::isLayerSurface() {
    return this->config.islayersurface;
}

bool MMSFBSurface::isSubSurface() {
    return this->is_sub_surface;
}

MMSFBSurface *MMSFBSurface::getParent() {
    return this->parent;
}

MMSFBSurface *MMSFBSurface::getRootParent() {
    return this->root_parent;
}

bool MMSFBSurface::setWinSurface(bool iswinsurface) {

    /* check if initialized */
    INITCHECK;

    /* set the flag */
    this->config.iswinsurface = iswinsurface;

    return true;
}

bool MMSFBSurface::setLayerSurface(bool islayersurface) {

    /* check if initialized */
    INITCHECK;

    /* set the flag */
    this->config.islayersurface = islayersurface;

    return true;
}


bool MMSFBSurface::getPixelFormat(MMSFBSurfacePixelFormat *pixelformat) {

    /* check if initialized */
    INITCHECK;

    /* return the pixelformat */
    *pixelformat = this->config.surface_buffer->pixelformat;

    return true;
}

bool MMSFBSurface::getSize(int *w, int *h) {

    /* check if initialized */
    INITCHECK;

    /* return values */
    *w = this->config.w;
    *h = this->config.h;

    return true;
}

bool MMSFBSurface::getNumberOfBuffers(int *num) {

    // check if initialized
    INITCHECK;

    // return value
    *num = this->config.surface_buffer->backbuffer + 1;

    return true;
}

bool MMSFBSurface::getMemSize(int *size) {

	/* check if initialized */
    INITCHECK;

    /* init size */
    if (!size)
    	return false;
    *size = 0;

    *size = calcSize(this->config.surface_buffer->buffers[0].pitch, this->config.h);

    return true;
}


bool MMSFBSurface::setFlipFlags(MMSFBFlipFlags flags) {
	this->flipflags = flags;
	return true;
}



bool MMSFBSurface::clear(unsigned char r, unsigned char g,
                         unsigned char b, unsigned char a) {
    bool ret = false;

    /* check if initialized */
    INITCHECK;


	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;
	    D_DEBUG_AT( MMS_Surface, "clear( argb %02x %02x %02x %02x ) <- %dx%d\n",
	                a, r, g, b, this->config.surface_buffer->w, this->config.surface_buffer->h );
	    MMSFB_TRACE();

	    if ((a < 0xff)&&(this->config.surface_buffer->premultiplied)) {
			// premultiplied surface, have to premultiply the color
			register int aa = a + 1;
			r = (aa * r) >> 8;
			g = (aa * g) >> 8;
			b = (aa * b) >> 8;
		}

		if (!this->is_sub_surface) {
			/* clear surface */
			if ((dfbres=this->llsurface->Clear(this->llsurface, r, g, b, a)) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::Clear() failed");
				return false;
			}
			ret = true;
		}
		else {

#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE
#endif

			/* clear surface */
			if (this->llsurface->Clear(this->llsurface, r, g, b, a) == DFB_OK)
				ret = true;

#ifndef USE_DFB_SUBSURFACE
			UNCLIPSUBSURFACE
#endif
		}
#endif
	}
	else {
		MMSFBColor *col = &this->config.color;
		MMSFBColor savedcol = *col;
		col->r = r;
		col->g = g;
		col->b = b;
		col->a = a;
		MMSFBDrawingFlags saveddf = this->config.drawingflags;
		this->config.drawingflags = MMSFB_DRAW_SRC_PREMULTIPLY;

		if (!this->is_sub_surface) {
			ret = extendedAccelFillRectangle(0, 0, this->config.w, this->config.h);
		}
		else {
			CLIPSUBSURFACE

			ret = extendedAccelFillRectangle(this->sub_surface_xoff, this->sub_surface_yoff, this->config.w, this->config.h);

			UNCLIPSUBSURFACE
		}

		*col = savedcol;
		this->config.drawingflags = saveddf;
	}


    return ret;
}

bool MMSFBSurface::setColor(unsigned char r, unsigned char g,
                            unsigned char b, unsigned char a) {

    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;

	    /* set color */
#ifdef USE_DFB_SUBSURFACE
		if ((dfbres=this->llsurface->SetColor(this->llsurface, r, g, b, a)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::SetColor() failed");
			return false;
		}
#else
		if (!this->is_sub_surface) {
			if ((dfbres=this->llsurface->SetColor(this->llsurface, r, g, b, a)) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::SetColor() failed");
				return false;
			}
		}
#endif
#endif
	}

    /* save the color */
	MMSFBColor *col = &this->config.color;
	col->r = r;
	col->g = g;
	col->b = b;
	col->a = a;

    // set the default drawing flags
    // reason a): if it is an PREMULTIPLIED surface, the given color has to
    //            premultiplied internally before using it
    // reason b): if an alpha value is specified, the next draw function
    //            should blend over the surface
	this->setDrawingFlagsByAlpha(a);

    return true;
}

bool MMSFBSurface::setColor(MMSFBColor &color) {
	return setColor(color.r, color.g, color.b, color.a);
}

bool MMSFBSurface::getColor(MMSFBColor *color) {

    /* check if initialized */
    INITCHECK;

    /* return the color */
    *color = this->config.color;

    return true;
}

bool MMSFBSurface::setClip(MMSFBRegion *clip) {

    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;

	    /* set clip */
#ifdef USE_DFB_SUBSURFACE
		if ((dfbres=this->llsurface->SetClip(this->llsurface, (DFBRegion*)clip)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::SetClip() failed");
			return false;
		}
#else
		if (!this->is_sub_surface) {
			if ((dfbres=this->llsurface->SetClip(this->llsurface, (DFBRegion*)clip)) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::SetClip() failed");
				return false;
			}
		}
#endif
#endif
	}

    /* save the region */
    if (clip) {
    	this->config.clipped = true;
	    this->config.clip = *clip;
    }
    else
    	this->config.clipped = false;

    return true;
}

bool MMSFBSurface::setClip(int x1, int y1, int x2, int y2) {
	MMSFBRegion clip;
	clip.x1=x1;
	clip.y1=y1;
	clip.x2=x2;
	clip.y2=y2;
	return setClip(&clip);
}

bool MMSFBSurface::getClip(MMSFBRegion *clip) {

	/* check if initialized */
    INITCHECK;

    /* return the clip region */
    if (this->config.clipped) {
    	*clip = this->config.clip;
    }
    else {
    	clip->x1 = 0;
    	clip->y1 = 0;
    	clip->x2 = this->config.w - 1;
    	clip->y2 = this->config.h - 1;
    }

	return true;
}


bool MMSFBSurface::setDrawingFlags(MMSFBDrawingFlags flags) {

    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;

	    /* set the drawing flags */
#ifdef USE_DFB_SUBSURFACE
		if ((dfbres=this->llsurface->SetDrawingFlags(this->llsurface, getDFBSurfaceDrawingFlagsFromMMSFBDrawingFlags(flags))) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::SetDrawingFlags() failed");
			return false;
		}
#else
		if (!this->is_sub_surface) {
			if ((dfbres=this->llsurface->SetDrawingFlags(this->llsurface, getDFBSurfaceDrawingFlagsFromMMSFBDrawingFlags(flags))) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::SetDrawingFlags() failed");
				return false;
			}
		}
#endif
#endif
	}

    /* save the flags */
    this->config.drawingflags = flags;

    return true;
}



bool MMSFBSurface::drawLine(int x1, int y1, int x2, int y2) {
    bool ret = false;

    // check if initialized
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;
	    MMSFB_BREAK();

	    // draw a line
		if (!this->is_sub_surface) {
			if (!extendedAccelDrawLine(x1, y1, x2, y2))
				if ((dfbres=this->llsurface->DrawLine(this->llsurface, x1, y1, x2, y2)) != DFB_OK) {
					MMSFB_SetError(dfbres, "IDirectFBSurface::DrawLine() failed");
					return false;
				}
			ret = true;
		}
		else {

#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			x1+=this->sub_surface_xoff;
			y1+=this->sub_surface_yoff;
			x2+=this->sub_surface_xoff;
			y2+=this->sub_surface_yoff;

			SETSUBSURFACE_DRAWINGFLAGS;
#endif

			if (extendedAccelDrawLine(x1, y1, x2, y2))
				ret = true;
			else
				if (this->llsurface->DrawLine(this->llsurface, x1, y1, x2, y2) == DFB_OK)
					ret = true;

#ifndef USE_DFB_SUBSURFACE
			RESETSUBSURFACE_DRAWINGFLAGS;

			UNCLIPSUBSURFACE
#endif
		}

#endif
	}
	else {

		if (!this->is_sub_surface) {
			ret = extendedAccelDrawLine(x1, y1, x2, y2);
		}
		else {
			CLIPSUBSURFACE

			x1+=this->sub_surface_xoff;
			y1+=this->sub_surface_yoff;
			x2+=this->sub_surface_xoff;
			y2+=this->sub_surface_yoff;

			ret = extendedAccelDrawLine(x1, y1, x2, y2);

			UNCLIPSUBSURFACE
		}

	}

    return ret;
}

bool MMSFBSurface::drawRectangle(int x, int y, int w, int h) {
	bool ret = false;

    // check if initialized
    INITCHECK;
    if (w < 1 || h < 1)
    	return false;

    // draw lines...
    if (w==1)
    	ret = drawLine(x, y, x, y+h-1);
    else
    if (h==1)
    	ret = drawLine(x, y, x+w-1, y);
    else {
    	ret = drawLine(x, y, x+w-1, y);
    	ret = drawLine(x, y+h-1, x+w-1, y+h-1);
    	if (h>2) {
        	ret = drawLine(x, y+1, x, y+h-2);
        	ret = drawLine(x+w-1, y+1, x+w-1, y+h-2);
    	}
    }

    return ret;
}

bool MMSFBSurface::fillRectangle(int x, int y, int w, int h) {
    bool		ret = false;

    // check if initialized
    INITCHECK;

    if ((w <= 0) || (h <= 0)) {
    	// use full surface
    	x = 0;
    	y = 0;
    	w = this->config.w;
    	h = this->config.h;
    }

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;
	    D_DEBUG_AT( MMS_Surface, "fill( %d,%d - %dx%d ) <- %dx%d, %02x %02x %02x %02x\n",
	                x, y, w, h, this->config.surface_buffer->w, this->config.surface_buffer->h,
	                this->config.color.a, this->config.color.r, this->config.color.g, this->config.color.b );
	    MMSFB_TRACE();

	    /* fill rectangle */
		if (!this->is_sub_surface) {
			if (!extendedAccelFillRectangle(x, y, w, h))
				if ((dfbres=this->llsurface->FillRectangle(this->llsurface, x, y, w, h)) != DFB_OK) {
					MMSFB_SetError(dfbres, "IDirectFBSurface::FillRectangle() failed");
					return false;
				}
			ret = true;
		}
		else {

#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			x+=this->sub_surface_xoff;
			y+=this->sub_surface_yoff;

			SETSUBSURFACE_DRAWINGFLAGS;
#endif

			if (extendedAccelFillRectangle(x, y, w, h))
				ret = true;
			else
				if (this->llsurface->FillRectangle(this->llsurface, x, y, w, h) == DFB_OK)
					ret = true;

#ifndef USE_DFB_SUBSURFACE
			RESETSUBSURFACE_DRAWINGFLAGS;

			UNCLIPSUBSURFACE
#endif
		}
#endif
	}
	else {

		if (!this->is_sub_surface) {
			ret = extendedAccelFillRectangle(x, y, w, h);
		}
		else {
			CLIPSUBSURFACE

			x+=this->sub_surface_xoff;
			y+=this->sub_surface_yoff;

			ret = extendedAccelFillRectangle(x, y, w, h);

			UNCLIPSUBSURFACE
		}

	}

    return ret;
}

bool MMSFBSurface::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    MMSFB_BREAK();

    /* check if initialized */
    INITCHECK;

    /* draw triangle */
    drawLine(x1, y1, x2, y2);
    drawLine(x1, y1, x3, y3);
    drawLine(x2, y2, x3, y3);

    return true;
}

bool MMSFBSurface::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {

    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;
	    MMSFB_BREAK();

	    /* fill triangle */
		if (!this->is_sub_surface) {
			if ((dfbres=this->llsurface->FillTriangle(this->llsurface, x1, y1, x2, y2, x3, y3)) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::FillTriangle() failed");
				return false;
			}
		}
		else {

#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			x1+=this->sub_surface_xoff;
			y1+=this->sub_surface_yoff;
			x2+=this->sub_surface_xoff;
			y2+=this->sub_surface_yoff;
			x3+=this->sub_surface_xoff;
			y3+=this->sub_surface_yoff;

			SETSUBSURFACE_DRAWINGFLAGS;
#endif

			this->llsurface->FillTriangle(this->llsurface, x1, y1, x2, y2, x3, y3);

#ifndef USE_DFB_SUBSURFACE
			RESETSUBSURFACE_DRAWINGFLAGS;

			UNCLIPSUBSURFACE
#endif

		}
#endif
	}
	else {
		//TODO
	}

    return true;
}

bool MMSFBSurface::drawCircle(int x, int y, int radius, int start_octant, int end_octant) {

    MMSFB_BREAK();

    /* check if initialized */
    INITCHECK;

    /* draw circle */
    if (end_octant < start_octant) end_octant = start_octant;
    if ((start_octant<=4)&&(end_octant>=3))
        drawLine(x, y + radius, x, y + radius);
    if ((start_octant==0)||(end_octant==7))
        drawLine(x, y - radius, x, y - radius);
    if ((start_octant<=2)&&(end_octant>=1))
        drawLine(x + radius, y, x + radius, y);
    if ((start_octant<=6)&&(end_octant>=5))
        drawLine(x - radius, y, x - radius, y);
    int mr = radius * radius;
    int mx = 1;
    int my = (int) (sqrt(mr - 1) + 0.5);

    while (mx < my) {
        if ((start_octant<=0)&&(end_octant>=0))
            drawLine(x + mx, y - my, x + mx, y - my); /* octant 0 */
        if ((start_octant<=1)&&(end_octant>=1))
            drawLine(x + my, y - mx, x + my, y - mx); /* octant 1 */
        if ((start_octant<=2)&&(end_octant>=2))
            drawLine(x + my, y + mx, x + my, y + mx); /* octant 2 */
        if ((start_octant<=3)&&(end_octant>=3))
            drawLine(x + mx, y + my, x + mx, y + my); /* octant 3 */
        if ((start_octant<=4)&&(end_octant>=4))
            drawLine(x - mx, y + my, x - mx, y + my); /* octant 4 */
        if ((start_octant<=5)&&(end_octant>=5))
            drawLine(x - my, y + mx, x - my, y + mx); /* octant 5 */
        if ((start_octant<=6)&&(end_octant>=6))
            drawLine(x - my, y - mx, x - my, y - mx); /* octant 6 */
        if ((start_octant<=7)&&(end_octant>=7))
            drawLine(x - mx, y - my, x - mx, y - my); /* octant 7 */

        mx++;
        my = (int) (sqrt(mr - mx*mx) + 0.5);
    }

    if (mx == my) {
        if ((start_octant<=3)&&(end_octant>=2))
            drawLine(x + mx, y + my, x + mx, y + my);
        if ((start_octant<=1)&&(end_octant>=0))
            drawLine(x + mx, y - my, x + mx, y - my);
        if ((start_octant<=5)&&(end_octant>=4))
            drawLine(x - mx, y + my, x - mx, y + my);
        if ((start_octant<=7)&&(end_octant>=6))
            drawLine(x - mx, y - my, x - mx, y - my);
    }

    return true;
}



bool MMSFBSurface::setBlittingFlags(MMSFBBlittingFlags flags) {

    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;

	    if ((flags & MMSFB_BLIT_BLEND_ALPHACHANNEL)||(flags & MMSFB_BLIT_BLEND_COLORALPHA)) {
			/* if we do alpha channel blitting, we have to change the default settings to become correct results */
			if (this->config.surface_buffer->alphachannel)
				llsurface->SetSrcBlendFunction(llsurface,(DFBSurfaceBlendFunction)DSBF_ONE);
			else
				llsurface->SetSrcBlendFunction(llsurface,(DFBSurfaceBlendFunction)DSBF_SRCALPHA);
			llsurface->SetDstBlendFunction(llsurface,(DFBSurfaceBlendFunction)(DSBF_INVSRCALPHA));

			if (flags & MMSFB_BLIT_BLEND_COLORALPHA)
				 flags = (MMSFBBlittingFlags)(flags | MMSFB_BLIT_SRC_PREMULTCOLOR);
		}

		/* set the blitting flags */
		if ((dfbres=this->llsurface->SetBlittingFlags(this->llsurface, getDFBSurfaceBlittingFlagsFromMMSFBBlittingFlags(flags))) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::SetBlittingFlags() failed");

			return false;
		}
#endif
	}

    /* save the flags */
    this->config.blittingflags = flags;

    return true;
}

bool MMSFBSurface::getBlittingFlags(MMSFBBlittingFlags *flags) {

	// check if initialized
    INITCHECK;

    // parameter given?
    if (!flags)
    	return false;

    // save the flags
    *flags = this->config.blittingflags;

    return true;
}

bool MMSFBSurface::extendedLock(MMSFBSurface *src, MMSFBSurfacePlanes *src_planes,
								MMSFBSurface *dst, MMSFBSurfacePlanes *dst_planes) {

	if (src) {
		memset(src_planes, 0, sizeof(MMSFBSurfacePlanes));
		src->lock(MMSFB_LOCK_READ, src_planes, false);
		if (!src_planes->ptr) {
			return false;
		}
	}
	if (dst) {
		memset(dst_planes, 0, sizeof(MMSFBSurfacePlanes));
		dst->lock(MMSFB_LOCK_WRITE, dst_planes, false);
		if (!dst_planes->ptr) {
			if (src)
				src->unlock(false);
			return false;
		}
	}

	if (this->surface_invert_lock) {
		if (src_planes && dst_planes) {
			MMSFBSurfacePlanes t_planes;
			t_planes = *src_planes;
			*src_planes = *dst_planes;
			*dst_planes = t_planes;
		}
	}

	return true;
}

void MMSFBSurface::extendedUnlock(MMSFBSurface *src, MMSFBSurface *dst, MMSFBSurfacePlanes *dst_planes) {
	if (dst) {
		if (dst_planes) {
			// save the given dst_planes to the surface buffers array
			MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
			sb->buffers[sb->currbuffer_write] = *dst_planes;
		}
		else {
			// dst_planes not given, reset the special flags
			MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
			sb->buffers[sb->currbuffer_write].opaque = false;
			sb->buffers[sb->currbuffer_write].transparent = false;
		}
		dst->unlock(false);
	}
	if (src) {
		src->unlock(false);
	}
}


bool MMSFBSurface::printMissingCombination(string method, MMSFBSurface *source, MMSFBSurfacePlanes *src_planes,
										   MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height) {
#ifdef  __HAVE_DIRECTFB__
	// failed, check if it must not
	if ((!this->use_own_alloc)&&(!source->use_own_alloc))
		return false;
#endif

	// fatal error!!!
	// we use our own surface buffer handling, but we have not found a matching routine!!!
	// so print the missing combination and return true
	printf("DISKO: Missing following combination in method %s\n", method.c_str());
	if (source) {
		printf("  source type:               %s\n", (source->is_sub_surface)?"subsurface":"surface");
		printf("  source memory:             %s\n", (source->use_own_alloc)?"managed by disko":"managed by dfb");
		printf("  source pixelformat:        %s\n", getMMSFBPixelFormatString(source->config.surface_buffer->pixelformat).c_str());
		printf("  source premultiplied:      %s\n", (source->config.surface_buffer->premultiplied)?"yes":"no");
	}
	if (src_planes) {
		printf("  source type:               surface\n");
		printf("  source memory:             extern (0x%08lx, pitch=%d)\n", (unsigned long)src_planes->ptr, src_planes->pitch);
		if (src_planes->ptr2) {
			printf("                                    (0x%08lx, pitch=%d)\n",  (unsigned long)src_planes->ptr2, src_planes->pitch2);
			if (src_planes->ptr3)
				printf("                                    (0x%08lx, pitch=%d)\n",  (unsigned long)src_planes->ptr3, src_planes->pitch3);
		}
		printf("  source pixelformat:        %s\n", getMMSFBPixelFormatString(src_pixelformat).c_str());
	}
	printf("  destination type:          %s\n", (this->is_sub_surface)?"subsurface":"surface");
	printf("  destination memory:        %s\n", (this->use_own_alloc)?"managed by disko":"managed by dfb");
	printf("  destination pixelformat:   %s\n", getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat).c_str());
	printf("  destination premultiplied: %s\n", (this->config.surface_buffer->premultiplied)?"yes":"no");
	printf("  destination color:         r=%d, g=%d, b=%d, a=%d\n", this->config.color.r, this->config.color.g, this->config.color.b, this->config.color.a);
	if ((source)||(src_planes)) {
		printf("  blitting flags (%06x):  ", this->config.blittingflags);
		if (this->config.blittingflags == MMSFB_BLIT_NOFX)
			printf(" NOFX");
		if (this->config.blittingflags & MMSFB_BLIT_BLEND_ALPHACHANNEL)
			printf(" BLEND_ALPHACHANNEL");
		if (this->config.blittingflags & MMSFB_BLIT_BLEND_COLORALPHA)
			printf(" BLEND_COLORALPHA");
		if (this->config.blittingflags & MMSFB_BLIT_COLORIZE)
			printf(" COLORIZE");
		if (this->config.blittingflags & MMSFB_BLIT_SRC_PREMULTIPLY)
			printf(" SRC_PREMULTIPLY");
		if (this->config.blittingflags & MMSFB_BLIT_ANTIALIASING)
			printf(" ANTIALIASING");
		printf("\n");
	}
	else {
		printf("  drawing flags (%06x):   ", this->config.drawingflags);
		if (this->config.drawingflags == MMSFB_DRAW_NOFX)
			printf(" NOFX");
		if (this->config.drawingflags & MMSFB_DRAW_BLEND)
			printf(" BLEND");
		if (this->config.drawingflags & MMSFB_DRAW_SRC_PREMULTIPLY)
			printf(" SRC_PREMULTIPLY");
		printf("\n");
	}
	printf("*****\n");
	return true;
}




bool MMSFBSurface::extendedAccelBlitEx(MMSFBSurface *source,
									   MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
									   MMSFBRectangle *src_rect, int x, int y) {
	MMSFBSurfacePlanes my_src_planes;
	if (source) {
		// premultiplied surface?
		if (!source->config.surface_buffer->premultiplied)
			return false;

		src_pixelformat = source->config.surface_buffer->pixelformat;
		src_width = (!source->root_parent)?source->config.w:source->root_parent->config.w;
		src_height = (!source->root_parent)?source->config.h:source->root_parent->config.h;

		// empty source planes
		memset(&my_src_planes, 0, sizeof(MMSFBSurfacePlanes));
		src_planes = &my_src_planes;
	}

	// a few help and clipping values
	MMSFBSurfacePlanes dst_planes;
	int sx = src_rect->x;
	int sy = src_rect->y;
	int sw = src_rect->w;
	int sh = src_rect->h;
	MMSFBRegion clipreg;
#ifndef USE_DFB_SUBSURFACE
	if (!this->is_sub_surface) {
#endif
		// normal surface or dfb subsurface
		if (!this->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->config.w - 1;
			clipreg.y2 = this->config.h - 1;
		}
		else
			clipreg = this->config.clip;
#ifndef USE_DFB_SUBSURFACE
	}
	else {
		// subsurface
		if (!this->root_parent->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->root_parent->config.w - 1;
			clipreg.y2 = this->root_parent->config.h - 1;
		}
		else
			clipreg = this->root_parent->config.clip;
	}
#endif

	if (x < clipreg.x1) {
		// left outside
		sx+= clipreg.x1 - x;
		sw-= clipreg.x1 - x;
		if (sw <= 0)
			return true;
		x = clipreg.x1;
	}
	else
	if (x > clipreg.x2)
		// right outside
		return true;
	if (y < clipreg.y1) {
		// top outside
		sy+= clipreg.y1 - y;
		sh-= clipreg.y1 - y;
		if (sh <= 0)
			return true;
		y = clipreg.y1;
	}
	else
	if (y > clipreg.y2)
		// bottom outside
		return true;
	if (x + sw - 1 > clipreg.x2)
		// to width
		sw = clipreg.x2 - x + 1;
	if (y + sh - 1 > clipreg.y2)
		// to height
		sh = clipreg.y2 - y + 1;

	// adjust x/y
	if (x < 0) {
		sx -= x;
		sw += x;
		x = 0;
	}
	if (y < 0) {
		sy -= y;
		sh += y;
		y = 0;
	}
	if ((sw <= 0)||(sh <= 0))
		return true;

	// extract antialiasing flag from blittingflags
//	bool antialiasing = (this->config.blittingflags & MMSFB_BLIT_ANTIALIASING);
	MMSFBBlittingFlags blittingflags = this->config.blittingflags & ~MMSFB_BLIT_ANTIALIASING;

	// checking pixelformats...
	switch (src_pixelformat) {
	case MMSFB_PF_ARGB:
		// source is ARGB
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB) {
			// destination is ARGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_argb_to_argb(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb_to_argb(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_coloralpha_argb_to_argb(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y,
							this->config.color.a);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_AiRGB) {
			// destination is AiRGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb_to_airgb(
							src_planes, src_height,
							sx, sy, sw, sh,
							(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb_to_rgb32(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_coloralpha_argb_to_rgb32(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y,
							this->config.color.a);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB16) {
			// destination is RGB16 (RGB565)
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_argb_to_rgb16(src_planes, src_height,
											 sx, sy, sw, sh,
											 (unsigned short int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											 x, y);
					extendedUnlock(source, this);
					return true;
				}
				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb_to_rgb16(src_planes, src_height,
												   sx, sy, sw, sh,
												   &dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
												   x, y);
					extendedUnlock(source, this);
					return true;
				}
				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
//				printf("fehlt noch-!!!\n");
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB3565) {
			// destination is ARGB3565
			switch (blittingflags) {
			case MMSFB_BLIT_NOFX:
				// convert without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_argb_to_argb3565(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}
				return false;

			case MMSFB_BLIT_BLEND_ALPHACHANNEL:
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb_to_argb3565(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}
				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_argb_to_yv12(
							src_planes, src_height,
							sx, sy, sw, sh,
							(unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb_to_yv12(
							src_planes, src_height,
							sx, sy, sw, sh,
							(unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_coloralpha_argb_to_yv12(
							src_planes, src_height,
							sx, sy, sw, sh,
							(unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y,
							this->config.color.a);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB24) {
			// destination is RGB24
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb_to_rgb24(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_BGR24) {
			// destination is BGR24
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb_to_bgr24(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_coloralpha_argb_to_bgr24(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y,
							this->config.color.a);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_BGR555) {
			// destination is BGR555
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb_to_bgr555(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_RGB32:
		// source is RGB32
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_rgb32_to_rgb32(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_COLORALPHA)) {
				// blitting with coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_coloralpha_rgb32_to_rgb32(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y,
							this->config.color.a);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}


			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_RGB16:
		// source is RGB16 (RGB565)
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB16) {
			// destination is RGB16 (RGB565)
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_rgb16_to_rgb16(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB) {
			// destination is ARGB
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_rgb16_to_argb(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_rgb16_to_rgb32(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_AiRGB:
		// source is AiRGB
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_AiRGB) {
			// destination is AiRGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_airgb_to_airgb(src_planes, src_height,
											  sx, sy, sw, sh,
											  &dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											  x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_airgb_to_airgb(src_planes, src_height,
													sx, sy, sw, sh,
													(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
													x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_coloralpha_airgb_to_airgb(src_planes, src_height,
															   sx, sy, sw, sh,
															   (unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
															   x, y,
															   this->config.color.a);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB16) {
			// destination is RGB16 (RGB565)
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_airgb_to_rgb16(src_planes, src_height,
											  sx, sy, sw, sh,
											  (unsigned short int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											  x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_airgb_to_rgb16(src_planes, src_height,
													sx, sy, sw, sh,
													(unsigned short int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
													x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_AYUV:
		// source is AYUV
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_AYUV) {
			// destination is AYUV
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_ayuv_to_ayuv(src_planes, src_height,
										    sx, sy, sw, sh,
										    &dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
										    x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_ayuv_to_ayuv(src_planes, src_height,
												  sx, sy, sw, sh,
												  (unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
												  x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_coloralpha_ayuv_to_ayuv(src_planes, src_height,
															 sx, sy, sw, sh,
															 (unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
															 x, y,
															 this->config.color.a);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB16) {
			// destination is RGB16 (RGB565)
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// convert without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_ayuv_to_rgb16(src_planes, src_height,
											 sx, sy, sw, sh,
											 (unsigned short int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											 x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_ayuv_to_rgb16(src_planes, src_height,
												   sx, sy, sw, sh,
												   (unsigned short int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
												   x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_ayuv_to_yv12(src_planes, src_height,
												  sx, sy, sw, sh,
												  (unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
												  x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_coloralpha_ayuv_to_yv12(src_planes, src_height,
															 sx, sy, sw, sh,
															 (unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
															 x, y,
															 this->config.color.a);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_YV12:
		// source is YV12
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_yv12_to_yv12(src_planes, src_height,
											sx, sy, sw, sh,
											&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_yv12_to_rgb32(src_planes, src_height,
											 sx, sy, sw, sh,
											 (unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											 x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_I420:
		// source is I420
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_I420) {
			// destination is I420
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_i420_to_i420(src_planes, src_height,
											sx, sy, sw, sh,
											&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_i420_to_yv12(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_YUY2:
		// source is YUY2
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YUY2) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_yuy2_to_yuy2(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_yuy2_to_yv12(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_RGB24:
		// source is RGB24
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB) {
			// destination is ARGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_rgb24_to_argb(src_planes, src_height,
										    sx, sy, sw, sh,
										    &dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
										    x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_rgb24_to_rgb32(src_planes, src_height,
											  sx, sy, sw, sh,
											  &dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											  x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB24) {
			// destination is RGB24
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_rgb24_to_rgb24(src_planes, src_height,
											  sx, sy, sw, sh,
											  &dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											  x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_rgb24_to_yv12(
							src_planes, src_height,
							sx, sy, sw, sh,
							(unsigned char *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_BGR24:
		// source is BGR24
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_BGR24) {
			// destination is BGR24
			if 	  ((blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX)
				|| (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_bgr24_to_bgr24(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_COLORALPHA)) {
				// blitting with coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_coloralpha_bgr24_to_bgr24(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y,
							this->config.color.a);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_ARGB3565:
		// source is ARGB3565
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB3565) {
			// destination is ARGB3565
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_argb3565_to_argb3565(src_planes, src_height,
											        sx, sy, sw, sh,
											        &dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
											        x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_ARGB4444:
		// source is ARGB4444
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB4444) {
			// destination is ARGB4444
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_argb4444_to_argb4444(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb4444_to_argb4444(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_coloralpha_argb4444_to_argb4444(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y,
							this->config.color.a);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb4444_to_rgb32(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_coloralpha_argb4444_to_rgb32(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y,
							this->config.color.a);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_BGR555:
		// source is BGR555
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_BGR555) {
			// destination is BGR555
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_bgr555_to_bgr555(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_blit_blend_argb_to_bgr555(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							x, y);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	default:
		// does not match
		break;
	}


	// does not match
	return false;
}

bool MMSFBSurface::extendedAccelBlit(MMSFBSurface *source, MMSFBRectangle *src_rect, int x, int y) {
	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelBlitEx(source,
			                 NULL, MMSFB_PF_NONE, 0, 0,
			                 src_rect, x, y))
		return printMissingCombination("extendedAccelBlit()", source);
	else
		return true;
}

bool MMSFBSurface::extendedAccelBlitBuffer(MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
										   MMSFBRectangle *src_rect, int x, int y) {
	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelBlitEx(NULL,
							 src_planes, src_pixelformat, src_width, src_height,
			                 src_rect, x, y))
		return printMissingCombination("extendedAccelBlitBuffer()", NULL, src_planes, src_pixelformat, src_width, src_height);
	else
		return true;
}




bool MMSFBSurface::extendedAccelStretchBlitEx(MMSFBSurface *source,
											  MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
											  MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
											  MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
	MMSFBSurfacePlanes my_src_planes;
	if (source) {
		// premultiplied surface?
		if (!source->config.surface_buffer->premultiplied)
			return false;

		src_pixelformat = source->config.surface_buffer->pixelformat;
		src_width = (!source->root_parent)?source->config.w:source->root_parent->config.w;
		src_height = (!source->root_parent)?source->config.h:source->root_parent->config.h;

		// empty source planes
		memset(&my_src_planes, 0, sizeof(MMSFBSurfacePlanes));
		src_planes = &my_src_planes;
	}

	// a few help and clipping values
	MMSFBSurfacePlanes dst_planes;
	int sx = src_rect->x;
	int sy = src_rect->y;
	int sw = src_rect->w;
	int sh = src_rect->h;
	int dx = dest_rect->x;
	int dy = dest_rect->y;
	int dw = dest_rect->w;
	int dh = dest_rect->h;
	int wf;
	int hf;
	if (!calc_dest_rect) {
		// calc factor
		wf = (dw<<16)/sw;
		hf = (dh<<16)/sh;
	}
	else {
		// have to calculate accurate factor based on surface dimensions
		wf = (this->config.w<<16)/src_width;
		hf = (this->config.h<<16)/src_height;
	}

//printf("sx=%d,sy=%d,sw=%d,sh=%d,dx=%d,dy=%d,dw=%d,dh=%d\n", sx,sy,sw,sh,dx,dy,dw,dh);


	MMSFBRegion clipreg;
#ifndef USE_DFB_SUBSURFACE
	if (!this->is_sub_surface) {
#endif
		// normal surface or dfb subsurface
		if (!this->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->config.w - 1;
			clipreg.y2 = this->config.h - 1;
		}
		else
			clipreg = this->config.clip;
#ifndef USE_DFB_SUBSURFACE
	}
	else {
		// subsurface
		if (!this->root_parent->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->root_parent->config.w - 1;
			clipreg.y2 = this->root_parent->config.h - 1;
		}
		else
			clipreg = this->root_parent->config.clip;
	}
#endif

//printf("cx1=%d,cy1=%d,cx2=%d,cy2=%d\n", clipreg.x1,clipreg.y1,clipreg.x2,clipreg.y2);


	if (dx < clipreg.x1) {
		// left outside
		sx+= ((clipreg.x1 - dx)<<16) / wf;
/*		sw-= (clipreg.x1 - dx) / wf;
		if (sw <= 0)
			return true;*/
		dw-= clipreg.x1 - dx;
		if (dw <= 0)
			return true;
		sw = (dw<<16) / wf;
		dx = clipreg.x1;
	}
	else
	if (dx > clipreg.x2)
		// right outside
		return true;
	if (dy < clipreg.y1) {
		// top outside
		sy+= ((clipreg.y1 - dy)<<16) / hf;
/*		sh-= (clipreg.y1 - dy) / hf;
		if (sh <= 0)
			return true;*/
		dh-= clipreg.y1 - dy;
		if (dh <= 0)
			return true;
		sh = (dh<<16) / hf;
		dy = clipreg.y1;
	}
	else
	if (dy > clipreg.y2)
		// bottom outside
		return true;
	if (dx + dw - 1 > clipreg.x2) {
		// to width
		dw = clipreg.x2 - dx + 1;
		sw = (dw<<16) / wf;
	}
	if (dy + dh - 1 > clipreg.y2) {
		// to height
		dh = clipreg.y2 - dy + 1;
		sh = (dh<<16) / hf;
	}
	if (sw<=0) sw = 1;
	if (sh<=0) sh = 1;
	if (dw<=0) dw = 1;
	if (dh<=0) dh = 1;

	if (calc_dest_rect) {
		// signal the following routines, that stretch factors have to based on surface dimensions
		dw=0;
		dh=0;
	}

//printf(">sx=%d,sy=%d,sw=%d,sh=%d,dx=%d,dy=%d,dw=%d,dh=%d\n", sx,sy,sw,sh,dx,dy,dw,dh);

//if (source->is_sub_surface) {
//	sx+=source->sub_surface_xoff;
//	sy+=source->sub_surface_yoff;
//}

//printf("!sx=%d,sy=%d,sw=%d,sh=%d,dx=%d,dy=%d,dw=%d,dh=%d\n", sx,sy,sw,sh,dx,dy,dw,dh);

//dy-=10;


	// extract antialiasing flag from blittingflags
	bool antialiasing = (this->config.blittingflags & MMSFB_BLIT_ANTIALIASING);
	MMSFBBlittingFlags blittingflags = this->config.blittingflags & ~MMSFB_BLIT_ANTIALIASING;


	// checking pixelformats...
	switch (src_pixelformat) {
	case MMSFB_PF_ARGB:
		// source is ARGB
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB) {
			// destination is ARGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_argb_to_argb(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							antialiasing);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_blend_argb_to_argb(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_blend_coloralpha_argb_to_argb(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							this->config.color.a);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_RGB32:
		// source is RGB32
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_rgb32_to_rgb32(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							antialiasing);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_RGB24:
		// source is RGB24
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB) {
			// destination is ARGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_rgb24_to_argb(
							src_planes, src_height,
							sx, sy, sw, sh,
							(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							antialiasing);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}

			// does not match
			return false;
		}
		else
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB32) {
			// destination is RGB32
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_rgb24_to_rgb32(
							src_planes, src_height,
							sx, sy, sw, sh,
							(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							antialiasing);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_AiRGB:
		// source is AiRGB
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_AiRGB) {
			// destination is AiRGB
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_airgb_to_airgb(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							antialiasing);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_blend_airgb_to_airgb(
							src_planes, src_height,
							sx, sy, sw, sh,
							(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_blend_coloralpha_airgb_to_airgb(
							src_planes, src_height,
							sx, sy, sw, sh,
							(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							this->config.color.a);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_AYUV:
		// source is AYUV
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_AYUV) {
			// destination is AYUV
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_NOFX) {
				// blitting without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_ayuv_to_ayuv(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							antialiasing);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}
			else
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_blend_ayuv_to_ayuv(
							src_planes, src_height,
							sx, sy, sw, sh,
							(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_blend_coloralpha_ayuv_to_ayuv(
							src_planes, src_height,
							sx, sy, sw, sh,
							(unsigned int *)dst_planes.ptr, dst_planes.pitch, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							this->config.color.a);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_YV12:
		// source is YV12
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// stretch without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_yv12_to_yv12(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							antialiasing);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_I420:
		// source is I420
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// stretch without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_i420_to_yv12(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							antialiasing);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_YUY2:
		// source is YUY2
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
			// destination is YV12
			if   ((blittingflags == MMSFB_BLIT_NOFX)
				||(blittingflags == MMSFB_BLIT_BLEND_ALPHACHANNEL)) {
				// stretch without alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_yuy2_to_yv12(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							antialiasing);
					extendedUnlock(source, this);
					return true;
				}

				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_ARGB4444:
		// source is ARGB4444
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_ARGB4444) {
			// destination is ARGB4444
			if (blittingflags == (MMSFBBlittingFlags)MMSFB_BLIT_BLEND_ALPHACHANNEL) {
				// blitting with alpha channel
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_blend_argb4444_to_argb4444(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}
			else
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA|MMSFB_BLIT_SRC_PREMULTCOLOR))) {
				// blitting with alpha channel and coloralpha
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_blend_coloralpha_argb4444_to_argb4444(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							this->config.color.a);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_RGB16:
		// source is RGB16
		if (this->config.surface_buffer->pixelformat == MMSFB_PF_RGB16) {
			// destination is RGB16
			if   ((blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_NOFX))
				||(blittingflags == (MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL))) {
				if (extendedLock(source, src_planes, this, &dst_planes)) {
					mmsfb_stretchblit_rgb16_to_rgb16(
							src_planes, src_height,
							sx, sy, sw, sh,
							&dst_planes, (!this->root_parent)?this->config.h:this->root_parent->config.h,
							dx, dy, dw, dh,
							antialiasing);
					extendedUnlock(source, this);

					return true;
				}
				return false;
			}

			// does not match
			return false;
		}

		// does not match
		return false;
    default:
    	break;
	}

	// does not match
	return false;
}


bool MMSFBSurface::extendedAccelStretchBlit(MMSFBSurface *source, MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
											MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelStretchBlitEx(source,
			                        NULL, MMSFB_PF_NONE, 0, 0,
			                        src_rect, dest_rect,
			                        real_dest_rect, calc_dest_rect))
		return printMissingCombination("extendedAccelStretchBlit()", source);
	else
		return true;
}

bool MMSFBSurface::extendedAccelStretchBlitBuffer(MMSFBSurfacePlanes *src_planes, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
												  MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
												  MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelStretchBlitEx(NULL,
							        src_planes, src_pixelformat, src_width, src_height,
							        src_rect, dest_rect,
							        real_dest_rect, calc_dest_rect))
		return printMissingCombination("extendedAccelStretchBlitBuffer()", NULL, src_planes, src_pixelformat, src_width, src_height);
	else
		return true;
}




bool MMSFBSurface::extendedAccelFillRectangleEx(int x, int y, int w, int h) {

	// a few help and clipping values
	MMSFBSurfacePlanes dst_planes;
	int sx = x;
	int sy = y;
	int sw = w;
	int sh = h;
	MMSFBRegion clipreg;
	int dst_height = (!this->root_parent)?this->config.h:this->root_parent->config.h;


#ifndef USE_DFB_SUBSURFACE
	if (!this->is_sub_surface) {
#endif
		// normal surface or dfb subsurface
		if (!this->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->config.w - 1;
			clipreg.y2 = this->config.h - 1;
		}
		else
			clipreg = this->config.clip;
#ifndef USE_DFB_SUBSURFACE
	}
	else {
		// subsurface
		if (!this->root_parent->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->root_parent->config.w - 1;
			clipreg.y2 = this->root_parent->config.h - 1;
		}
		else
			clipreg = this->root_parent->config.clip;
	}
#endif

	if (x < clipreg.x1) {
		// left outside
		sx+= clipreg.x1 - x;
		sw-= clipreg.x1 - x;
		if (sw <= 0)
			return true;
		x = clipreg.x1;
	}
	else
	if (x > clipreg.x2)
		// right outside
		return true;
	if (y < clipreg.y1) {
		// top outside
		sy+= clipreg.y1 - y;
		sh-= clipreg.y1 - y;
		if (sh <= 0)
			return true;
		y = clipreg.y1;
	}
	else
	if (y > clipreg.y2)
		// bottom outside
		return true;
	if (x + sw - 1 > clipreg.x2)
		// to width
		sw = clipreg.x2 - x + 1;
	if (y + sh - 1 > clipreg.y2)
		// to height
		sh = clipreg.y2 - y + 1;

	// calculate the color
	MMSFBColor color = this->config.color;
	if (this->config.drawingflags & (MMSFBDrawingFlags)MMSFB_DRAW_SRC_PREMULTIPLY) {
		// pre-multiplication needed
		if (color.a != 0xff) {
			color.r = ((color.a+1) * color.r) >> 8;
			color.g = ((color.a+1) * color.g) >> 8;
			color.b = ((color.a+1) * color.b) >> 8;
		}
	}

	// checking pixelformats...
	switch (this->config.surface_buffer->pixelformat) {
	case MMSFB_PF_ARGB:
		// destination is ARGB
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_argb(&dst_planes, dst_height,
										 sx, sy, sw, sh, color);
				extendedUnlock(NULL, this, &dst_planes);
				return true;
			}

			return false;
		}
		else
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing with alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_blend_argb(&dst_planes, dst_height,
											   sx, sy, sw, sh, color);
				extendedUnlock(NULL, this, &dst_planes);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_AYUV:
		// destination is AYUV
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_ayuv(&dst_planes, dst_height,
										 sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}
		else
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing with alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_blend_ayuv(&dst_planes, dst_height,
											   sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_RGB32:
		// destination is RGB32
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_rgb32(&dst_planes, dst_height,
										  sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_RGB24:
		// destination is RGB24
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_rgb24(&dst_planes, dst_height,
										  sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_RGB16:
		// destination is RGB16 (RGB565)
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_rgb16(
						&dst_planes, dst_height,
						sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}
		else
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing with alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_blend_rgb16(
						&dst_planes, dst_height,
						sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_YV12:
		// destination is YV12
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_yv12(&dst_planes, dst_height,
										 sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_I420:
		// destination is I420
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_i420(&dst_planes, dst_height,
										 sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_YUY2:
		// destination is YUY2
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_yuy2(&dst_planes, dst_height,
										 sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_ARGB3565:
		// destination is ARGB3565
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_argb3565(&dst_planes, dst_height,
										     sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_ARGB4444:
		// destination is ARGB4444
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_argb4444(
						&dst_planes, dst_height,
						sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}
		else
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing with alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_blend_argb4444(
						&dst_planes, dst_height,
						sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_BGR24:
		// destination is BGR24
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_bgr24(&dst_planes, dst_height,
										  sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	case MMSFB_PF_BGR555:
		// destination is BGR555
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_fillrectangle_bgr555(&dst_planes, dst_height,
										   sx, sy, sw, sh, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	default:
		// does not match
		break;
	}

	// does not match
	return false;
}


bool MMSFBSurface::extendedAccelFillRectangle(int x, int y, int w, int h) {

	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelFillRectangleEx(x, y, w, h))
		return printMissingCombination("extendedAccelFillRectangle()");
	else
		return true;
}




bool MMSFBSurface::extendedAccelDrawLineEx(int x1, int y1, int x2, int y2) {

	// check if we can use fill rectangle
	if (x1 == x2)
		return extendedAccelFillRectangle(x1, y1, 1, y2-y1+1);
	else
	if (y1 == y2)
		return extendedAccelFillRectangle(x1, y1, x2-x1+1, 1);

	// a few help and clipping values
	MMSFBSurfacePlanes dst_planes;
	MMSFBRegion clipreg;
	int dst_height = (!this->root_parent)?this->config.h:this->root_parent->config.h;

#ifndef USE_DFB_SUBSURFACE
	if (!this->is_sub_surface) {
#endif
		// normal surface or dfb subsurface
		if (!this->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->config.w - 1;
			clipreg.y2 = this->config.h - 1;
		}
		else
			clipreg = this->config.clip;
#ifndef USE_DFB_SUBSURFACE
	}
	else {
		// subsurface
		if (!this->root_parent->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->root_parent->config.w - 1;
			clipreg.y2 = this->root_parent->config.h - 1;
		}
		else
			clipreg = this->root_parent->config.clip;
	}
#endif

	// calculate the color
	MMSFBColor color = this->config.color;
	if (this->config.drawingflags & (MMSFBDrawingFlags)MMSFB_DRAW_SRC_PREMULTIPLY) {
		// pre-multiplication needed
		if (color.a != 0xff) {
			color.r = ((color.a+1) * color.r) >> 8;
			color.g = ((color.a+1) * color.g) >> 8;
			color.b = ((color.a+1) * color.b) >> 8;
		}
	}

	// checking pixelformats...
	switch (this->config.surface_buffer->pixelformat) {
	case MMSFB_PF_ARGB:
		// destination is ARGB
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_drawline_argb(&dst_planes, dst_height, clipreg, x1, y1, x2, y2, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}
		else
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				// drawing with alpha channel
				mmsfb_drawline_blend_argb(&dst_planes, dst_height, clipreg, x1, y1, x2, y2, color);
				extendedUnlock(NULL, this);
				return true;
			}
		}

		// does not match
		return false;

	case MMSFB_PF_ARGB4444:
		// destination is ARGB4444
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			| (this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			// drawing without alpha channel
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_drawline_argb4444(&dst_planes, dst_height, clipreg, x1, y1, x2, y2, color);
				extendedUnlock(NULL, this);
				return true;
			}

			return false;
		}

		// does not match
		return false;

	default:
		// does not match
		break;
	}

	// does not match
	return false;
}


bool MMSFBSurface::extendedAccelDrawLine(int x1, int y1, int x2, int y2) {

	// extended acceleration on?
	if (!this->extendedaccel)
		return false;

	if (!extendedAccelDrawLineEx(x1, y1, x2, y2))
		return printMissingCombination("extendedAccelDrawLine()");
	else
		return true;
}





bool MMSFBSurface::blit(MMSFBSurface *source, MMSFBRectangle *src_rect, int x, int y) {
    MMSFBRectangle src;
    bool 		 ret = false;

    /* check if initialized */
    INITCHECK;

    if (src_rect) {
         src = *src_rect;
    }
    else {
         src.x = 0;
         src.y = 0;
         src.w = source->config.w;
         src.h = source->config.h;
    }

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult    dfbres;
	    D_DEBUG_AT( MMS_Surface, "blit( %d,%d - %dx%d -> %d,%d ) <- %dx%d\n",
	                DFB_RECTANGLE_VALS(&srcr), x, y, this->config.w, this->config.h );
	    MMSFB_TRACE();

#ifndef USE_DFB_SUBSURFACE
		// prepare source rectangle
		if (source->is_sub_surface) {
			src.x+=source->sub_surface_xoff;
			src.y+=source->sub_surface_yoff;
		}
#endif

	    // blit
		if (!this->is_sub_surface) {
			if (!extendedAccelBlit(source, &src, x, y))
				if ((dfbres=this->llsurface->Blit(this->llsurface, (IDirectFBSurface *)source->getDFBSurface(), (DFBRectangle*)&src, x, y)) != DFB_OK) {
#ifndef USE_DFB_SUBSURFACE
					// reset source rectangle
					if (source->is_sub_surface) {
						src.x-=source->sub_surface_xoff;
						src.y-=source->sub_surface_yoff;
					}
#endif
					MMSFB_SetError(dfbres, "IDirectFBSurface::Blit() failed, src rect="
										   +iToStr(src.x)+","+iToStr(src.y)+","+iToStr(src.w)+","+iToStr(src.h)
										   +", dst="+iToStr(x)+","+iToStr(y));
					return false;
				}
			ret = true;
		}
		else {

#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			x+=this->sub_surface_xoff;
			y+=this->sub_surface_yoff;

			SETSUBSURFACE_BLITTINGFLAGS;
#endif

			if (extendedAccelBlit(source, &src, x, y))
				ret = true;
			else
				if (this->llsurface->Blit(this->llsurface, (IDirectFBSurface *)source->getDFBSurface(), (DFBRectangle*)&src, x, y) == DFB_OK)
					ret = true;

#ifndef USE_DFB_SUBSURFACE
			RESETSUBSURFACE_BLITTINGFLAGS;

			UNCLIPSUBSURFACE
#endif

		}

#ifndef USE_DFB_SUBSURFACE
		// reset source rectangle
		if (source->is_sub_surface) {
			src.x-=source->sub_surface_xoff;
			src.y-=source->sub_surface_yoff;
		}
#endif

#endif
	}
	else {

		// prepare source rectangle
		if (source->is_sub_surface) {
			src.x+=source->sub_surface_xoff;
			src.y+=source->sub_surface_yoff;
		}

		// blit
		if (!this->is_sub_surface) {
			ret = extendedAccelBlit(source, &src, x, y);
		}
		else {
			CLIPSUBSURFACE

			x+=this->sub_surface_xoff;
			y+=this->sub_surface_yoff;

			ret = extendedAccelBlit(source, &src, x, y);

			UNCLIPSUBSURFACE
		}

		// reset source rectangle
		if (source->is_sub_surface) {
			src.x-=source->sub_surface_xoff;
			src.y-=source->sub_surface_yoff;
		}
	}

    return ret;
}


bool MMSFBSurface::blitBuffer(MMSFBExternalSurfaceBuffer *extbuf, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
							  MMSFBRectangle *src_rect, int x, int y) {
    MMSFBRectangle src;
    bool 		 ret = false;

    if (src_rect) {
         src = *src_rect;
    }
    else {
         src.x = 0;
         src.y = 0;
         src.w = src_width;
         src.h = src_height;
    }

    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
#endif
	}
	else {
		/* blit buffer */
		if (!this->is_sub_surface) {
			ret = extendedAccelBlitBuffer(extbuf, src_pixelformat, src_width, src_height, &src, x, y);
		}
		else {
			CLIPSUBSURFACE

			x+=this->sub_surface_xoff;
			y+=this->sub_surface_yoff;

			ret = extendedAccelBlitBuffer(extbuf, src_pixelformat, src_width, src_height, &src, x, y);

			UNCLIPSUBSURFACE

		}
	}

    return ret;
}

bool MMSFBSurface::blitBuffer(void *src_ptr, int src_pitch, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
							  MMSFBRectangle *src_rect, int x, int y) {
	MMSFBExternalSurfaceBuffer extbuf;
	memset(&extbuf, 0, sizeof(extbuf));
	extbuf.ptr = src_ptr;
	extbuf.pitch = src_pitch;
	return blitBuffer(&extbuf, src_pixelformat, src_width, src_height, src_rect, x, y);
}

bool MMSFBSurface::stretchBlit(MMSFBSurface *source, MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
							   MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
    MMSFBRectangle src;
    MMSFBRectangle dst;
    bool 		 ret = false;

    // use whole source surface if src_rect is not given
    if (src_rect) {
         src = *src_rect;
    }
    else {
         src.x = 0;
         src.y = 0;
         src.w = source->config.w;
         src.h = source->config.h;
    }

    // use whole destination surface if dest_rect is not given and calc_dest_rect is not set
    if ((dest_rect)&&(!calc_dest_rect)) {
         dst = *dest_rect;
    }
    else {
    	if (!calc_dest_rect) {
			dst.x = 0;
			dst.y = 0;
			dst.w = this->config.w;
			dst.h = this->config.h;
    	}
    	else {
    		// calc dest_rect from src_rect based on src/dst surface dimension
    		dst.x = (src.x * this->config.w) / source->config.w;
    		dst.y = (src.y * this->config.h) / source->config.h;
    		dst.w = src.x + src.w - 1;
    		dst.h = src.y + src.h - 1;
    		dst.w = (dst.w * this->config.w) / source->config.w;
    		dst.h = (dst.h * this->config.h) / source->config.h;
    		dst.w = dst.w - dst.x + 1;
    		dst.h = dst.h - dst.y + 1;
    	}
    }

    // return the used dest_rect
    if (real_dest_rect)
    	*real_dest_rect = dst;

    /* check if i can blit without stretching */
    if (src.w == dst.w && src.h == dst.h)
        return blit(source, &src, dst.x, dst.y);

    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult    dfbres;
	    bool         blit_done = false;
	    D_DEBUG_AT( MMS_Surface, "stretchBlit( %d,%d - %dx%d  ->  %d,%d - %dx%d ) <- %dx%d\n",
	                DFB_RECTANGLE_VALS(&src), DFB_RECTANGLE_VALS(&dst), this->config.w, this->config.h);
	    MMSFB_BREAK();

#ifndef USE_DFB_SUBSURFACE
		// prepare source rectangle
		if (source->is_sub_surface) {
			src.x+=source->sub_surface_xoff;
			src.y+=source->sub_surface_yoff;
		}
#endif

		if (this->config.blittingflags != MMSFB_BLIT_NOFX) {
			/* stretch blit with blitting flags */

			if (!this->is_sub_surface) {
				if (extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect)) {
					blit_done = true;
					ret = true;
				}
			}
			else {

#ifndef USE_DFB_SUBSURFACE
				CLIPSUBSURFACE

				dst.x+=this->sub_surface_xoff;
				dst.y+=this->sub_surface_yoff;

				SETSUBSURFACE_BLITTINGFLAGS;
#endif

				if (extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect)) {
					blit_done = true;
					ret = true;
				}


#ifndef USE_DFB_SUBSURFACE
				RESETSUBSURFACE_BLITTINGFLAGS;

				dst.x-=this->sub_surface_xoff;
				dst.y-=this->sub_surface_yoff;

				UNCLIPSUBSURFACE
#endif
			}

			if (!blit_done) {
				/* we use a temporary surface to separate the stretchblit from the extra blitting functions */
				MMSFBSurface *tempsuf = mmsfbsurfacemanager->getTemporarySurface(dst.w, dst.h);

				if (tempsuf) {
					MMSFBRectangle temp;
					temp.x=0;
					temp.y=0;
					temp.w=dst.w;
					temp.h=dst.h;

					dfbres = DFB_OK;
					dfbres=((IDirectFBSurface *)tempsuf->getDFBSurface())->StretchBlit((IDirectFBSurface *)tempsuf->getDFBSurface(), (IDirectFBSurface *)source->getDFBSurface(), (DFBRectangle*)&src, (DFBRectangle*)&temp);
					if (dfbres == DFB_OK) {
						if (!this->is_sub_surface) {
							if (extendedAccelBlit(tempsuf, &temp, dst.x, dst.y)) {
								blit_done = true;
								ret = true;
							}
							else
							if ((dfbres=this->llsurface->Blit(this->llsurface, (IDirectFBSurface *)tempsuf->getDFBSurface(), (DFBRectangle*)&temp, dst.x, dst.y)) == DFB_OK) {
								blit_done = true;
								ret = true;
							}
						}
						else {

#ifndef USE_DFB_SUBSURFACE
							CLIPSUBSURFACE

							dst.x+=this->sub_surface_xoff;
							dst.y+=this->sub_surface_yoff;

							SETSUBSURFACE_BLITTINGFLAGS;
#endif

							if (!extendedAccelBlit(tempsuf, &temp, dst.x, dst.y))
								this->llsurface->Blit(this->llsurface, (IDirectFBSurface *)tempsuf->getDFBSurface(), (DFBRectangle*)&temp, dst.x, dst.y);

#ifndef USE_DFB_SUBSURFACE
							RESETSUBSURFACE_BLITTINGFLAGS;

							dst.x-=this->sub_surface_xoff;
							dst.y-=this->sub_surface_yoff;

							UNCLIPSUBSURFACE
#endif

							blit_done = true;
							ret = true;

						}
					}

					mmsfbsurfacemanager->releaseTemporarySurface(tempsuf);
				}
			}
		}

		if (!blit_done) {
			// normal stretch blit
			if (!this->is_sub_surface) {
				dfbres = DFB_OK;
				if (!extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect))
					dfbres=this->llsurface->StretchBlit(this->llsurface, (IDirectFBSurface *)source->getDFBSurface(), (DFBRectangle*)&src, (DFBRectangle*)&dst);
				if (dfbres != DFB_OK) {
#ifndef USE_DFB_SUBSURFACE
					// reset source rectangle
					if (source->is_sub_surface) {
						src.x-=source->sub_surface_xoff;
						src.y-=source->sub_surface_yoff;
					}
#endif
					MMSFB_SetError(dfbres, "IDirectFBSurface::StretchBlit() failed");
					return false;
				}
				ret = true;
			}
			else {

#ifndef USE_DFB_SUBSURFACE
				CLIPSUBSURFACE

				dst.x+=this->sub_surface_xoff;
				dst.y+=this->sub_surface_yoff;

				SETSUBSURFACE_BLITTINGFLAGS;
#endif

				if (!extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect))
					this->llsurface->StretchBlit(this->llsurface, (IDirectFBSurface *)source->getDFBSurface(), (DFBRectangle*)&src, (DFBRectangle*)&dst);
				ret = true;

#ifndef USE_DFB_SUBSURFACE
				RESETSUBSURFACE_BLITTINGFLAGS;

				dst.x-=this->sub_surface_xoff;
				dst.y-=this->sub_surface_yoff;

				UNCLIPSUBSURFACE
#endif
			}
		}

#ifndef USE_DFB_SUBSURFACE
		// reset source rectangle
		if (source->is_sub_surface) {
			src.x-=source->sub_surface_xoff;
			src.y-=source->sub_surface_yoff;
		}
#endif

#endif
	}
	else {

		// prepare source rectangle
		if (source->is_sub_surface) {
			src.x+=source->sub_surface_xoff;
			src.y+=source->sub_surface_yoff;
		}

		// normal stretch blit
		if (!this->is_sub_surface) {
			ret = extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect);
		}
		else {
			CLIPSUBSURFACE

			dst.x+=this->sub_surface_xoff;
			dst.y+=this->sub_surface_yoff;

			ret = extendedAccelStretchBlit(source, &src, &dst, real_dest_rect, calc_dest_rect);

			dst.x-=this->sub_surface_xoff;
			dst.y-=this->sub_surface_yoff;

			UNCLIPSUBSURFACE
		}

		// reset source rectangle
		if (source->is_sub_surface) {
			src.x-=source->sub_surface_xoff;
			src.y-=source->sub_surface_yoff;
		}
	}

    return ret;
}

bool MMSFBSurface::stretchBlitBuffer(MMSFBExternalSurfaceBuffer *extbuf, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
									 MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
									 MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
    MMSFBRectangle src;
    MMSFBRectangle dst;
    bool ret = false;

    // use whole source surface if src_rect is not given
    if (src_rect) {
         src = *src_rect;
    }
    else {
         src.x = 0;
         src.y = 0;
         src.w = src_width;
         src.h = src_height;
    }

    // use whole destination surface if dest_rect is not given and calc_dest_rect is not set
    if ((dest_rect)&&(!calc_dest_rect)) {
         dst = *dest_rect;
    }
    else {
    	if (!calc_dest_rect) {
			dst.x = 0;
			dst.y = 0;
			dst.w = this->config.w;
			dst.h = this->config.h;
    	}
    	else {
    		// calc dest_rect from src_rect based on src/dst surface dimension
    		dst.x = (src.x * this->config.w) / src_width;
    		dst.y = (src.y * this->config.h) / src_height;
    		dst.w = src.x + src.w - 1;
    		dst.h = src.y + src.h - 1;
    		dst.w = (dst.w * this->config.w) / src_width;
    		dst.h = (dst.h * this->config.h) / src_height;
    		dst.w = dst.w - dst.x + 1;
    		dst.h = dst.h - dst.y + 1;
    	}
    }

    // return the used dest_rect
    if (real_dest_rect)
    	*real_dest_rect = dst;

    // check if i can blit without stretching
    if (src.w == dst.w && src.h == dst.h)
        return blitBuffer(extbuf->ptr, extbuf->pitch, src_pixelformat, src_width, src_height, &src, dst.x, dst.y);

    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
#endif
	}
	else {
		/* normal stretch blit */
		if (!this->is_sub_surface) {
			ret = extendedAccelStretchBlitBuffer(extbuf, src_pixelformat, src_width, src_height, &src, &dst,
												 real_dest_rect, calc_dest_rect);
		}
		else {
			CLIPSUBSURFACE

			dst.x+=this->sub_surface_xoff;
			dst.y+=this->sub_surface_yoff;

			ret = extendedAccelStretchBlitBuffer(extbuf, src_pixelformat, src_width, src_height, &src, &dst,
												 real_dest_rect, calc_dest_rect);

			dst.x-=this->sub_surface_xoff;
			dst.y-=this->sub_surface_yoff;

			UNCLIPSUBSURFACE
		}

	}

    return ret;
}

bool MMSFBSurface::stretchBlitBuffer(void *src_ptr, int src_pitch, MMSFBSurfacePixelFormat src_pixelformat, int src_width, int src_height,
									 MMSFBRectangle *src_rect, MMSFBRectangle *dest_rect,
									 MMSFBRectangle *real_dest_rect, bool calc_dest_rect) {
	MMSFBExternalSurfaceBuffer extbuf;
	memset(&extbuf, 0, sizeof(extbuf));
	extbuf.ptr = src_ptr;
	extbuf.pitch = src_pitch;
	return stretchBlitBuffer(&extbuf, src_pixelformat, src_width, src_height, src_rect, dest_rect,
						     real_dest_rect, calc_dest_rect);
}



bool MMSFBSurface::flip(MMSFBRegion *region) {

    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    if (region)
	         D_DEBUG_AT( MMS_Surface, "flip( %d,%d - %dx%d ) <- %dx%d\n",
	                     DFB_RECTANGLE_VALS_FROM_REGION(region), this->config.w, this->config.h );
	    else
	         D_DEBUG_AT( MMS_Surface, "flip( %d,%d - %dx%d ) <- %dx%d\n",
	                     0, 0, this->config.w, this->config.h, this->config.w, this->config.h );

	    MMSFB_TRACE();

	    DFBResult   dfbres;

#ifdef USE_DFB_WINMAN

		/* flip */
		if ((dfbres=this->llsurface->Flip(this->llsurface, region, this->flipflags)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::Flip() failed");

			return false;
		}

#endif

#ifdef USE_MMSFB_WINMAN

		/* flip */
		if (!this->is_sub_surface) {
			if ((dfbres=this->llsurface->Flip(this->llsurface, (DFBRegion*)region, getDFBSurfaceFlipFlagsFromMMSFBFlipFlags(this->flipflags))) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::Flip() failed");

				return false;
			}
		}
		else {
#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			MMSFBRegion myregion;
			if (!region) {
				myregion.x1 = 0;
				myregion.y1 = 0;
				myregion.x2 = this->config.w - 1;
				myregion.y2 = this->config.h - 1;
			}
			else
				myregion = *region;

			myregion.x1+=this->sub_surface_xoff;
			myregion.y1+=this->sub_surface_yoff;
			myregion.x2+=this->sub_surface_xoff;
			myregion.y2+=this->sub_surface_yoff;

			this->llsurface->Flip(this->llsurface, (DFBRegion*)&myregion, getDFBSurfaceFlipFlagsFromMMSFBFlipFlags(this->flipflags));

#else
			this->llsurface->Flip(this->llsurface, region, getDFBSurfaceFlipFlagsFromMMSFBFlipFlags(this->flipflags));
#endif

#ifndef USE_DFB_SUBSURFACE
			UNCLIPSUBSURFACE
#endif
		}

		if (this->config.iswinsurface) {
			// inform the window manager
			mmsfbwindowmanager->flipSurface(this, region);
		}
		else {
	    	if (this->is_sub_surface) {
				// sub surface, use the root parent surface
	    		if (this->root_parent->config.iswinsurface) {
	    			// inform the window manager, use the correct region
	    			MMSFBRegion reg;
	    			if (region)
	    				reg = *region;
	    			else {
	    				reg.x1=0;
	    				reg.y1=0;
	    				reg.x2=sub_surface_rect.w-1;
	    				reg.y2=sub_surface_rect.h-1;
	    			}
	    			reg.x1+=this->sub_surface_xoff;
	    			reg.y1+=this->sub_surface_yoff;
	    			reg.x2+=this->sub_surface_xoff;
	    			reg.y2+=this->sub_surface_yoff;
	    			mmsfbwindowmanager->flipSurface(this->root_parent, &reg);
	    		}
			}
		}

#endif

		return true;
#else
		return false;
#endif
	}
	else {
		// flip my own surfaces
		MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
#ifdef __HAVE_FBDEV__
		if ((sb->numbuffers > 1) && ((!sb->mmsfbdev_surface) || (sb->mmsfbdev_surface != this))) {
			// backbuffer in video memory, so we do not have to flip here
#else
		if (sb->numbuffers > 1) {
#endif
			// flip is only needed, if we have at least one backbuffer
			if (!this->is_sub_surface) {
				// not a subsurface
				if (!region) {
					// flip my buffers without blitting
					sb->currbuffer_read++;
					if (sb->currbuffer_read >= sb->numbuffers)
						sb->currbuffer_read = 0;
					sb->currbuffer_write++;
					if (sb->currbuffer_write >= sb->numbuffers)
						sb->currbuffer_write = 0;
				}
				else {
					MMSFBRectangle src_rect;
					src_rect.x = region->x1;
					src_rect.y = region->y1;
					src_rect.w = region->x2 - region->x1 + 1;
					src_rect.h = region->y2 - region->y1 + 1;

					// check if region is equal to the whole surface
					if   ((src_rect.x == 0) && (src_rect.y == 0)
						&&(src_rect.w == this->config.w) && (src_rect.h == this->config.h)) {
						// yes, flip my buffers without blitting
						sb->currbuffer_read++;
						if (sb->currbuffer_read >= sb->numbuffers)
							sb->currbuffer_read = 0;
						sb->currbuffer_write++;
						if (sb->currbuffer_write >= sb->numbuffers)
							sb->currbuffer_write = 0;
					}
					else {
						// blit region from write to read buffer of the same MMSFBSurface
						MMSFBBlittingFlags savedbf = this->config.blittingflags;
						this->config.blittingflags = (MMSFBBlittingFlags)MMSFB_BLIT_NOFX;

						this->surface_invert_lock = true;
						this->extendedAccelBlit(this, &src_rect, src_rect.x, src_rect.y);
						this->surface_invert_lock = false;

						this->config.blittingflags = savedbf;
					}
				}
			}
			else {
				CLIPSUBSURFACE

				MMSFBRectangle src_rect;
				if (!region) {
					src_rect.x = 0;
					src_rect.y = 0;
					src_rect.w = this->config.w;
					src_rect.h = this->config.h;
				}
				else {
					src_rect.x = region->x1;
					src_rect.y = region->y1;
					src_rect.w = region->x2 - region->x1 + 1;
					src_rect.h = region->y2 - region->y1 + 1;
				}

				src_rect.x+=this->sub_surface_xoff;
				src_rect.y+=this->sub_surface_yoff;

				// blit region from write to read buffer of the same MMSFBSurface
				MMSFBBlittingFlags savedbf = this->config.blittingflags;
				this->config.blittingflags = (MMSFBBlittingFlags)MMSFB_BLIT_NOFX;

				this->surface_invert_lock = true;
				this->extendedAccelBlit(this, &src_rect, src_rect.x, src_rect.y);
				this->surface_invert_lock = false;

				this->config.blittingflags = savedbf;

				UNCLIPSUBSURFACE
			}
		}

#ifdef __HAVE_FBDEV__
		if (sb->mmsfbdev_surface) {
			if (sb->mmsfbdev_surface == this) {
				// this surface is the front and backbuffer in video memory of the layer
				// flip my buffers without blitting
				sb->currbuffer_read++;
				if (sb->currbuffer_read >= sb->numbuffers)
					sb->currbuffer_read = 0;
				sb->currbuffer_write++;
				if (sb->currbuffer_write >= sb->numbuffers)
					sb->currbuffer_write = 0;

				// do hardware panning
				mmsfb->mmsfbdev->panDisplay(sb->currbuffer_read, sb->buffers[0].ptr);
			}
			else {
				// this surface is the backbuffer in system memory of the layer

				// sync
				mmsfb->mmsfbdev->waitForVSync();

				// put the image to the framebuffer
				sb->mmsfbdev_surface->blit(this, NULL, 0, 0);
			}
		}
		else {
#endif

#ifdef __HAVE_XLIB__
		if (sb->x_image[0]) {
			// XSHM, put the image to the x-server
			if (!this->scaler) {
				// no scaler defined
				mmsfb->xlock.lock();
				XLockDisplay(mmsfb->x_display);
				if (!region) {
					// put whole image
					int dx = 0;
					int dy = 0;
					if (mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {
						dx = (mmsfb->display_w - this->config.w) >> 1;
						dy = (mmsfb->display_h - this->config.h) >> 1;
					}
					XShmPutImage(mmsfb->x_display, mmsfb->x_window, mmsfb->x_gc, sb->x_image[sb->currbuffer_read],
								  0, 0, dx, dy,
								  this->config.w, this->config.h, False);
					//XFlush(mmsfb->x_display);
					XSync(mmsfb->x_display, False);
				}
				else {
					// put only a region
					MMSFBRegion myreg = *region;
					if (myreg.x1 < 0) myreg.x1 = 0;
					if (myreg.y1 < 0) myreg.y1 = 0;
					if (myreg.x2 >= this->config.w) myreg.x2 = this->config.w - 1;
					if (myreg.y2 >= this->config.h) myreg.y2 = this->config.h - 1;
					if ((myreg.x2 >= myreg.x1)&&(myreg.y2 >= myreg.y1)) {
						int dx = 0;
						int dy = 0;
						if (mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {
							dx = (mmsfb->display_w - this->config.w) >> 1;
							dy = (mmsfb->display_h - this->config.h) >> 1;
						}
						XShmPutImage(mmsfb->x_display, mmsfb->x_window, mmsfb->x_gc, sb->x_image[sb->currbuffer_read],
									 myreg.x1, myreg.y1, myreg.x1 + dx, myreg.y1 + dy,
									 myreg.x2 - myreg.x1 + 1, myreg.y2 - myreg.y1 + 1, False);
						//XFlush(mmsfb->x_display);
						XSync(mmsfb->x_display, False);
					}
				}
				XUnlockDisplay(mmsfb->x_display);
				mmsfb->xlock.unlock();
			}
			else {
				// scale to scaler
				if (!region) {
					// scale whole image
					this->scaler->stretchBlit(this, NULL, NULL);
					this->scaler->flip();
				}
				else {
					// scale only a region
					MMSFBRegion myreg = *region;

					// enlarge the region because of little calulation errors while stretching
					myreg.x1--;
					myreg.y1--;
					myreg.x2++;
					myreg.y2++;

					// check region
					if (myreg.x1 < 0) myreg.x1 = 0;
					if (myreg.y1 < 0) myreg.y1 = 0;
					if (myreg.x2 >= this->config.w) myreg.x2 = this->config.w - 1;
					if (myreg.y2 >= this->config.h) myreg.y2 = this->config.h - 1;
					if ((myreg.x2 >= myreg.x1)&&(myreg.y2 >= myreg.y1)) {
						// stretch & flip to make it visible on the screen
						MMSFBRectangle src_rect;
						src_rect.x = myreg.x1;
						src_rect.y = myreg.y1;
						src_rect.w = myreg.x2 - myreg.x1 + 1;
						src_rect.h = myreg.y2 - myreg.y1 + 1;
						MMSFBRectangle dst_rect;
						this->scaler->stretchBlit(this, &src_rect, NULL, &dst_rect, true);
						myreg.x1 = dst_rect.x;
						myreg.y1 = dst_rect.y;
						myreg.x2 = dst_rect.x + dst_rect.w - 1;
						myreg.y2 = dst_rect.y + dst_rect.h - 1;
						this->scaler->flip(&myreg);
					}
				}
			}
		}
		else
		if (sb->xv_image[0]) {
			// XVSHM, put the image to the x-server
			mmsfb->xlock.lock();
			XLockDisplay(mmsfb->x_display);
			if (mmsfb->fullscreen == MMSFB_FSM_TRUE || mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {
				// calc ratio
				MMSFBRectangle dest;
				calcAspectRatio(mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h, mmsfb->display_w, mmsfb->display_h, dest,
								(mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO), true);

				// put image
				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, mmsfb->x_window, mmsfb->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  dest.x, dest.y, dest.w, dest.h, False);

			} else if(mmsfb->resized) {
				printf("stretch to %d:%d\n",mmsfb->target_window_w, mmsfb->target_window_h);
				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, mmsfb->x_window, mmsfb->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  0, 0, mmsfb->target_window_w, mmsfb->target_window_h, False);
			}else{
				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, mmsfb->x_window, mmsfb->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h, False);
			}
			//XFlush(mmsfb->x_display);
			XSync(mmsfb->x_display, False);
			XUnlockDisplay(mmsfb->x_display);
			mmsfb->xlock.unlock();
		}
#endif

#ifdef __HAVE_FBDEV__
		}
#endif

		if (this->config.iswinsurface) {
			// inform the window manager
			mmsfbwindowmanager->flipSurface(this, region);
		}
		else {
	    	if (this->is_sub_surface) {
				// sub surface, use the root parent surface
	    		if (this->root_parent->config.iswinsurface) {
	    			// inform the window manager, use the correct region
	    			MMSFBRegion reg;
	    			if (region)
	    				reg = *region;
	    			else {
	    				reg.x1=0;
	    				reg.y1=0;
	    				reg.x2=sub_surface_rect.w-1;
	    				reg.y2=sub_surface_rect.h-1;
	    			}
	    			reg.x1+=this->sub_surface_xoff;
	    			reg.y1+=this->sub_surface_yoff;
	    			reg.x2+=this->sub_surface_xoff;
	    			reg.y2+=this->sub_surface_yoff;
	    			mmsfbwindowmanager->flipSurface(this->root_parent, &reg);
	    		}
			}
		}

		return true;
	}
}

bool MMSFBSurface::refresh() {
    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
#endif
	}
	else {
#ifdef __HAVE_XLIB__
		MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
		if (sb->x_image[0]) {
			// XSHM, put the image to the x-server
			if (!this->scaler) {
				// no scaler defined
				mmsfb->xlock.lock();
				XLockDisplay(mmsfb->x_display);
				int dx = 0;
				int dy = 0;
				if (mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {
					dx = (mmsfb->display_w - this->config.w) >> 1;
					dy = (mmsfb->display_h - this->config.h) >> 1;
				}
				XShmPutImage(mmsfb->x_display, mmsfb->x_window, mmsfb->x_gc, sb->x_image[sb->currbuffer_read],
							  0, 0, dx, dy,
							  this->config.w, this->config.h, False);
				//XFlush(mmsfb->x_display);
				XSync(mmsfb->x_display, False);
				XUnlockDisplay(mmsfb->x_display);
				mmsfb->xlock.unlock();
			}
			else {
				// scale to scaler
				this->scaler->stretchBlit(this, NULL, NULL);
				this->scaler->flip();
			}
		}
		else
		if (sb->xv_image[0]) {
			// XVSHM, put the image to the x-server
			this->lock();
			mmsfb->xlock.lock();
			XLockDisplay(mmsfb->x_display);
			if (mmsfb->fullscreen == MMSFB_FSM_TRUE || mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO) {
				// calc ratio
				MMSFBRectangle dest;
				calcAspectRatio(mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h, mmsfb->display_w, mmsfb->display_h, dest,
								(mmsfb->fullscreen == MMSFB_FSM_ASPECT_RATIO), true);

				// put image
				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, mmsfb->x_window, mmsfb->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  dest.x, dest.y, dest.w, dest.h, False);
			} else if(mmsfb->resized) {
				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, mmsfb->x_window, mmsfb->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  0, 0, mmsfb->target_window_w, mmsfb->target_window_h, False);
			}else{
				XvShmPutImage(mmsfb->x_display, mmsfb->xv_port, mmsfb->x_window, mmsfb->x_gc, sb->xv_image[sb->currbuffer_read],
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h,
							  0, 0, mmsfb->x11_win_rect.w, mmsfb->x11_win_rect.h, False);
			}
			//XFlush(mmsfb->x_display);
			XSync(mmsfb->x_display, False);
			XUnlockDisplay(mmsfb->x_display);
			mmsfb->xlock.unlock();
			this->unlock();
		}
#endif
	}

	return true;
}

bool MMSFBSurface::createCopy(MMSFBSurface **dstsurface, int w, int h,
                              bool copycontent, bool withbackbuffer, MMSFBSurfacePixelFormat pixelformat) {

    /* check if initialized */
    INITCHECK;

    if (this->is_sub_surface)
    	return false;

    *dstsurface = NULL;

    if (!w) w = config.w;
    if (!h) h = config.h;

    /* create new surface */
    if (!mmsfb->createSurface(dstsurface, w, h, (pixelformat==MMSFB_PF_NONE)?this->config.surface_buffer->pixelformat:pixelformat,
                             (withbackbuffer)?this->config.surface_buffer->backbuffer:0,this->config.surface_buffer->systemonly)) {
        if (*dstsurface)
            delete *dstsurface;
        *dstsurface = NULL;
        return false;
    }

    if (copycontent) {
        /* copy the content */
        MMSFBRectangle dstrect;
        dstrect.x = 0;
        dstrect.y = 0;
        dstrect.w = w;
        dstrect.h = h;
        (*dstsurface)->setDrawingFlags((MMSFBDrawingFlags) MMSFB_DRAW_NOFX);
        (*dstsurface)->setBlittingFlags((MMSFBBlittingFlags) MMSFB_BLIT_NOFX);
        (*dstsurface)->stretchBlit(this, NULL, &dstrect);
        if (withbackbuffer) {
            (*dstsurface)->flip();
        }
    }

    return true;
}

bool MMSFBSurface::resize(int w, int h) {

	// check old size, resize only if size changed
    int old_w, old_h;
    if (!getSize(&old_w, &old_h)) return false;
    if ((old_w == w) && (old_h == h)) return true;

    if (!this->is_sub_surface) {
        // normal surface
	    lock();

	    // create a copy
	    MMSFBSurface *dstsurface;
	    createCopy(&dstsurface, w, h, true, true);

		if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
		    D_DEBUG_AT( MMS_Surface, "resize( %dx%d -> %dx%d )\n",
		                this->config.w, this->config.h, w, h );

		    MMSFB_TRACE();

		    // move the dfb pointers
			IDirectFBSurface *s = this->llsurface;
			this->llsurface = dstsurface->llsurface;
			dstsurface->llsurface = s;

			// load the new configuration
			this->getConfiguration();
			dstsurface->getConfiguration();
#endif
		}
		else {
			// move the surface buffer data
			MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
			this->config.surface_buffer = dstsurface->config.surface_buffer;
			dstsurface->config.surface_buffer = sb;

			// load the new configuration
			this->getConfiguration();
		}

	    // free dstsurface
	    delete dstsurface;

	    unlock();
	    return true;
    }
    else  {
    	// sub surface
	    MMSFBRectangle rect = this->sub_surface_rect;
	    rect.w = w;
	    rect.h = h;
	    return setSubSurface(&rect);

    }
}



void MMSFBSurface::modulateBrightness(MMSFBColor *color, unsigned char brightness) {

    /* full brightness? */
    if (brightness == 255) return;

    /* full darkness? */
    if (brightness == 0) {
        color->r = 0;
        color->g = 0;
        color->b = 0;
        return;
    }

    /* modulate the color */
    unsigned int bn = 100000 * (255-brightness);
    if (color->r > 0) {
        unsigned int i = (10000 * 255) / (unsigned int)color->r;
        color->r = (5+((10 * (unsigned int)color->r) - (bn / i))) / 10;
    }
    if (color->g > 0) {
        unsigned int i = (10000 * 255) / (unsigned int)color->g;
        color->g = (5+((10 * (unsigned int)color->g) - (bn / i))) / 10;
    }
    if (color->b > 0) {
        unsigned int i = (10000 * 255) / (unsigned int)color->b;
        color->b = (5+((10 * (unsigned int)color->b) - (bn / i))) / 10;
    }
}

void MMSFBSurface::modulateOpacity(MMSFBColor *color, unsigned char opacity) {

    /* full opacity? */
    if (opacity == 255) return;

    /* complete transparent? */
    if (opacity == 0) {
        color->a = 0;
        return;
    }

    /* modulate the alpha value */
    unsigned int bn = 100000 * (255-opacity);
    if (color->a > 0) {
        unsigned int i = (10000 * 255) / (unsigned int)color->a;
        color->a = (5+((10 * (unsigned int)color->a) - (bn / i))) / 10;
    }
}


bool MMSFBSurface::setBlittingFlagsByBrightnessAlphaAndOpacity(
                    unsigned char brightness, unsigned char alpha, unsigned char opacity) {
    MMSFBColor color;

    /* check if initialized */
    INITCHECK;

    /* modulate the opacity into the color */
    color.a = alpha;
    modulateOpacity(&color, opacity);

    /* set color for blitting */
    setColor(brightness, brightness, brightness, color.a);

    /* set blitting flags */
    if (brightness != 255) {
        if (color.a == 255)
            setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_COLORIZE|MMSFB_BLIT_BLEND_ALPHACHANNEL));
        else
            setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_COLORIZE|MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA));
    }
    else {
        if (color.a == 255)
            setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL));
        else
            setBlittingFlags((MMSFBBlittingFlags)(MMSFB_BLIT_BLEND_ALPHACHANNEL|MMSFB_BLIT_BLEND_COLORALPHA));
    }

    return true;
}

bool MMSFBSurface::setDrawingFlagsByAlpha(unsigned char alpha) {

    // check if initialized
    INITCHECK;

    // set the drawing flags
    if (this->config.surface_buffer->premultiplied) {
    	// premultiplied surface, have to premultiply the color
	    if (alpha == 255)
	        setDrawingFlags((MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY));
	    else
	        setDrawingFlags((MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY));
    }
    else {
	    if (alpha == 255)
	        setDrawingFlags((MMSFBDrawingFlags)MMSFB_DRAW_NOFX);
	    else
	        setDrawingFlags((MMSFBDrawingFlags)MMSFB_DRAW_BLEND);
    }

    return true;
}


bool MMSFBSurface::setDrawingColorAndFlagsByBrightnessAndOpacity(
                        MMSFBColor color, unsigned char brightness, unsigned char opacity) {

    /* check if initialized */
    INITCHECK;

    /* modulate the brightness into the color */
    modulateBrightness(&color, brightness);

    /* modulate the opacity into the color */
    modulateOpacity(&color, opacity);

    /* set the color for drawing */
    setColor(color.r, color.g, color.b, color.a);

    /* set the drawing flags */
    setDrawingFlagsByAlpha(color.a);

    return true;
}


bool MMSFBSurface::setFont(MMSFBFont *font) {

    /* check if initialized */
    INITCHECK;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    DFBResult   dfbres;

	    /* set font */
		if ((dfbres=this->llsurface->SetFont(this->llsurface, (IDirectFBFont*)font->dfbfont)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFBSurface::SetFont() failed");
			return false;
		}
#endif
	}
	else {
		//TODO
	}

    /* save the font */
    this->config.font = font;

    return true;
}



bool MMSFBSurface::blit_text(string &text, int len, int x, int y) {
	MMSFBRegion clipreg;
	MMSFBSurfacePlanes dst_planes;

#ifndef USE_DFB_SUBSURFACE
	if (!this->is_sub_surface) {
#endif
		// normal surface or dfb subsurface
		if (!this->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->config.w - 1;
			clipreg.y2 = this->config.h - 1;
		}
		else
			clipreg = this->config.clip;
#ifndef USE_DFB_SUBSURFACE
	}
	else {
		// subsurface
		if (!this->root_parent->config.clipped) {
			clipreg.x1 = 0;
			clipreg.y1 = 0;
			clipreg.x2 = this->root_parent->config.w - 1;
			clipreg.y2 = this->root_parent->config.h - 1;
		}
		else
			clipreg = this->root_parent->config.clip;
	}
#endif

	// calculate the color
	MMSFBColor color = this->config.color;
	if (this->config.drawingflags & (MMSFBDrawingFlags)MMSFB_DRAW_SRC_PREMULTIPLY) {
		// pre-multiplication needed
		if (color.a != 0xff) {
			color.r = ((color.a+1) * color.r) >> 8;
			color.g = ((color.a+1) * color.g) >> 8;
			color.b = ((color.a+1) * color.b) >> 8;
		}
	}

	// checking pixelformats...
	switch (this->config.surface_buffer->pixelformat) {
	case MMSFB_PF_ARGB:
		// destination is ARGB
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			||(this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_drawstring_blend_argb(
						&dst_planes, this->config.font, clipreg,
						text, len, x, y, color);
				extendedUnlock(NULL, this);
				return true;
			}
			return false;
		}
		else
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			||(this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_drawstring_blend_coloralpha_argb(
						&dst_planes, this->config.font, clipreg,
						text, len, x, y, color);
				extendedUnlock(NULL, this);
				return true;
			}
			return false;
		}
		break;

	case MMSFB_PF_ARGB4444:
		// destination is ARGB4444
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			||(this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_drawstring_blend_argb4444(
						&dst_planes, this->config.font, clipreg,
						text, len, x, y, color);
				extendedUnlock(NULL, this);
				return true;
			}
			return false;
		}
		break;

	case MMSFB_PF_RGB16:
		// destination is RGB16
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX))
			||(this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_NOFX|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_drawstring_blend_rgb16(
						&dst_planes, this->config.font, clipreg,
						text, len, x, y, color);
				extendedUnlock(NULL, this);
				return true;
			}
			return false;
		}
		else
		if   ((this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND))
			||(this->config.drawingflags == (MMSFBDrawingFlags)(MMSFB_DRAW_BLEND|MMSFB_DRAW_SRC_PREMULTIPLY))) {
			if (extendedLock(NULL, NULL, this, &dst_planes)) {
				mmsfb_drawstring_blend_coloralpha_rgb16(
						&dst_planes, this->config.font, clipreg,
						text, len, x, y, color);
				extendedUnlock(NULL, this);
				return true;
			}
			return false;
		}
		break;

	default:
		// does not match
		break;
	}

	return printMissingCombination("blit_text()", NULL, NULL, MMSFB_PF_NONE, 0, 0);
}


bool MMSFBSurface::drawString(string text, int len, int x, int y) {

    // check if initialized
    INITCHECK;

    if (!this->config.font)
    	return false;

	// get the length of the string
	if (len < 0) len = text.size();
	if (!len) return true;

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
	    D_DEBUG_AT( MMS_Surface, "drawString( '%s', %d, %d,%d ) <- %dx%d\n",
	                text.c_str(), len, x, y, this->config.w, this->config.h );
	    MMSFB_TRACE();

	    // draw a string
	    DFBResult dfbres;
		if (!this->is_sub_surface) {
			if ((dfbres=this->llsurface->DrawString(this->llsurface, text.c_str(), len, x, y, DSTF_TOPLEFT)) != DFB_OK) {
				MMSFB_SetError(dfbres, "IDirectFBSurface::DrawString() failed");

				return false;
			}
		}
		else {
#ifndef USE_DFB_SUBSURFACE
			CLIPSUBSURFACE

			x+=this->sub_surface_xoff;
			y+=this->sub_surface_yoff;

			SETSUBSURFACE_DRAWINGFLAGS;
#endif

			this->llsurface->DrawString(this->llsurface, text.c_str(), len, x, y, DSTF_TOPLEFT);

#ifndef USE_DFB_SUBSURFACE
			RESETSUBSURFACE_DRAWINGFLAGS;

			UNCLIPSUBSURFACE
#endif
		}
#endif
	}
	else {
		// draw a string
		if (!this->is_sub_surface) {
			blit_text(text, len, x, y);
		}
		else {
			CLIPSUBSURFACE

			x+=this->sub_surface_xoff;
			y+=this->sub_surface_yoff;

			blit_text(text, len, x, y);

			UNCLIPSUBSURFACE
		}
	}

    return true;
}

void MMSFBSurface::lock(MMSFBLockFlags flags, MMSFBSurfacePlanes *planes, bool pthread_lock) {
	if (!pthread_lock) {
		// no pthread lock needed
		if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
			if (flags && planes) {
				// get the access to the surface buffer
				memset(planes, 0, sizeof(MMSFBSurfacePlanes));
				if (flags == MMSFB_LOCK_READ) {
					if (this->llsurface->Lock(this->llsurface, DSLF_READ, &planes->ptr, &planes->pitch) != DFB_OK) {
						planes->ptr = NULL;
						planes->pitch = 0;
					}
				}
				else
				if (flags == MMSFB_LOCK_WRITE) {
					if (this->llsurface->Lock(this->llsurface, DSLF_WRITE, &planes->ptr, &planes->pitch) != DFB_OK) {
						planes->ptr = NULL;
						planes->pitch = 0;
					}
				}
			}
#endif
		}
		else {
			if (flags && planes) {
				// get the access to the surface buffer
				memset(planes, 0, sizeof(MMSFBSurfacePlanes));
				MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
				if (flags == MMSFB_LOCK_READ) {
					*planes = sb->buffers[sb->currbuffer_read];
				}
				else
				if (flags == MMSFB_LOCK_WRITE) {
					*planes = sb->buffers[sb->currbuffer_write];
				}
			}
		}
		return;
	}

	// which surface is to lock?
	MMSFBSurface *tolock = this;
	if (this->root_parent)
		tolock = this->root_parent;
	else
	if (this->parent)
		tolock = this->parent;

    if (tolock->Lock.trylock() == 0) {
        // I have got the lock the first time
    	tolock->TID = pthread_self();
    	tolock->Lock_cnt = 1;
    }
    else {
        if ((tolock->TID == pthread_self())&&(tolock->Lock_cnt > 0)) {
            // I am the thread which has already locked this surface
        	tolock->Lock_cnt++;
        }
        else {
            // another thread has already locked this surface, waiting for...
        	tolock->Lock.lock();
        	tolock->TID = pthread_self();
        	tolock->Lock_cnt = 1;
        }
    }

	if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
		if (flags && planes) {
			// get the access to the surface buffer
			memset(planes, 0, sizeof(MMSFBSurfacePlanes));
			if (flags == MMSFB_LOCK_READ) {
				if (!tolock->surface_read_locked) {
					if (this->llsurface->Lock(this->llsurface, DSLF_READ, &planes->ptr, &planes->pitch) != DFB_OK) {
						planes->ptr = NULL;
						planes->pitch = 0;
					}
					else {
						tolock->surface_read_locked = true;
						tolock->surface_read_lock_cnt = tolock->Lock_cnt;
					}
				}
			}
			else
			if (flags == MMSFB_LOCK_WRITE) {
				if (!tolock->surface_write_locked) {
					if (this->llsurface->Lock(this->llsurface, DSLF_WRITE, &planes->ptr, &planes->pitch) != DFB_OK) {
						planes->ptr = NULL;
						planes->pitch = 0;
					}
					else {
						tolock->surface_write_locked = true;
						tolock->surface_write_lock_cnt = tolock->Lock_cnt;
					}
				}
			}
		}
#endif
	}
	else {
		if (flags && planes) {
			// get the access to the surface buffer
			memset(planes, 0, sizeof(MMSFBSurfacePlanes));
			MMSFBSurfaceBuffer *sb = this->config.surface_buffer;
			if (flags == MMSFB_LOCK_READ) {
				if (!tolock->surface_read_locked) {
					*planes = sb->buffers[sb->currbuffer_read];
					tolock->surface_read_locked = true;
					tolock->surface_read_lock_cnt = tolock->Lock_cnt;
				}
			}
			else
			if (flags == MMSFB_LOCK_WRITE) {
				if (!tolock->surface_write_locked) {
					*planes = sb->buffers[sb->currbuffer_write];
					tolock->surface_write_locked = true;
					tolock->surface_write_lock_cnt = tolock->Lock_cnt;
				}
			}
		}
	}
}

void MMSFBSurface::lock(MMSFBLockFlags flags, void **ptr, int *pitch) {
	if (!ptr || !pitch) {
		// nothing to return
		lock(flags, NULL, true);
	}
	else {
		// get the planes an return the first one
		MMSFBSurfacePlanes planes;
		lock(flags, &planes, true);
		*ptr = planes.ptr;
		*pitch = planes.pitch;
	}
}

void MMSFBSurface::lock(MMSFBLockFlags flags, MMSFBSurfacePlanes *planes) {
	lock(flags, planes, true);
}

void MMSFBSurface::unlock(bool pthread_unlock) {
	if (!pthread_unlock) {
		// no pthread unlock needed
		if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
			this->llsurface->Unlock(this->llsurface);
#endif
		}
		return;
	}

	// which surface is to lock?
	MMSFBSurface *tolock = this;
	if (this->root_parent)
		tolock = this->root_parent;
	else
	if (this->parent)
		tolock = this->parent;

	if (tolock->TID != pthread_self())
        return;

    if (tolock->Lock_cnt==0)
    	return;

	// unlock dfb surface?
	if ((tolock->surface_read_locked)&&(tolock->surface_read_lock_cnt == tolock->Lock_cnt)) {
		if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
			this->llsurface->Unlock(this->llsurface);
#endif
		}
		tolock->surface_read_locked = false;
		tolock->surface_read_lock_cnt = 0;
	}
	else
	if ((tolock->surface_write_locked)&&(tolock->surface_write_lock_cnt == tolock->Lock_cnt)) {
		if (!this->use_own_alloc) {
#ifdef  __HAVE_DIRECTFB__
			this->llsurface->Unlock(this->llsurface);
#endif
		}
		tolock->surface_write_locked = false;
		tolock->surface_write_lock_cnt = 0;
	}

    tolock->Lock_cnt--;

    if (tolock->Lock_cnt == 0)
    	tolock->Lock.unlock();
}

void MMSFBSurface::unlock() {
	unlock(true);
}

MMSFBSurface *MMSFBSurface::getSubSurface(MMSFBRectangle *rect) {
    void   			*subsuf = NULL;
    MMSFBSurface 	*surface;

    /* check if initialized */
    INITCHECK;

#ifdef USE_DFB_SUBSURFACE
    /* get a sub surface */
    DFBResult dfbres;
    if ((dfbres=this->llsurface->GetSubSurface(this->llsurface, rect, (IDirectFBSurface*)&subsuf)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBSurface::GetSubSurface() failed");
        return false;
    }
#endif

    /* create a new surface instance */
    surface = new MMSFBSurface(subsuf, this, rect);
    if (!surface) {
#ifdef USE_DFB_SUBSURFACE
    	if (subsuf)
    		((IDirectFBSurface*)subsuf)->Release((IDirectFBSurface*)subsuf);
#endif
        MMSFB_SetError(0, "cannot create new instance of MMSFBSurface");
        return NULL;
    }

    /* add to my list */
    this->children.push_back(surface);

    return surface;
}

bool MMSFBSurface::setSubSurface(MMSFBRectangle *rect) {

	/* check if initialized */
    INITCHECK;

    /* only sub surfaces can be moved */
    if (!this->is_sub_surface)
		return false;

    lock();

    if (memcmp(rect, &(this->sub_surface_rect), sizeof(this->sub_surface_rect)) == 0) {
    	/* nothing changed */
    	unlock();
    	return false;
    }

#ifdef USE_DFB_SUBSURFACE
    /* because dfb has no IDirectFBSurface::setSubSurface(), allocate a new and release the old one */
    DFBResult dfbres;
    IDirectFBSurface *subsuf = NULL;
    if ((dfbres=this->parent->llsurface->GetSubSurface(this->parent->llsurface, rect, &subsuf)) != DFB_OK) {
        MMSFB_SetError(dfbres, "IDirectFBSurface::GetSubSurface() failed");
        unlock();
        return false;
    }

    if (this->llsurface)
    	this->llsurface->Release(this->llsurface);

    this->llsurface = subsuf;

#endif

    this->sub_surface_rect = *rect;

#ifndef USE_DFB_SUBSURFACE

    getRealSubSurfacePos(NULL, true);

#endif

    unlock();

    return true;
}

bool MMSFBSurface::setSubSurface(MMSFBRegion *region) {
	MMSFBRectangle rect;

	if (!region)
		return false;

	rect.x = region->x1;
	rect.y = region->y1;
	rect.w = region->x2 - region->x1 + 1;
	rect.h = region->y2 - region->y1 + 1;

	return setSubSurface(&rect);
}

bool MMSFBSurface::moveTo(int x, int y) {
	MMSFBRectangle rect;

	rect = this->sub_surface_rect;
	rect.x = x;
	rect.y = y;

	return setSubSurface(&rect);
}

bool MMSFBSurface::move(int x, int y) {
	MMSFBRectangle rect;

	rect = this->sub_surface_rect;
	rect.x += x;
	rect.y += y;

	return setSubSurface(&rect);
}


bool MMSFBSurface::dump2fcb(bool (*fcb)(char *, int, void *, int *), void *argp, int *argi,
						   int x, int y, int w, int h, MMSFBSurfaceDumpMode dumpmode) {
#define D2FCB_ADDSTR1(f) {int l=sprintf(ob,f);if(!fcb(ob,l,argp,argi)){this->unlock();return false;}}
#define D2FCB_ADDSTR2(f,v) {int l=sprintf(ob,f,v);if(!fcb(ob,l,argp,argi)){this->unlock();return false;}}
	// check inputs
	if (!fcb)
		return false;
	if ((x < 0)||(y < 0)||(w < 0)||(h < 0))
		return false;
	if (w == 0)
		w = this->config.w - x;
	if (h == 0)
		h = this->config.h - y;
	if ((x + w > this->config.w)||(y + h > this->config.h))
		return false;

	// set buffer
	char ob[65536];

	// get access to the surface memory
	unsigned char 	*sbuf;
	int				pitch;
	this->lock(MMSFB_LOCK_READ, (void**)&sbuf, &pitch);
	if (!sbuf)
		return false;

	// print format
	sprintf(ob, "* %s: x=%d, y=%d, w=%d, h=%d",
				getMMSFBPixelFormatString(this->config.surface_buffer->pixelformat).c_str(),
				x, y, w, h);
	fcb(ob, strlen(ob), argp, argi);

	bool dumpok = false;
	if (dumpmode == MMSFBSURFACE_DUMPMODE_BYTE) {
		// dump byte-by-byte
		switch (this->config.surface_buffer->pixelformat) {
		case MMSFB_PF_I420:
		case MMSFB_PF_YV12:
		case MMSFB_PF_NV12:
		case MMSFB_PF_NV16:
		case MMSFB_PF_NV21:
		case MMSFB_PF_ARGB3565:
			// do not dump plane formats here
			break;
		default: {
				// all other formats
				int bits_pp = getBitsPerPixel(this->config.surface_buffer->pixelformat);
				int bytes_pp = bits_pp / 8;
				unsigned char *buf = sbuf + x * bytes_pp + y * pitch;
				D2FCB_ADDSTR1("\n* byte-by-byte ****************************************************************");
				for (int j = 0; j < h-y; j++) {
					int i = j * pitch;
					D2FCB_ADDSTR2("\n%02x", buf[i++]);
					while (i < (w-x) * bytes_pp + j * pitch) {
						D2FCB_ADDSTR2(",%02x", buf[i]);
						i++;
					}
				}
				dumpok = true;
			}
			break;
		}
	}

	if (!dumpok) {
		// dump pixels
		switch (this->config.surface_buffer->pixelformat) {
		case MMSFB_PF_ARGB:
		case MMSFB_PF_RGB32: {
				int pitch_pix = pitch >> 2;
				unsigned int *buf = (unsigned int*)sbuf + x + y * pitch_pix;
				switch (this->config.surface_buffer->pixelformat) {
				case MMSFB_PF_ARGB:
					D2FCB_ADDSTR1("\n* aarrggbb hex (4-byte integer) ***********************************************");
					break;
				case MMSFB_PF_RGB32:
					D2FCB_ADDSTR1("\n* --rrggbb hex (4-byte integer) ***********************************************");
					break;
				default:
					break;
				}
				for (int j = 0; j < h-y; j++) {
					int i = j * pitch_pix;
					D2FCB_ADDSTR2("\n%08x", (int)buf[i++]);
					while (i < (w-x) + j * pitch_pix) {
						D2FCB_ADDSTR2(",%08x", (int)buf[i]);
						i++;
					}
				}
				D2FCB_ADDSTR1("\n*******************************************************************************");
			}
			break;
		case MMSFB_PF_BGR24: {
				D2FCB_ADDSTR1("\n* bbggrr hex (3-byte) *********************************************************");
				D2FCB_ADDSTR1("\nn/a");
				D2FCB_ADDSTR1("\n*******************************************************************************");
			}
			break;
		case MMSFB_PF_RGB16:
		case MMSFB_PF_BGR555: {
				int pitch_pix = pitch >> 1;
				unsigned short int *buf = (unsigned short int*)sbuf + x + y * pitch_pix;
				switch (this->config.surface_buffer->pixelformat) {
				case MMSFB_PF_RGB16:
					D2FCB_ADDSTR1("\n* rrrrrggggggbbbbb bin (2-byte integer) ***************************************");
					break;
				case MMSFB_PF_BGR555:
					D2FCB_ADDSTR1("\n* 0bbbbbgggggrrrrr bin (2-byte integer) ***************************************");
					break;
				default:
					break;
				}
				for (int j = 0; j < h-y; j++) {
					int i = j * pitch_pix;
					D2FCB_ADDSTR2("\n%04x", buf[i++]);
					while (i < (w-x) + j * pitch_pix) {
						D2FCB_ADDSTR2(",%04x", buf[i]);
						i++;
					}
				}
				D2FCB_ADDSTR1("\n*******************************************************************************");
			}
			break;
		case MMSFB_PF_I420:
		case MMSFB_PF_YV12: {
				int pitch_pix = pitch;
				unsigned char *buf_y;
				unsigned char *buf_u = sbuf + pitch_pix * this->config.h + (x >> 1) + (y >> 1) * (pitch_pix >> 1);
				unsigned char *buf_v = sbuf + pitch_pix * (this->config.h + (this->config.h >> 2)) + (x >> 1) + (y >> 1) * (pitch_pix >> 1);
				if (this->config.surface_buffer->pixelformat == MMSFB_PF_YV12) {
					buf_y = buf_u;
					buf_u = buf_v;
					buf_v = buf_y;
				}
				buf_y = sbuf + x + y * pitch_pix;
				D2FCB_ADDSTR1("\n* Y plane *********************************************************************");
				for (int j = 0; j < h-y; j++) {
					int i = j * pitch_pix;
					D2FCB_ADDSTR2("\n%02x", buf_y[i++]);
					while (i < (w-x) + j * pitch_pix) {
						D2FCB_ADDSTR2(",%02x", buf_y[i]);
						i++;
					}
				}
				D2FCB_ADDSTR1("\n* U plane *********************************************************************");
				x = x >> 1;
				y = y >> 1;
				w = w >> 1;
				h = h >> 1;
				for (int j = 0; j < h-y; j++) {
					int i = j * (pitch_pix >> 1);
					D2FCB_ADDSTR2("\n%02x", buf_u[i++]);
					while (i < (w-x) + j * (pitch_pix >> 1)) {
						D2FCB_ADDSTR2(",%02x", buf_u[i]);
						i++;
					}
				}
				D2FCB_ADDSTR1("\n* V plane *********************************************************************");
				for (int j = 0; j < h-y; j++) {
					int i = j * (pitch_pix >> 1);
					D2FCB_ADDSTR2("\n%02x", buf_v[i++]);
					while (i < (w-x) + j * (pitch_pix >> 1)) {
						D2FCB_ADDSTR2(",%02x", buf_v[i]);
						i++;
					}
				}
				D2FCB_ADDSTR1("\n*******************************************************************************");
			}
			break;
		default:
			// no dump routine for this pixelformat
			this->unlock();
			return false;
		}
	}

	// finalize
	this->unlock();
	D2FCB_ADDSTR1("\n");
	return true;
}

bool dump2buffer_fcb(char *buf, int len, void *argp, int *argi) {
	if (len >= *argi) return false;
	char *ap = *((char**)argp);
	memcpy(ap, buf, len);
	ap+= len;
	*((void**)argp) = ap;
	*argi = *argi - len;
	return true;
}

int MMSFBSurface::dump2buffer(char *out_buffer, int out_buffer_len, int x, int y, int w, int h,
							  MMSFBSurfaceDumpMode dumpmode) {
	int obl = out_buffer_len;
	if (dump2fcb(dump2buffer_fcb, (void*)(&out_buffer), &obl, x, y, w, h, dumpmode)) {
		out_buffer[out_buffer_len - obl] = 0;
		return out_buffer_len - obl;
	}
	return 0;
}

bool dump2file_fcb(char *buf, int len, void *argp, int *argi) {
	size_t ritems;
	((MMSFile *)argp)->writeBuffer(buf, &ritems, 1, len);
	return true;
}

bool MMSFBSurface::dump2file(string filename, int x, int y, int w, int h,
							 MMSFBSurfaceDumpMode dumpmode) {
	MMSFile *mmsfile = new MMSFile(filename, MMSFM_WRITE);
	if (mmsfile) {
		if (dump2fcb(dump2file_fcb, mmsfile, NULL, x, y, w, h, dumpmode)) {
			delete mmsfile;
			return true;
		}
		delete mmsfile;
	}
	return false;
}

bool MMSFBSurface::dump2file(string filename, MMSFBSurfaceDumpMode dumpmode) {
	return dump2file(filename, 0, 0, 0, 0, dumpmode);
}

bool dump_fcb(char *buf, int len, void *argp, int *argi) {
	buf[len] = 0;
	printf(buf);
	return true;
}

bool MMSFBSurface::dump(int x, int y, int w, int h,
						MMSFBSurfaceDumpMode dumpmode) {
	if (dump2fcb(dump_fcb, NULL, NULL, x, y, w, h, dumpmode)) {
		printf("\n");
		return true;
	}
	return false;
}

bool MMSFBSurface::dump(MMSFBSurfaceDumpMode dumpmode) {
	return dump(0, 0, 0, 0, dumpmode);
}

bool mmsfb_create_cached_surface(MMSFBSurface **cs, int width, int height,
								 MMSFBSurfacePixelFormat pixelformat) {
	if (!cs) return false;

	// check the properties of the existing surface
	if (*cs) {
		// check if old surface has the same dimension
		int w, h;
        (*cs)->getSize(&w, &h);
        if ((w != width) || (h != height)) {
        	delete *cs;
        	*cs = NULL;
        }
	}

	if (*cs) {
		// check if old surface has the same pixelformat
		MMSFBSurfacePixelFormat pf;
        (*cs)->getPixelFormat(&pf);
        if (pf != pixelformat) {
        	delete *cs;
        	*cs = NULL;
        }
	}

	if (!*cs) {
		// create new surface
		*cs = new MMSFBSurface(width, height, pixelformat);
	}

	return (*cs);
}

