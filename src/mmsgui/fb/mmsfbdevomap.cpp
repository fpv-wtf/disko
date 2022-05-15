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

#ifdef __HAVE_FBDEV__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include "mmsgui/fb/mmsfbdevomap.h"
#include <sys/ioctl.h>
#include <cstring>
#include "mmsgui/fb/omapfb.h"


#define INITCHECK  if(!this->isinitialized){MMSFB_SetError(0,"MMSFBDevOmap is not initialized");return false;}

MMSFBDevOmap::MMSFBDevOmap() {
	this->osd0.fbdev	= NULL;
	this->vid.fbdev		= NULL;
	this->osd1.fbdev	= NULL;
	this->primary		= NULL;
}

MMSFBDevOmap::~MMSFBDevOmap() {
	closeDevice();
}

bool MMSFBDevOmap::openDevice(char *device_file, int console) {
	// close the device if opened
	closeDevice();

	// open omap frame buffers
	for (int i = 0; i < 3; i++) {
		MMSFBDevOmap *fbdev;
		char      dev[100];
		sprintf(dev, "/dev/fb%d", i);
		fbdev = new MMSFBDevOmap();
//		if (!fbdev->openDevice(dev, (i==1)?-1:-2)) {
		if (!fbdev->MMSFBDev::openDevice(dev, -2)) {
			delete fbdev;
			closeDevice();
			return false;
		}

		if (memcmp(fbdev->fix_screeninfo.id, "omapfb", 6) == 0) {
			switch (i) {
			case 0:
				this->osd0.fbdev = fbdev;
				strcpy(this->osd0.device, dev);
				this->osd0.width = 0;
				this->primary = &this->osd0;
				// disable device
				this->osd0.fbdev->setMode(-1, -1, MMSFB_PF_NONE, false);
				break;
			case 1:
				this->vid.fbdev = fbdev;
				strcpy(this->vid.device, dev);
				this->vid.width = 0;
				if (!this->primary)
					this->primary = &this->vid;
				// disable device
	//			this->vid.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, false);
				break;
			case 2:
				this->osd1.fbdev = fbdev;
				strcpy(this->osd1.device, dev);
				//this->osd1.width = -1;
				this->primary = &this->osd1;
				break;
			}
		}
		else {
			// not supported
			printf("MMSFBDevOmap: unsupported accelerator %d (%.16s)\n", fbdev->fix_screeninfo.accel, fbdev->fix_screeninfo.id);
			delete fbdev;
			closeDevice();
			return false;
		}
	}

    // all initialized :)
	this->isinitialized = true;

	return true;
}

void MMSFBDevOmap::closeDevice() {
	// close frame buffers
	if (this->osd1.fbdev) {
		delete this->osd1.fbdev;
		this->osd1.fbdev = NULL;
	}
	if (this->vid.fbdev) {
		delete this->vid.fbdev;
		this->vid.fbdev = NULL;
	}
	if (this->osd0.fbdev) {
		delete this->osd0.fbdev;
		this->osd0.fbdev = NULL;
	}
	this->primary = NULL;

	// reset all other
	this->isinitialized = false;
}

bool MMSFBDevOmap::waitForVSync() {
	// is initialized?
	INITCHECK;

	if (!this->primary->fbdev)
		return false;

	static const int s = 0;
	if (ioctl(this->primary->fbdev->fd, FBIO_WAITFORVSYNC, &s)) {
		// failed, well then???
	}

	return true;
}

bool MMSFBDevOmap::panDisplay(int buffer_id, void *framebuffer_base) {
	// is initialized?
	INITCHECK;

	if (framebuffer_base == this->osd0.fbdev->framebuffer_base) {
		// Graphic layer (OSD0)
		if (this->osd0.fbdev)
			return this->osd0.fbdev->MMSFBDev::panDisplay(buffer_id);
		return false;
	}
	else
	if (framebuffer_base == this->vid.fbdev->framebuffer_base) {
		// Video layer (VID)
		if (this->vid.fbdev)
			return this->vid.fbdev->MMSFBDev::panDisplay(buffer_id);
		return false;
	}
	else
	if (framebuffer_base == this->osd1.fbdev->framebuffer_base) {
		// Graphic layer (OSD1)
		if (this->osd1.fbdev)
			return this->osd1.fbdev->MMSFBDev::panDisplay(buffer_id);
		return false;
	}

	// check framebuffer_base pointer
	printf("MMSFBDevOmap: framebuffer base pointer not correct\n");
	return false;
}

bool MMSFBDevOmap::testLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
	    return true;
	case 1:
		// Video layer
		return true;
	case 2:
		// OSD layer
		return true;
	default:
    	printf("MMSFBDevOmap: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}


bool MMSFBDevOmap::initLayer(int layer_id, int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		break;
		// default fbdev primary layer 0 on primary screen 0
		if (!this->osd0.fbdev) {
			printf("MMSFBDevOmap: OSD Layer %d not initialized\n", layer_id);
			return false;
		}

		if   ((pixelformat != MMSFB_PF_ARGB)
			&&(pixelformat != MMSFB_PF_RGB32)
			&&(pixelformat != MMSFB_PF_RGB16)) {
			printf("MMSFBDevOmap: OSD Layer %d needs pixelformat ARGB, RGB32 or RGB16, but %s given\n",
						layer_id, getMMSFBPixelFormatString(pixelformat).c_str());
			return false;
		}

		if (backbuffer) {
			printf("MMSFBDevOmap: OSD Layer %d does not support backbuffer handling\n", layer_id);
			return false;
		}

		// enable OSD0
		if (this->osd0.fbdev->setMode(width, height, pixelformat, backbuffer)) {
			// set values
			this->layers[layer_id].width = width;
			this->layers[layer_id].height = height;
			this->layers[layer_id].pixelformat = pixelformat;


			// save the buffers
			memcpy(this->layers[layer_id].buffers, this->osd0.fbdev->layers[0].buffers, sizeof(this->osd0.fbdev->layers[0].buffers));

			// clear layer
			MMSFBColor color(0x00, 0x00, 0x00, 0xff);
			switch (pixelformat) {
			case MMSFB_PF_ARGB:
				mmsfb_fillrectangle_argb(&(this->layers[layer_id].buffers[0]), this->layers[layer_id].height,
										 0, 0, this->layers[layer_id].width, this->layers[layer_id].height, color);
				break;
			case MMSFB_PF_RGB32:
				mmsfb_fillrectangle_rgb32(&(this->layers[layer_id].buffers[0]), this->layers[layer_id].height,
										  0, 0, this->layers[layer_id].width, this->layers[layer_id].height, color);
				break;
			case MMSFB_PF_RGB16:
				mmsfb_fillrectangle_rgb16(&(this->layers[layer_id].buffers[0]), this->layers[layer_id].height,
										  0, 0, this->layers[layer_id].width, this->layers[layer_id].height, color);
				break;
			default:
				break;
			}

			// layer is initialized
			this->layers[layer_id].isinitialized = true;

			printf("MMSFBDevOmap: OSD Layer %d initialized with %dx%dx%d, pixelformat %s\n",
						layer_id, width, height, backbuffer+1, getMMSFBPixelFormatString(pixelformat).c_str());

			return true;
		}
		return false;

	case 1:
		// Video layer (VID)
		if (!this->vid.fbdev) {
			printf("MMSFBDevOmap: Video Layer %d not initialized\n", layer_id);
			return false;
		}

		if   (pixelformat != MMSFB_PF_I420) {
			printf("MMSFBDevOmap: Video Layer %d needs pixelformat I420 (==YUV420) but %s given\n",
						layer_id, getMMSFBPixelFormatString(pixelformat).c_str());
			return false;
		}

		// enable VID
		if (this->vid.fbdev->setMode(width, height, pixelformat, backbuffer)) {
			// set values
			this->layers[layer_id].width = width;
			this->layers[layer_id].height = height;
			this->layers[layer_id].pixelformat = pixelformat;

			// save the buffers
			memcpy(this->layers[layer_id].buffers, this->vid.fbdev->layers[0].buffers, sizeof(this->vid.fbdev->layers[0].buffers));
			this->layers[layer_id].buffers[0].ptr  = this->vid.fbdev->framebuffer_base;
			this->layers[layer_id].buffers[0].pitch= this->vid.fbdev->fix_screeninfo.line_length;
			this->layers[layer_id].buffers[0].hwbuffer = true;

			// clear layer
			MMSFBColor color(0x00, 0x00, 0x00, 0xff);
			switch (pixelformat) {
				case MMSFB_PF_I420:
					mmsfb_fillrectangle_i420(&(this->layers[layer_id].buffers[0]), this->layers[layer_id].height,
										 0, 0, this->layers[layer_id].width, this->layers[layer_id].height, color);
					break;
				default:
					break;		
			}

			// layer is initialized
			this->layers[layer_id].isinitialized = true;

			printf("MMSFBDevOmap: Video Layer %d initialized with %dx%dx%d, pixelformat %s\n",
						layer_id, width, height, backbuffer+1, getMMSFBPixelFormatString(pixelformat).c_str());

			return true;
		}
		return false;

	case 2:
		// OSD layer (OSD1)
		if (!this->osd1.fbdev) {
			printf("MMSFBDevOmap: OSD Layer %d not initialized\n", layer_id);
			return false;
		}

		if   ((pixelformat != MMSFB_PF_ARGB)&&(pixelformat != MMSFB_PF_RGB32)) {
			printf("MMSFBDevOmap: OSD Layer %d needs pixelformat ARGB but %s given\n",
						layer_id, getMMSFBPixelFormatString(pixelformat).c_str());
			return false;
		}

		if (backbuffer) {
			printf("MMSFBDevOmap: OSD Layer %d does not support backbuffer handling\n", layer_id);
			return false;
		}

		// enable OSD1
		if (this->osd1.fbdev->setMode(width, height, pixelformat, backbuffer)) {
			// set values
			this->layers[layer_id].width = width;
			this->layers[layer_id].height = height;
			this->layers[layer_id].pixelformat = pixelformat;

			// save the buffers
			memcpy(this->layers[layer_id].buffers, this->osd1.fbdev->layers[0].buffers, sizeof(this->osd1.fbdev->layers[0].buffers));
			this->layers[layer_id].buffers[0].ptr  = this->osd1.fbdev->framebuffer_base;
			this->layers[layer_id].buffers[0].pitch= this->osd1.fbdev->fix_screeninfo.line_length;
			this->layers[layer_id].buffers[0].hwbuffer = true;

			// clear layer
			MMSFBColor color(0x00, 0x00, 0x00, 0xff);
			switch (pixelformat) {
			case MMSFB_PF_ARGB:
				mmsfb_fillrectangle_argb(&(this->layers[layer_id].buffers[0]), this->layers[layer_id].height,
										 0, 0, this->layers[layer_id].width, this->layers[layer_id].height, color);
				break;
			default:
				break;
			}

			// layer is initialized
			this->layers[layer_id].isinitialized = true;

			printf("MMSFBDevOmap: OSD Layer %d initialized with %dx%dx%d, pixelformat %s\n",
						layer_id, width, height, backbuffer+1, getMMSFBPixelFormatString(pixelformat).c_str());


			// add alpha channel if alpha channel pf
			if(isAlphaPixelFormat(pixelformat)) {
				printf("MMSFBDevOmap: set alpha blending!\n");

				int sysfd;
				sysfd = open("/sys/devices/platform/omapdss/manager0/alpha_blending_enabled",O_WRONLY);
				if(sysfd == -1) {
					printf("MMSFBDevOmap: could not access display manager!\n");
				}
				write(sysfd,"1\n",2);
				close(sysfd);
			}

			return true;
		}
		return false;

	default:
		printf("MMSFBDevOmap: layer %d is not supported\n", layer_id);
		break;
	}

	return false;
}

bool MMSFBDevOmap::releaseLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
		printf("MMSFBDevOmap: layer %d cannot be released\n", layer_id);
		return false;
	case 1:
		// Video layer (VID)
		if (this->vid.fbdev) {
			// disable
			this->vid.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
			// close
			this->vid.fbdev->closeDevice();
			return true;
		}
		printf("MMSFBDevOmap: Video Layer %d not initialized\n", layer_id);
		return false;
	case 2:
		// OSD layer
		printf("MMSFBDevOmap: layer %d cannot be released\n", layer_id);
		return false;
	default:
    	printf("MMSFBDevOmap: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}

bool MMSFBDevOmap::restoreLayer(int layer_id) {
	// is initialized?
	INITCHECK;

	switch (layer_id) {
	case 0:
		// default fbdev primary layer 0 on primary screen 0
		printf("MMSFBDevOmap: layer %d cannot be restored\n", layer_id);
	    return false;
	case 1:
		// Video layer (VID)
		if (this->vid.fbdev) {
			if (this->vid.fbdev->openDevice(this->vid.device, -2)) {
				if (!this->vid.width)
					return this->vid.fbdev->initLayer(0, 0, 0, MMSFB_PF_NONE, 0);
				else
				if (this->vid.width > 0)
					return this->vid.fbdev->initLayer(0, this->vid.width, this->vid.height,
													  this->vid.pixelformat, this->vid.backbuffer);
				return true;
			}
			return false;
		}
		printf("MMSFBDevOmap: Video Layer %d not initialized\n", layer_id);
		return false;
	case 2:
		// OSD layer (OSD1)
		printf("MMSFBDevOmap: layer %d cannot be restored\n", layer_id);
	    return false;
	default:
    	printf("MMSFBDevOmap: layer %d is not supported\n", layer_id);
    	break;
	}

	return false;
}

bool MMSFBDevOmap::vtGetFd(int *fd) {
	if ((this->primary)&&(this->primary->fbdev)) {
		if (this->primary->fbdev->vt.fd != -1) {
			*fd = this->primary->fbdev->vt.fd;
			return true;
		}
	}
	return false;
}

bool MMSFBDevOmap::setMode(int width, int height, MMSFBSurfacePixelFormat pixelformat, int backbuffer) {
	bool do_switch = false;

	// is initialized?
	INITCHECK;

	if (width <= 0 || height <= 0) {
		struct omapfb_plane_info plane_info;
		ioctl(this->fd, OMAPFB_QUERY_PLANE, &plane_info);
		plane_info.enabled = 0;
		ioctl(this->fd, OMAPFB_SETUP_PLANE, &plane_info);
		close(this->fd);
		delete this->osd1.fbdev;
		return true;
		// have to disable the framebuffer
		// use FBIOPUT_VSCREENINFO and FBIOBLANK together
		struct fb_var_screeninfo var_screeninfo;
	    ioctl(this->fd, FBIOGET_VSCREENINFO, &var_screeninfo);
		var_screeninfo.activate = FB_ACTIVATE_NOW;
		var_screeninfo.accel_flags = 0;
		var_screeninfo.xres = 0;
		var_screeninfo.yres = 0;
		var_screeninfo.xres_virtual = 0;
		var_screeninfo.yres_virtual = 0;
		var_screeninfo.xoffset = 0;
		var_screeninfo.yoffset = 0;
		var_screeninfo.grayscale = 0;
	    ioctl(this->fd, FBIOPUT_VSCREENINFO, &var_screeninfo);
		//ioctl(this->fd, FBIOBLANK, 1);
	
		
	}

    // reload fix screen infos before changing it
    if (ioctl(this->fd, FBIOGET_FSCREENINFO, &this->fix_screeninfo) < 0) {
    	printf("MMSFBDev: could not get fix screen infos\n");
        return false;
    }
    printFixScreenInfo();

    // reload variable screen infos before changing it
    if (ioctl(this->fd, FBIOGET_VSCREENINFO, &this->var_screeninfo) < 0) {
    	printf("MMSFBDev: could not get var screen infos\n");
        return false;
    }
    printVarScreenInfo();

    if (backbuffer) {
    	if ((!this->fix_screeninfo.ypanstep)&&(!this->fix_screeninfo.ywrapstep)) {
        	printf("MMSFBDev: backbuffer requested, but hardware does not support it\n");
            return false;
    	}
    }

	// get bits per pixel and its length/offset
	int red_length, red_offset;
	int green_length, green_offset;
	int blue_length, blue_offset;
	int transp_length, transp_offset;
	int bits_per_pixel = getBitsPerPixel(pixelformat,
										 &red_length, &red_offset,
										 &green_length, &green_offset,
										 &blue_length, &blue_offset,
										 &transp_length, &transp_offset);

	// check if mode is already set
    if (!backbuffer) {
    	// do it only if no back buffer requested
		if    ((width == (int)this->var_screeninfo.xres) && (height == (int)this->var_screeninfo.yres)
			&& (bits_per_pixel == (int)this->var_screeninfo.bits_per_pixel) && (this->layers[0].pixelformat == pixelformat)) {
			// mode already set, no switch required
			printf("MMSFBDev: using preset mode %dx%d, pixelformat %s (%d bits)\n",
					width, height, getMMSFBPixelFormatString(pixelformat).c_str(), bits_per_pixel);
			return true;
		}
    }

	// searching for mode
    for (int cnt = 0; cnt < this->modes_cnt; cnt++) {
    	struct fb_var_screeninfo *mode = &this->modes[cnt];
    	if ((width == (int)mode->xres) && (height == (int)mode->yres) && (bits_per_pixel == (int)mode->bits_per_pixel)) {
    		// mode matches, switch to it
    	    this->var_screeninfo = *mode;

    	    this->var_screeninfo.activate = FB_ACTIVATE_NOW;
    	    this->var_screeninfo.accel_flags = 0;

			if(pixelformat == MMSFB_PF_I420) {
				this->var_screeninfo.nonstd = OMAPFB_COLOR_YUV420;
			} else {
			    this->var_screeninfo.red.length = red_length;
				this->var_screeninfo.red.offset = red_offset;
				this->var_screeninfo.red.msb_right = 0;
				this->var_screeninfo.green.length = green_length;
				this->var_screeninfo.green.offset = green_offset;
				this->var_screeninfo.green.msb_right = 0;
				this->var_screeninfo.blue.length = blue_length;
				this->var_screeninfo.blue.offset = blue_offset;
				this->var_screeninfo.blue.msb_right = 0;
				this->var_screeninfo.transp.length = transp_length;
				this->var_screeninfo.transp.offset = transp_offset;
				this->var_screeninfo.transp.msb_right = 0;
			}

			this->var_screeninfo.xres_virtual = this->var_screeninfo.xres;
			this->var_screeninfo.yres_virtual = this->var_screeninfo.yres * (backbuffer+1);
			this->var_screeninfo.xoffset = 0;
			this->var_screeninfo.yoffset = 0;
			this->var_screeninfo.grayscale = 0;

			do_switch = true;
			break;
    	}
	}

    if (!do_switch) {
        // no mode found
    	printf("MMSFBDev: no mode %dx%d, bit depth %d (%s) found in /etc/fb.modes\n",
    			width, height, bits_per_pixel, getMMSFBPixelFormatString(pixelformat).c_str());

    	// searching for mode with any pixelformat
        for (int cnt = 0; cnt < this->modes_cnt; cnt++) {
        	struct fb_var_screeninfo *mode = &this->modes[cnt];
        	if ((width == (int)mode->xres) && (height == (int)mode->yres)) {
        		// mode matches, switch to it
        	    this->var_screeninfo = *mode;

				printf("MMSFBDev: trying to use first mode %dx%d, bit depth %d from /etc/fb.modes\n", width, height, this->var_screeninfo.bits_per_pixel);

                this->var_screeninfo.activate = FB_ACTIVATE_NOW;
        	    this->var_screeninfo.accel_flags = 0;

        	    this->var_screeninfo.bits_per_pixel = bits_per_pixel;
				if(pixelformat == MMSFB_PF_I420) {
					this->var_screeninfo.nonstd = OMAPFB_COLOR_YUV420;
				} else {
		    	    this->var_screeninfo.red.length = red_length;
					this->var_screeninfo.red.offset = red_offset;
					this->var_screeninfo.green.length = green_length;
					this->var_screeninfo.green.offset = green_offset;
					this->var_screeninfo.blue.length = blue_length;
					this->var_screeninfo.blue.offset = blue_offset;
					this->var_screeninfo.transp.length = transp_length;
					this->var_screeninfo.transp.offset = transp_offset;
				}

    			this->var_screeninfo.xres_virtual = this->var_screeninfo.xres;
    			this->var_screeninfo.yres_virtual = this->var_screeninfo.yres * (backbuffer+1);
    			this->var_screeninfo.xoffset = 0;
    			this->var_screeninfo.yoffset = 0;
    			this->var_screeninfo.grayscale = 0;

    			do_switch = true;
    			break;
        	}
    	}
    }

    if (!do_switch) {
        // no mode found
    	printf("MMSFBDev: no mode %dx%d with any bit depth found in /etc/fb.modes\n", width, height);

        // check if resolution has changed
    	if  ((width == (int)this->var_screeninfo.xres) && (height == (int)this->var_screeninfo.yres)) {
			// resolution has not changed, so try to change only the pixelformat
			printf("MMSFBDev: resolution is the same, so try to change the pixelformat to %s\n",
					getMMSFBPixelFormatString(pixelformat).c_str());

			this->var_screeninfo.activate = FB_ACTIVATE_NOW;
			this->var_screeninfo.accel_flags = 0;

			this->var_screeninfo.bits_per_pixel = bits_per_pixel;
			if(pixelformat == MMSFB_PF_I420) {
				this->var_screeninfo.nonstd = OMAPFB_COLOR_YUV420;
			} else {
				this->var_screeninfo.red.length = red_length;
				this->var_screeninfo.red.offset = red_offset;
				this->var_screeninfo.green.length = green_length;
				this->var_screeninfo.green.offset = green_offset;
				this->var_screeninfo.blue.length = blue_length;
				this->var_screeninfo.blue.offset = blue_offset;
				this->var_screeninfo.transp.length = transp_length;
				this->var_screeninfo.transp.offset = transp_offset;
			}

			if (backbuffer) {
    			this->var_screeninfo.xres_virtual = this->var_screeninfo.xres;
				this->var_screeninfo.yres_virtual = this->var_screeninfo.yres * (backbuffer+1);
    			this->var_screeninfo.xoffset = 0;
    			this->var_screeninfo.yoffset = 0;
			}

			do_switch = true;
		}
    	else
    		//if  (1) {
		if  (this->layers[0].pixelformat == pixelformat) {
			// pixelformat has not changed, so try to change only the resolution
			printf("MMSFBDev: pixelformat is the same, so try to change the resolution to %dx%d\n",
					width, height);

			this->var_screeninfo.activate = FB_ACTIVATE_NOW;
			this->var_screeninfo.accel_flags = 0;

			this->var_screeninfo.xres = width;
			this->var_screeninfo.yres = height;

			this->var_screeninfo.xres_virtual = this->var_screeninfo.xres;
			this->var_screeninfo.yres_virtual = this->var_screeninfo.yres * (backbuffer+1);
			this->var_screeninfo.xoffset = 0;
			this->var_screeninfo.yoffset = 0;
			this->var_screeninfo.grayscale = 0;

			do_switch = true;
		}
    }

	if (do_switch) {
		//this->var_screeninfo.nonstd = 12;
		// switch now
	    if (ioctl(this->fd, FBIOPUT_VSCREENINFO, &this->var_screeninfo) < 0) {
	    	printf("MMSFBDev: could not switch to mode %dx%d, pixelformat %s (%d bits)\n",
	    			width, height, getMMSFBPixelFormatString(pixelformat).c_str(), bits_per_pixel);
	        return false;
	    }

	    // get fix screen infos
	    if (ioctl(this->fd, FBIOGET_FSCREENINFO, &this->fix_screeninfo) < 0) {
	    	printf("MMSFBDev: could not get fix screen infos\n");
	        return false;
	    }
	    printFixScreenInfo();

	    // get variable screen infos
	    if (ioctl(this->fd, FBIOGET_VSCREENINFO, &this->var_screeninfo) < 0) {
	    	printf("MMSFBDev: could not get var screen infos\n");
	        return false;
	    }
	    printVarScreenInfo();
    	printf("MMSFBDev: query plane\n");

//return true;

	}

	    struct omapfb_plane_info plane_info;
	    ioctl (this->fd, OMAPFB_QUERY_PLANE, &plane_info);
	    plane_info.enabled = 1;
	    plane_info.pos_x = 0;
	    plane_info.pos_y = 0;
	    plane_info.out_width = this->var_screeninfo.xres;
	    plane_info.out_height = this->var_screeninfo.yres;

	    printf("MMSFBDev: setup plane\n");
	    if (ioctl (this->fd, OMAPFB_SETUP_PLANE, &plane_info)) {
	    	printf("MMSFBDev: could not setup plane\n");
	        return false;

	    }
    	printf("MMSFBDev: done setup plane\n");






	return true;
}


#endif
