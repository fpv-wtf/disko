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

#include "mmsgui/mmssliderwidget.h"

MMSSliderWidget::MMSSliderWidget(MMSWindow *root, string className, MMSTheme *theme) {
    create(root, className, theme);
}

MMSSliderWidget::~MMSSliderWidget() {
	// delete the callbacks
    if (this->onSliderIncrement) delete this->onSliderIncrement;
    if (this->onSliderDecrement) delete this->onSliderDecrement;

    if (this->rootwindow) {
        this->rootwindow->im->releaseImage(this->image);
        this->rootwindow->im->releaseImage(this->selimage);
        this->rootwindow->im->releaseImage(this->image_p);
        this->rootwindow->im->releaseImage(this->selimage_p);
        this->rootwindow->im->releaseImage(this->image_i);
        this->rootwindow->im->releaseImage(this->selimage_i);
    }
}

bool MMSSliderWidget::create(MMSWindow *root, string className, MMSTheme *theme) {
	this->type = MMSWIDGETTYPE_SLIDER;
    this->className = className;

    // init attributes for drawable widgets
	this->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    if (theme) this->da->theme = theme; else this->da->theme = globalTheme;
    this->sliderWidgetClass = this->da->theme->getSliderWidgetClass(className);
    this->da->baseWidgetClass = &(this->da->theme->sliderWidgetClass.widgetClass);
    if (this->sliderWidgetClass) this->da->widgetClass = &(this->sliderWidgetClass->widgetClass); else this->da->widgetClass = NULL;

    /* clear */
    this->image = NULL;
    this->selimage = NULL;
    this->image_p = NULL;
    this->selimage_p = NULL;
    this->image_i = NULL;
    this->selimage_i = NULL;

    /* initialize the callbacks */
    this->onSliderIncrement = new sigc::signal<bool, MMSWidget*>::accumulated<neg_bool_accumulator>;
    this->onSliderDecrement = new sigc::signal<bool, MMSWidget*>::accumulated<neg_bool_accumulator>;

    /* create widget base */
    return MMSWidget::create(root, true, false, false, true, true, true, true);
}

MMSWidget *MMSSliderWidget::copyWidget() {
    /* create widget */
    MMSSliderWidget *newWidget = new MMSSliderWidget(this->rootwindow, className);

    /* copy widget */
    *newWidget = *this;

    /* copy base widget */
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    /* initialize the callbacks */
    this->onSliderIncrement = new sigc::signal<bool, MMSWidget*>::accumulated<neg_bool_accumulator>;
    this->onSliderDecrement = new sigc::signal<bool, MMSWidget*>::accumulated<neg_bool_accumulator>;

    /* reload my images */
    newWidget->image = NULL;
    newWidget->selimage = NULL;
    newWidget->image_p = NULL;
    newWidget->selimage_p = NULL;
    newWidget->image_i = NULL;
    newWidget->selimage_i = NULL;
    if (this->rootwindow) {
        newWidget->image = this->rootwindow->im->getImage(newWidget->getImagePath(), newWidget->getImageName());
        newWidget->selimage = this->rootwindow->im->getImage(newWidget->getSelImagePath(), newWidget->getSelImageName());
        newWidget->image_p = this->rootwindow->im->getImage(newWidget->getImagePath_p(), newWidget->getImageName_p());
        newWidget->selimage_p = this->rootwindow->im->getImage(newWidget->getSelImagePath_p(), newWidget->getSelImageName_p());
        newWidget->image_i = this->rootwindow->im->getImage(newWidget->getImagePath_i(), newWidget->getImageName_i());
        newWidget->selimage_i = this->rootwindow->im->getImage(newWidget->getSelImagePath_i(), newWidget->getSelImageName_i());
    }

    return newWidget;
}

bool MMSSliderWidget::init() {
    /* init widget basics */
    if (!MMSWidget::init())
        return false;

    /* load images */
    this->image = this->rootwindow->im->getImage(getImagePath(), getImageName());
    this->selimage = this->rootwindow->im->getImage(getSelImagePath(), getSelImageName());
    this->image_p = this->rootwindow->im->getImage(getImagePath_p(), getImageName_p());
    this->selimage_p = this->rootwindow->im->getImage(getSelImagePath_p(), getSelImageName_p());
    this->image_i = this->rootwindow->im->getImage(getImagePath_i(), getImageName_i());
    this->selimage_i = this->rootwindow->im->getImage(getSelImagePath_i(), getSelImageName_i());

    return true;
}

void MMSSliderWidget::getImage(MMSFBSurface **suf) {
	// searching for the image
	*suf = NULL;

	if (isActivated()) {
		if (isSelected())
			*suf = this->selimage;
		else
			*suf = this->image;
		if (isPressed()) {
			if (isSelected()) {
				if (this->selimage_p)
					*suf = this->selimage_p;
			}
			else {
				if (this->image_p)
					*suf = this->image_p;
			}
		}
	}
	else {
		if (isSelected())
			*suf = this->selimage_i;
		else
			*suf = this->image_i;
	}
}

void MMSSliderWidget::calcPos(MMSFBSurface *suf, MMSFBRectangle *surfaceGeom, bool *vertical) {
    // calculate position of the slider
	int w, h;
	suf->getSize(&w, &h);
	if (surfaceGeom->w < w) w = surfaceGeom->w;
	if (surfaceGeom->h < h) h = surfaceGeom->h;
	if (surfaceGeom->w - w < surfaceGeom->h - h) {
		// vertical slider
		surfaceGeom->y += ((surfaceGeom->h - h) * getPosition()) / 100;
		w = surfaceGeom->w;
		surfaceGeom->h = h;
		*vertical = true;
	}
	else {
		// horizontal slider
		surfaceGeom->x += ((surfaceGeom->w - w) * getPosition()) / 100;
		surfaceGeom->w = w;
		h = surfaceGeom->h;
		*vertical = false;
	}
}


bool MMSSliderWidget::draw(bool *backgroundFilled) {
    bool myBackgroundFilled = false;

    if (backgroundFilled) {
    	if (this->has_own_surface)
    		*backgroundFilled = false;
    }
    else
        backgroundFilled = &myBackgroundFilled;

    /* draw widget basics */
    if (MMSWidget::draw(backgroundFilled)) {

        /* lock */
        this->surface->lock();

        /* draw my things */
        MMSFBRectangle surfaceGeom = getSurfaceGeometry();

        // searching for the image
        MMSFBSurface *suf = NULL;
        getImage(&suf);
        if (suf) {
            // prepare for blitting
            this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(this->brightness, 255, opacity);

            // calculate position of the slider
            bool vertical;
            calcPos(suf, &surfaceGeom, &vertical);

            // blit
            this->surface->stretchBlit(suf, NULL, &surfaceGeom);
        }

        /* unlock */
        this->surface->unlock();

        /* update window surface with an area of surface */
        updateWindowSurfaceWithSurface(!*backgroundFilled);
    }

    /* draw widgets debug frame */
    return MMSWidget::drawDebug();
}

bool MMSSliderWidget::scrollTo(int posx, int posy, bool refresh, bool *changed) {
	if (changed)
		*changed = false;

	// searching for the image
    MMSFBSurface *suf = NULL;
    getImage(&suf);
    if (suf) {
        // calculate position of the slider
        MMSFBRectangle sgeom = getGeometry();
        bool vertical;
        calcPos(suf, &sgeom, &vertical);

        if (vertical) {
        	// vertical slider
            if (posy < sgeom.y) {
            	// slider decrement
            	if (!this->onSliderDecrement->emit(this)) {
            		// here we can dec the slider

            	}
            	if (changed) *changed = true;
           		return true;
            }
            else
            if (posy >= sgeom.y + sgeom.h) {
            	// slider increment
            	if (!this->onSliderIncrement->emit(this)) {
            		// here we can inc the slider

            	}
           		if (changed) *changed = true;
           		return true;
            }
        }
        else {
        	// horizontal slider
            if (posx < sgeom.x) {
            	// slider decrement
            	if (!this->onSliderDecrement->emit(this)) {
            		// here we can dec the slider

            	}
            	if (changed) *changed = true;
           		return true;
            }
            else
            if (posx >= sgeom.x + sgeom.w) {
            	// slider increment
            	if (!this->onSliderIncrement->emit(this)) {
            		// here we can inc the slider

            	}
           		if (changed) *changed = true;
           		return true;
            }
        }
    }

    return false;
}

/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETSLIDER(x) \
    if (this->mySliderWidgetClass.is##x()) return mySliderWidgetClass.get##x(); \
    else if ((sliderWidgetClass)&&(sliderWidgetClass->is##x())) return sliderWidgetClass->get##x(); \
    else return this->da->theme->sliderWidgetClass.get##x();

string MMSSliderWidget::getImagePath() {
    GETSLIDER(ImagePath);
}

string MMSSliderWidget::getImageName() {
    GETSLIDER(ImageName);
}

string MMSSliderWidget::getSelImagePath() {
    GETSLIDER(SelImagePath);
}

string MMSSliderWidget::getSelImageName() {
    GETSLIDER(SelImageName);
}

string MMSSliderWidget::getImagePath_p() {
    GETSLIDER(ImagePath_p);
}

string MMSSliderWidget::getImageName_p() {
    GETSLIDER(ImageName_p);
}

string MMSSliderWidget::getSelImagePath_p() {
    GETSLIDER(SelImagePath_p);
}

string MMSSliderWidget::getSelImageName_p() {
    GETSLIDER(SelImageName_p);
}

string MMSSliderWidget::getImagePath_i() {
    GETSLIDER(ImagePath_i);
}

string MMSSliderWidget::getImageName_i() {
    GETSLIDER(ImageName_i);
}

string MMSSliderWidget::getSelImagePath_i() {
    GETSLIDER(SelImagePath_i);
}

string MMSSliderWidget::getSelImageName_i() {
    GETSLIDER(SelImageName_i);
}

unsigned int MMSSliderWidget::getPosition() {
    GETSLIDER(Position);
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSSliderWidget::setImagePath(string imagepath, bool load, bool refresh) {
    mySliderWidgetClass.setImagePath(imagepath);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->image);
            this->image = this->rootwindow->im->getImage(getImagePath(), getImageName());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setImageName(string imagename, bool load, bool refresh) {
    mySliderWidgetClass.setImageName(imagename);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->image);
            this->image = this->rootwindow->im->getImage(getImagePath(), getImageName());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setImage(string imagepath, string imagename, bool load, bool refresh) {
    mySliderWidgetClass.setImagePath(imagepath);
    mySliderWidgetClass.setImageName(imagename);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->image);
            this->image = this->rootwindow->im->getImage(getImagePath(), getImageName());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setSelImagePath(string selimagepath, bool load, bool refresh) {
    mySliderWidgetClass.setSelImagePath(selimagepath);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->selimage);
            this->selimage = this->rootwindow->im->getImage(getSelImagePath(), getSelImageName());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setSelImageName(string selimagename, bool load, bool refresh) {
    mySliderWidgetClass.setSelImageName(selimagename);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->selimage);
            this->selimage = this->rootwindow->im->getImage(getSelImagePath(), getSelImageName());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setSelImage(string selimagepath, string selimagename, bool load, bool refresh) {
    mySliderWidgetClass.setSelImagePath(selimagepath);
    mySliderWidgetClass.setSelImageName(selimagename);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->selimage);
            this->selimage = this->rootwindow->im->getImage(getSelImagePath(), getSelImageName());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setImagePath_p(string imagepath_p, bool load, bool refresh) {
    mySliderWidgetClass.setImagePath_p(imagepath_p);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->image_p);
            this->image_p = this->rootwindow->im->getImage(getImagePath_p(), getImageName_p());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setImageName_p(string imagename_p, bool load, bool refresh) {
    mySliderWidgetClass.setImageName_p(imagename_p);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->image_p);
            this->image_p = this->rootwindow->im->getImage(getImagePath_p(), getImageName_p());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setImage_p(string imagepath_p, string imagename_p, bool load, bool refresh) {
    mySliderWidgetClass.setImagePath_p(imagepath_p);
    mySliderWidgetClass.setImageName_p(imagename_p);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->image_p);
            this->image_p = this->rootwindow->im->getImage(getImagePath_p(), getImageName_p());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setSelImagePath_p(string selimagepath_p, bool load, bool refresh) {
    mySliderWidgetClass.setSelImagePath_p(selimagepath_p);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->selimage_p);
            this->selimage_p = this->rootwindow->im->getImage(getSelImagePath_p(), getSelImageName_p());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setSelImageName_p(string selimagename_p, bool load, bool refresh) {
    mySliderWidgetClass.setSelImageName_p(selimagename_p);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->selimage_p);
            this->selimage_p = this->rootwindow->im->getImage(getSelImagePath_p(), getSelImageName_p());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setSelImage_p(string selimagepath_p, string selimagename_p, bool load, bool refresh) {
    mySliderWidgetClass.setSelImagePath_p(selimagepath_p);
    mySliderWidgetClass.setSelImageName_p(selimagename_p);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->selimage_p);
            this->selimage_p = this->rootwindow->im->getImage(getSelImagePath_p(), getSelImageName_p());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setImagePath_i(string imagepath_i, bool load, bool refresh) {
    mySliderWidgetClass.setImagePath_i(imagepath_i);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->image_i);
            this->image_i = this->rootwindow->im->getImage(getImagePath_i(), getImageName_i());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setImageName_i(string imagename_i, bool load, bool refresh) {
    mySliderWidgetClass.setImageName_i(imagename_i);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->image_i);
            this->image_i = this->rootwindow->im->getImage(getImagePath_i(), getImageName_i());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setImage_i(string imagepath_i, string imagename_i, bool load, bool refresh) {
    mySliderWidgetClass.setImagePath_i(imagepath_i);
    mySliderWidgetClass.setImageName_i(imagename_i);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->image_i);
            this->image_i = this->rootwindow->im->getImage(getImagePath_i(), getImageName_i());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setSelImagePath_i(string selimagepath_i, bool load, bool refresh) {
    mySliderWidgetClass.setSelImagePath_i(selimagepath_i);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->selimage_i);
            this->selimage_i = this->rootwindow->im->getImage(getSelImagePath_i(), getSelImageName_i());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setSelImageName_i(string selimagename_i, bool load, bool refresh) {
    mySliderWidgetClass.setSelImageName_i(selimagename_i);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->selimage_i);
            this->selimage_i = this->rootwindow->im->getImage(getSelImagePath_i(), getSelImageName_i());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setSelImage_i(string selimagepath_i, string selimagename_i, bool load, bool refresh) {
    mySliderWidgetClass.setSelImagePath_i(selimagepath_i);
    mySliderWidgetClass.setSelImageName_i(selimagename_i);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->selimage_i);
            this->selimage_i = this->rootwindow->im->getImage(getSelImagePath_i(), getSelImageName_i());
        }
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::setPosition(unsigned int pos, bool refresh) {
    mySliderWidgetClass.setPosition(pos);
    if (refresh)
        this->refresh();
}

void MMSSliderWidget::updateFromThemeClass(MMSSliderWidgetClass *themeClass) {
    if (themeClass->isImagePath())
        setImagePath(themeClass->getImagePath());
    if (themeClass->isImageName())
        setImageName(themeClass->getImageName());
    if (themeClass->isSelImagePath())
        setSelImagePath(themeClass->getSelImagePath());
    if (themeClass->isSelImageName())
        setSelImageName(themeClass->getSelImageName());
    if (themeClass->isImagePath_p())
        setImagePath_p(themeClass->getImagePath_p());
    if (themeClass->isImageName_p())
        setImageName_p(themeClass->getImageName_p());
    if (themeClass->isSelImagePath_p())
        setSelImagePath_p(themeClass->getSelImagePath_p());
    if (themeClass->isSelImageName_p())
        setSelImageName_p(themeClass->getSelImageName_p());
    if (themeClass->isImagePath_i())
        setImagePath_i(themeClass->getImagePath_i());
    if (themeClass->isImageName_i())
        setImageName_i(themeClass->getImageName_i());
    if (themeClass->isSelImagePath_i())
        setSelImagePath_i(themeClass->getSelImagePath_i());
    if (themeClass->isSelImageName_i())
        setSelImageName_i(themeClass->getSelImageName_i());
    if (themeClass->isPosition())
        setPosition(themeClass->getPosition());

    MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
