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

#include "mmsgui/mmswidget.h"
#include "mmsgui/mmswidgetthread.h"
#include "mmsgui/mmsborder.h"
#include "mmsgui/mmsmenuwidget.h"
#include "mmsgui/mmssliderwidget.h"
#include <string.h>

// static variables
string MMSWidget_inputmode = "";

MMSWidget::MMSWidget() {
	// per default no attributes for drawable widgets are allocated
	this->da = NULL;

    this->initialized = false;
    this->rootwindow = NULL;
    this->parent_rootwindow = NULL;
//    this->dfb = NULL;
    this->windowSurface = NULL;
    this->parent = NULL;
    this->focused = false;
    this->selected = false;
    this->activated = true;
    this->pressed = false;

    this->brightness = 255;
    this->opacity = 255;

    this->name = "";
    MMSIdFactory factory;
    this->id = factory.getId();

    this->toRedraw = false;
    this->redrawChildren = false;
    this->visible = true;


    this->surface = NULL;

    memset(&(this->geom), 0, sizeof(this->geom));
    memset(&(this->innerGeom), 0, sizeof(this->innerGeom));
    memset(&(this->surfaceGeom), 0, sizeof(this->surfaceGeom));



//TODO: textbox widget should have its one surface
this->has_own_surface = false;
}

MMSWidget::~MMSWidget() {

	if (this->da) {
		// delete widget thread
		if (this->da->widgetthread)
			delete this->da->widgetthread;
	}

	// delete the callbacks
    if (onSelect) delete onSelect;
    if (onFocus)  delete onFocus;
    if (onReturn) delete onReturn;
    if (onClick)  delete onClick;

    // delete images
    if (this->rootwindow) {
    	if (this->da) {
			this->rootwindow->im->releaseImage(this->da->bgimage);
			this->rootwindow->im->releaseImage(this->da->selbgimage);
			this->rootwindow->im->releaseImage(this->da->bgimage_p);
			this->rootwindow->im->releaseImage(this->da->selbgimage_p);
			this->rootwindow->im->releaseImage(this->da->bgimage_i);
			this->rootwindow->im->releaseImage(this->da->selbgimage_i);
			for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++)
				this->rootwindow->im->releaseImage(this->da->borderimages[i]);
			for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++)
				this->rootwindow->im->releaseImage(this->da->borderselimages[i]);
    	}
    }

    // delete children
    for (unsigned int i = 0; i < children.size(); i++)
        delete children.at(i);

    // remove me from root window list
    if (this->rootwindow)
        this->rootwindow->remove(this);

    if(this->surface)
        delete this->surface;

    // delete attributes which are set for drawable widgets
    if (this->da)
    	delete this->da;
}

MMSWIDGETTYPE MMSWidget::getType() {
	return this->type;
}

bool MMSWidget::create(MMSWindow *root, bool drawable, bool needsparentdraw, bool focusable, bool selectable,
                       bool canhavechildren, bool canselectchildren, bool clickable) {
    bool		b;

//    logger.writeLog("Create MMSWidget");

	if (drawable) {
		// init attributes for drawable widgets
		// we assume, that this->da will be allocated by the caller!!!
		this->da->widgetthread = NULL;
	    this->da->bgimage = NULL;
	    this->da->selbgimage = NULL;
	    this->da->bgimage_p = NULL;
	    this->da->selbgimage_p = NULL;
	    this->da->bgimage_i = NULL;
	    this->da->selbgimage_i = NULL;
	    for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++)
	        this->da->borderimages[i] = NULL;
	    da->bordergeomset = false;
	    for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++)
	        this->da->borderselimages[i] = NULL;
	    da->borderselgeomset = false;

	    this->da->upArrowWidget		= NULL;
	    this->da->downArrowWidget	= NULL;
	    this->da->leftArrowWidget	= NULL;
	    this->da->rightArrowWidget	= NULL;
	    this->da->initialArrowsDrawn= false;

	    this->da->navigateUpWidget		= NULL;
	    this->da->navigateDownWidget	= NULL;
	    this->da->navigateLeftWidget	= NULL;
	    this->da->navigateRightWidget	= NULL;

	    this->da->vSliderWidget = NULL;
	    this->da->hSliderWidget = NULL;

	    this->da->scrollPosX = 0;
	    this->da->scrollPosY = 0;
	    this->da->scrollDX = 8;
	    this->da->scrollDY = 8;

	    this->da->joinedWidget = NULL;

	    onSelect = new sigc::signal<void, MMSWidget*>;
	    onFocus  = new sigc::signal<void, MMSWidget*, bool>;
	    onReturn = new sigc::signal<void, MMSWidget*>;
	    onClick  = new sigc::signal<void, MMSWidget*, int, int, int, int>;
	}
	else {
		// init attributes for non-drawable widgets
	    onSelect = NULL;
	    onFocus  = NULL;
	    onReturn = NULL;
	    onClick  = NULL;
	}

    this->drawable = drawable;
    this->needsparentdraw = needsparentdraw;
    this->focusable_initial = focusable;
    if (!this->focusable_initial)
        if (getFocusable(b))
			if (b)
				setFocusable(false, false);
    this->selectable_initial = selectable;
    if (!this->selectable_initial)
        if (getSelectable(b))
			if (b)
				setSelectable(false, false);
    this->canhavechildren = canhavechildren;
    this->canselectchildren = canselectchildren;
    this->clickable_initial = clickable;
    if (!this->clickable_initial)
        if (getClickable(b))
			if (b)
				setClickable(false);

    setRootWindow(root);
    if (this->rootwindow) {
        this->windowSurface = this->rootwindow->getSurface();
    }
    this->sizehint.clear();
    this->geomset=false;




//    logger.writeLog("MMSWidget created");

    return true;
}

void MMSWidget::copyWidget(MMSWidget *newWidget) {
    /* get new id */
    MMSIdFactory factory;
    newWidget->id = factory.getId();

    /* copy my children */
    for (unsigned int i = 0; i < children.size(); i++)
        newWidget->children.at(i) = children.at(i)->copyWidget();

    if (drawable) {
    	// copy attributes for drawable widgets
    	newWidget->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    	*(newWidget->da) = *(this->da);
    }

    /* initialize the callbacks */
    onSelect = new sigc::signal<void, MMSWidget*>;
    onFocus  = new sigc::signal<void, MMSWidget*, bool>;
    onReturn = new sigc::signal<void, MMSWidget*>;
    onClick  = new sigc::signal<void, MMSWidget*, int, int, int, int>;

    if (drawable) {
		// reload my images
		newWidget->da->bgimage = NULL;
		newWidget->da->selbgimage = NULL;
		newWidget->da->bgimage_p = NULL;
		newWidget->da->selbgimage_p = NULL;
		newWidget->da->bgimage_i = NULL;
		newWidget->da->selbgimage_i = NULL;
		for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++)
			newWidget->da->borderimages[i] = NULL;
		for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++)
			newWidget->da->borderselimages[i] = NULL;

        if (this->rootwindow) {
            string path, name;

            if (!newWidget->getBgImagePath(path)) path = "";
            if (!newWidget->getBgImageName(name)) name = "";
            newWidget->da->bgimage = this->rootwindow->im->getImage(path, name);

            if (!newWidget->getSelBgImagePath(path)) path = "";
            if (!newWidget->getSelBgImageName(name)) name = "";
            newWidget->da->selbgimage = this->rootwindow->im->getImage(path, name);

            if (!newWidget->getBgImagePath_p(path)) path = "";
            if (!newWidget->getBgImageName_p(name)) name = "";
            newWidget->da->bgimage_p = this->rootwindow->im->getImage(path, name);

            if (!newWidget->getSelBgImagePath_p(path)) path = "";
            if (!newWidget->getSelBgImageName_p(name)) name = "";
            newWidget->da->selbgimage_p = this->rootwindow->im->getImage(path, name);

            if (!newWidget->getBgImagePath_i(path)) path = "";
            if (!newWidget->getBgImageName_i(name)) name = "";
            newWidget->da->bgimage_i = this->rootwindow->im->getImage(path, name);

            if (!newWidget->getSelBgImagePath_i(path)) path = "";
            if (!newWidget->getSelBgImageName_i(name)) name = "";
            newWidget->da->selbgimage_i = this->rootwindow->im->getImage(path, name);

            if (!newWidget->getBorderImagePath(path)) path = "";
            for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++) {
	            if (!newWidget->getBorderImageNames((MMSBORDER_IMAGE_NUM)i, name)) name = "";
	            newWidget->da->borderimages[i] = this->rootwindow->im->getImage(path, name);
            }

            if (!newWidget->getBorderSelImagePath(path)) path = "";
            for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++) {
	            if (!newWidget->getBorderSelImageNames((MMSBORDER_IMAGE_NUM)i, name)) name = "";
	            newWidget->da->borderselimages[i] = this->rootwindow->im->getImage(path, name);
            }
        }
    }
}

MMSWidget* MMSWidget::getChild(unsigned int atpos) {
    if (atpos < children.size())
        return children.at(atpos);
    else
        return NULL;
}

MMSWidget* MMSWidget::disconnectChild(unsigned int atpos) {
    if (atpos < children.size()) {
        MMSWidget *widget = children.at(atpos);
        children.erase(children.begin()+atpos);
        return  widget;
    }
    else
        return NULL;
}

MMSWidget* MMSWidget::searchForWidget(string name) {
    MMSWidget *widget;

    if (name=="")
        return NULL;

    /* first, my own children */
    for (unsigned int i = 0; i < children.size(); i++)
        if (children.at(i)->getName() == name)
            return children.at(i);

    /* second, call search method of my children */
    for (unsigned int i = 0; i < children.size(); i++)
        if ((widget = children.at(i)->searchForWidget(name)))
            return widget;

    return NULL;
}

MMSWidget* MMSWidget::searchForWidgetType(MMSWIDGETTYPE type) {
    MMSWidget *widget;

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

MMSWidget* MMSWidget::operator[](string name) {
    MMSWidget *widget;

    if ((widget = searchForWidget(name)))
        return widget;
    throw new MMSWidgetError(1, "widget " + name + " not found");
}



void MMSWidget::setSurfaceGeometry(unsigned int width, unsigned int height) {
    MMSFBRectangle mygeom;

    if (!drawable)
        return;

    mygeom.x = 0;
    mygeom.y = 0;
    if ((int)width > this->innerGeom.w)
        mygeom.w = width;
    else
        mygeom.w = this->innerGeom.w;
    if ((int)height > this->innerGeom.h)
        mygeom.h = height;
    else
        mygeom.h = this->innerGeom.h;

    if ((this->surfaceGeom.w != mygeom.w)||(this->surfaceGeom.h != mygeom.h)) {

        /* surface geom has changed */
        this->surfaceGeom = mygeom;

        /* create or change my surface */
        if (this->surface) {
            delete this->surface;
            this->surface = NULL;
        }

        if (this->has_own_surface) {
        	/* has own surface, create it */
        	this->windowSurface->createCopy(&(this->surface), this->surfaceGeom.w, this->surfaceGeom.h);
        }
        else {
        	/* get sub surface */
        	this->surface = this->windowSurface->getSubSurface(&(this->innerGeom));
        }
    }
    else {
        if (!this->has_own_surface)
        	if (this->surface) {
    	    	/* position of sub surface has changed */
    		    this->surfaceGeom = mygeom;

    		    /* move the sub surface */
    		    this->surface->moveTo(this->innerGeom.x, this->innerGeom.y);
        	}
    }
}

MMSFBRectangle MMSWidget::getSurfaceGeometry() {
    return this->surfaceGeom;
}

MMSFBSurface *MMSWidget::getSurface() {
    return this->surface;
}

void MMSWidget::setInnerGeometry() {
    MMSFBRectangle mygeom;
    unsigned int diff = 0;

    /* check something */
    if (!isGeomSet())
        return;

    /* calculate geometry */
    mygeom = this->geom;


    if (drawable) {
    	unsigned int margin, borderthickness, bordermargin;
    	if (!getMargin(margin))
    		margin = 0;
    	if (!getBorderThickness(borderthickness))
    		borderthickness = 0;
    	if (!getBorderMargin(bordermargin))
    		bordermargin = 0;
        diff = margin + borderthickness + bordermargin;
    }

    if ((int)(2*diff) >= mygeom.w) {
        diff = mygeom.w / 2 - 1;
    }

    if ((int)(2*diff) >= mygeom.h) {
        diff = mygeom.h / 2 - 1;
    }

    mygeom.x+= diff;
    mygeom.y+= diff;
    mygeom.w-= 2*diff;
    mygeom.h-= 2*diff;

    if (memcmp(&(this->innerGeom), &mygeom, sizeof(mygeom))) {
        /* inner geom has changed */
        this->innerGeom = mygeom;

        /* set surface geometry */
        setSurfaceGeometry();
    }
}

MMSFBRectangle MMSWidget::getInnerGeometry() {
    return this->innerGeom;
}


void MMSWidget::setGeometry(MMSFBRectangle geom) {
    MMSFBRectangle oldgeom;
    bool dimChanged = true;


    if (this->geomset) {
        /* dimension has changed? */
        dimChanged = ((this->geom.w!=geom.w)||(this->geom.h!=geom.h));

        /* recalculate widgets border */
        if (dimChanged) {
            /* the dimension has changed */
        	if (drawable) {
				this->da->bordergeomset=false;
				this->da->borderselgeomset=false;
        	}
        }
        else {
            if (this->geom.x!=geom.x) {
                /* x pos has changed */
            	if (drawable) {
					if ((this->da->bordergeomset)||(this->da->borderselgeomset)) {
						int diff = this->geom.x - geom.x;
						for (int i=0;i<8;i++) {
							this->da->bordergeom[i].x -= diff;
							this->da->borderselgeom[i].x -= diff;
						}
					}
            	}
            }
            if (this->geom.y!=geom.y) {
                /* y pos has changed */
            	if (drawable) {
					if ((this->da->bordergeomset)||(this->da->borderselgeomset)) {
						int diff = this->geom.y - geom.y;
						for (int i=0;i<8;i++) {
							this->da->bordergeom[i].y -= diff;
							this->da->borderselgeom[i].y -= diff;
						}
					}
            	}
            }
        }
    }

    /* set new geom */
    this->geomset = true;
    oldgeom = this->geom;
    this->geom = geom;



    if (this->has_own_surface) {
	    if (dimChanged) {
	        /* calculate complete inner geometry */
	        setInnerGeometry();
	    }
	    else {
	        /* change only x and y values for inner geometry */
	        this->innerGeom.x+= this->geom.x - oldgeom.x;
	        this->innerGeom.y+= this->geom.y - oldgeom.y;
	    }
    }
	else {
        /* have only a subsurface, re-calculate inner geometry (e.g. move) */
        setInnerGeometry();
	}

    /* calculate my children */
    this->recalculateChildren();

}

MMSFBRectangle MMSWidget::getGeometry() {
    return this->geom;
}

MMSFBRectangle MMSWidget::getRealGeometry() {
	MMSFBRectangle r1,r2;

	/* have a parent widget? */
	if (!this->parent) {
		/* no, go to my window? */
		if (!this->rootwindow)
		    return this->geom;

		/* yes */
		if (!isGeomSet()) {
			this->rootwindow->recalculateChildren();
		}
		r1 = this->geom;
		r2 = this->rootwindow->getRealGeometry();
		r1.x+=r2.x;
		r1.y+=r2.y;
		return r1;
	}

	/* yes */
	r1 = this->geom;
	r2 = this->parent->getRealGeometry();
	r1.x+=r2.x;
	r1.y+=r2.y;
	return r1;
}



bool MMSWidget::loadArrowWidgets() {
	bool  	b;
	string 	s;

	if (!this->drawable)
		return false;

	// connect arrow widgets
    if (!this->da->upArrowWidget)
    	if (getUpArrow(s))
    		if (s != "")
		        if (this->rootwindow)
		            if ((this->da->upArrowWidget = this->rootwindow->searchForWidget(s))) {
		                if (!this->da->upArrowWidget->getSelectable(b))
		                    this->da->upArrowWidget = NULL;
		                else
			                if (!b)
			                    this->da->upArrowWidget = NULL;
		            }

    if (!this->da->downArrowWidget)
    	if (getDownArrow(s))
    		if (s != "")
		        if (this->rootwindow)
		            if ((this->da->downArrowWidget = this->rootwindow->searchForWidget(s))) {
		                if (!this->da->downArrowWidget->getSelectable(b))
		                    this->da->downArrowWidget = NULL;
		                else
			                if (!b)
			                    this->da->downArrowWidget = NULL;
		            }

    if (!this->da->leftArrowWidget)
    	if (getLeftArrow(s))
    		if (s != "")
		        if (this->rootwindow)
		            if ((this->da->leftArrowWidget = this->rootwindow->searchForWidget(s))) {
		                if (!this->da->leftArrowWidget->getSelectable(b))
		                    this->da->leftArrowWidget = NULL;
		                else
			                if (!b)
			                    this->da->leftArrowWidget = NULL;
		            }

    if (!this->da->rightArrowWidget)
    	if (getRightArrow(s))
    		if (s != "")
		        if (this->rootwindow)
		            if ((this->da->rightArrowWidget = this->rootwindow->searchForWidget(s))) {
		                if (!this->da->rightArrowWidget->getSelectable(b))
		                    this->da->rightArrowWidget = NULL;
		                else
			                if (!b)
			                    this->da->rightArrowWidget = NULL;
		            }

    return true;
}

void MMSWidget::switchArrowWidgets() {
    // connect arrow widgets
    if (!loadArrowWidgets())
    	return;

    // switch arrow widgets
    if (this->da->upArrowWidget) {
        if (this->da->scrollPosY == 0)
            this->da->upArrowWidget->setSelected(false);
        else
            this->da->upArrowWidget->setSelected(true);
    }

    if (this->da->downArrowWidget) {
        if (this->surfaceGeom.h - this->surfaceGeom.y - (int)this->da->scrollPosY > this->innerGeom.h)
            this->da->downArrowWidget->setSelected(true);
        else
            this->da->downArrowWidget->setSelected(false);
    }

    if (this->da->leftArrowWidget) {
        if (this->da->scrollPosX == 0)
            this->da->leftArrowWidget->setSelected(false);
        else
            this->da->leftArrowWidget->setSelected(true);
    }

    if (this->da->rightArrowWidget) {
        if (this->surfaceGeom.w - this->surfaceGeom.x - (int)this->da->scrollPosX > this->innerGeom.w)
            this->da->rightArrowWidget->setSelected(true);
        else
            this->da->rightArrowWidget->setSelected(false);
    }
}

bool MMSWidget::setScrollSize(unsigned int dX, unsigned int dY) {
	if (this->da) {
		this->da->scrollDX = dX;
		this->da->scrollDY = dY;
		return true;
	}
	return false;
}

bool MMSWidget::setScrollPos(int posX, int posY, bool refresh, bool test) {
    if (!isGeomSet()) {
        /* i must have my geometry */
        MMSWindow *root = getRootWindow();
        if (root) {
            root->recalculateChildren();
        }
        else
            return false;
    }

    if (!this->surface) return false;

    if (posX < 0) {
        if (this->da->scrollPosX > 0)
            posX = 0;
        else
            return false;
    }

    if (posX + innerGeom.w > surfaceGeom.w) {
        if ((int)this->da->scrollPosX + innerGeom.w < surfaceGeom.w)
            posX = surfaceGeom.w - innerGeom.w;
        else
            return false;
    }

    if (posY < 0) {
        if (this->da->scrollPosY > 0)
            posY = 0;
        else
            return false;
    }

    if (posY + innerGeom.h > surfaceGeom.h) {
        if ((int)this->da->scrollPosY + innerGeom.h < surfaceGeom.h)
            posY = surfaceGeom.h - innerGeom.h;
        else
            return false;
    }

    if (!test) {
        this->da->scrollPosX = posX;
        this->da->scrollPosY = posY;

        if (refresh)
            this->refresh();
    }

    return true;
}


bool MMSWidget::scrollDown(unsigned int count, bool refresh, bool test, bool leave_selection) {
	if (!this->da) return false;
    if (setScrollPos((int)this->da->scrollPosX, (int)this->da->scrollPosY + count*(int)this->da->scrollDY, refresh, test)) {
        if (!test)
            switchArrowWidgets();
        return true;
    }
    return false;
}

bool MMSWidget::scrollUp(unsigned int count, bool refresh, bool test, bool leave_selection) {
	if (!this->da) return false;
    if (setScrollPos((int)this->da->scrollPosX, (int)this->da->scrollPosY - count*(int)this->da->scrollDY, refresh, test)) {
        if (!test)
            switchArrowWidgets();
        return true;
    }
    return false;
}

bool MMSWidget::scrollRight(unsigned int count, bool refresh, bool test, bool leave_selection) {
	if (!this->da) return false;
    if (setScrollPos((int)this->da->scrollPosX + count*(int)this->da->scrollDX, (int)this->da->scrollPosY, refresh, test)) {
        if (!test)
            switchArrowWidgets();
        return true;
    }
    return false;
}

bool MMSWidget::scrollLeft(unsigned int count, bool refresh, bool test, bool leave_selection) {
	if (!this->da) return false;
    if (setScrollPos((int)this->da->scrollPosX - count*(int)this->da->scrollDX, (int)this->da->scrollPosY, refresh, test)) {
        if (!test)
            switchArrowWidgets();
        return true;
    }
    return false;
}

bool MMSWidget::scrollTo(int posx, int posy, bool refresh, bool *changed, MMSWIDGET_SCROLL_MODE mode, MMSFBRectangle *inputrect) {
	if (changed)
		*changed = false;
	return true;
}

MMSFBRectangle MMSWidget::getVisibleSurfaceArea() {
    MMSFBRectangle area;

    area.x = surfaceGeom.x + (this->da)?this->da->scrollPosX:0;
    area.y = surfaceGeom.y + (this->da)?this->da->scrollPosY:0;
    area.w = innerGeom.w;
    area.h = innerGeom.h;

    return area;
}

void MMSWidget::updateWindowSurfaceWithSurface(bool useAlphaChannel) {

	if (this->has_own_surface) {
		/* have own surface */
		MMSFBRectangle area = getVisibleSurfaceArea();

	    /* lock */
	    this->windowSurface->lock();

	    this->windowSurface->setBlittingFlags(MMSFB_BLIT_NOFX);
	    this->windowSurface->blit(this->surface, &area, innerGeom.x, innerGeom.y);

	    /* unlock */
	    this->windowSurface->unlock();
	}
}

bool MMSWidget::init() {
    if (drawable) {
        if (!this->rootwindow)
            return false;

        string path, name;

        if (!getBgImagePath(path)) path = "";
        if (!getBgImageName(name)) name = "";
        this->da->bgimage = this->rootwindow->im->getImage(path, name);

        if (!getSelBgImagePath(path)) path = "";
        if (!getSelBgImageName(name)) name = "";
        this->da->selbgimage = this->rootwindow->im->getImage(path, name);

        if (!getBgImagePath_p(path)) path = "";
        if (!getBgImageName_p(name)) name = "";
        this->da->bgimage_p = this->rootwindow->im->getImage(path, name);

        if (!getSelBgImagePath_p(path)) path = "";
        if (!getSelBgImageName_p(name)) name = "";
        this->da->selbgimage_p = this->rootwindow->im->getImage(path, name);

        if (!getBgImagePath_i(path)) path = "";
        if (!getBgImageName_i(name)) name = "";
        this->da->bgimage_i = this->rootwindow->im->getImage(path, name);

        if (!getSelBgImagePath_i(path)) path = "";
        if (!getSelBgImageName_i(name)) name = "";
        this->da->selbgimage_i = this->rootwindow->im->getImage(path, name);

        if (!getBorderImagePath(path)) path = "";
        for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++) {
            if (!getBorderImageNames((MMSBORDER_IMAGE_NUM)i, name)) name = "";
            this->da->borderimages[i] = this->rootwindow->im->getImage(path, name);
        }

        if (!getBorderSelImagePath(path)) path = "";
        for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++) {
            if (!getBorderSelImageNames((MMSBORDER_IMAGE_NUM)i, name)) name = "";
            this->da->borderselimages[i] = this->rootwindow->im->getImage(path, name);
        }

        /* get my four widgets to which I have to navigate */
        if (!getNavigateUp(name)) name = "";
        this->da->navigateUpWidget = this->rootwindow->searchForWidget(name);

        if (!getNavigateDown(name)) name = "";
        this->da->navigateDownWidget = this->rootwindow->searchForWidget(name);

        if (!getNavigateLeft(name)) name = "";
        this->da->navigateLeftWidget = this->rootwindow->searchForWidget(name);

        if (!getNavigateRight(name)) name = "";
        this->da->navigateRightWidget = this->rootwindow->searchForWidget(name);

        /* get my two widgets which represents the sliders */
        if (!getVSlider(name)) name = "";
        this->da->vSliderWidget = this->rootwindow->searchForWidget(name);

        if (!getHSlider(name)) name = "";
        this->da->hSliderWidget = this->rootwindow->searchForWidget(name);

        // get widget which is joined to me
        if (!getJoinedWidget(name)) name = "";
        this->da->joinedWidget = this->rootwindow->searchForWidget(name);
    }
    return true;
}


#ifdef OLDDRAW
bool MMSWidget(bool *backgroundFilled) {
    bool         myBackgroundFilled = false;
    bool         retry = false;

    if (!this->initialized) {
        /* init widget (e.g. load images, fonts, ...) */
        init();
        this->initialized = true;
    }

    if (backgroundFilled)
        *backgroundFilled = false;
    else
        backgroundFilled = &myBackgroundFilled;

    if ((!this->geomset)||(!this->visible))
        return false;

#ifdef __PUPTRACE__
    DEBUGOUT("draw widget = %s ***************************\n", getName().c_str());
#endif

    /* lock */
    if (this->surface) this->surface->lock();
    this->windowSurface->lock();
/*
if (!this->has_own_surface) {
//	if (this->surface) this->surface->clear(0xff,0,0,0xff);
	*backgroundFilled = true;
	if (this->surface) this->surface->unlock();
	this->windowSurface->unlock();
	return true;
}
*/
    /* draw background */
    do {
        /* searching for the background color or image */
        MMSFBColor col;
        MMSFBSurface *suf = NULL;
        col.a = 0;

        if (this->drawable) {
            if (isActivated()) {
                if (isSelected()) {
                    getSelBgColor(col);
                    suf = this->selbgimage;
                }
                else {
                    getBgColor(col);
                    suf = this->bgimage;
                }
            }
            else {
                if (isSelected()) {
                    getSelBgColor_i(col);
                    suf = this->selbgimage_i;
                }
                else {
                    getBgColor_i(col);
                    suf = this->bgimage_i;
                }
            }
        }

        if (suf) {
            if ((*backgroundFilled)||(retry)||(!this->has_own_surface)) {
                /* prepare for blitting */
                this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, (col.a)?col.a:255, opacity);

                /* fill background */
                surface->stretchBlit(suf, NULL, &surfaceGeom);
                *backgroundFilled = true;

                /* go out of the loop */
                break;
            }
            else
                /* the color has an alpha value and I need a background before drawing the image */
                retry = true;
        }
        else
        if (col.a) {
            if ((*backgroundFilled)||(retry)||(!this->has_own_surface)||((col.a==255)&&(opacity==255))) {
                /* prepare for drawing */
                this->surface->setDrawingColorAndFlagsByBrightnessAndOpacity(col, brightness, opacity);

                /* fill background */
                this->surface->fillRectangle(surfaceGeom.x, surfaceGeom.y, surfaceGeom.w, surfaceGeom.h);
                *backgroundFilled = true;

                /* go out of the loop */
                break;
            }
            else
                /* the color has an alpha value and I need a background for it */
                retry = true;
        }
        else {
            if ((*backgroundFilled)||(!this->has_own_surface))
                /* go out of the loop */
                break;
            else
                /* no color, no image, I need to search for a background */
                retry = true;
        }

        /* if not filled then */
        if (!*backgroundFilled) {
            /* searching for the next parent widget */
            MMSWidget *widget = NULL;
            vector<MMSWidget*> wlist;
            if (this->parent)
                widget = this->parent->getDrawableParent(false, false, false, &wlist);

            /* the widget found can be to far away from this widget */
            /* if wlist is filled, i can search for a better parent which has already a own surface */
            for (int i=0; i < wlist.size(); i++) {
                MMSWidget *w = wlist.at(i);
                if ((w->drawable)&&(w->geomset)&&(w->visible)) {
                    widget = w;
                    break;
                }
            }

            /* clear it (complete transparent) */
            if (this->drawable) {
                /* my own surface */
                this->surface->clear();
            }
            else {
                /* working direct on the window surface */
                MMSFBRegion clip;
                clip.x1 = innerGeom.x;
                clip.y1 = innerGeom.y;
                clip.x2 = innerGeom.x + innerGeom.w - 1;
                clip.y2 = innerGeom.y + innerGeom.h - 1;

#ifdef MMSGUI_STDOUT_TRACE
                DEBUGOUT("-----%u: widget setclip (%x)\n", pthread_self(), this->windowSurface);
#endif

                this->windowSurface->setClip(&clip);
                this->windowSurface->clear();
            }

            if (widget) {
                /* drawable parent found, calculate rectangle to copy */
                MMSFBRectangle srcrect = widget->getVisibleSurfaceArea();
                srcrect.x+= this->innerGeom.x - widget->innerGeom.x;
                srcrect.y+= this->innerGeom.y - widget->innerGeom.y;
                srcrect.w = this->innerGeom.w;
                srcrect.h = this->innerGeom.h;

                if (this->drawable) {
                    /* copy background from parent */
#ifdef __PUPTRACE__
                	DEBUGOUT("copy from widget, w = %s -> w = %s\n", widget->getName().c_str(), this->getName().c_str());
#endif
                	this->surface->setBlittingFlags((MMSFBSurfaceBlittingFlags)(DSBLIT_NOFX));
                    this->surface->blit(widget->surface, &srcrect, 0, 0);
                }
                else {
                    /* this is for example <hbox> or <vbox> which has no own drawing */
                    this->windowSurface->setBlittingFlags((MMSFBSurfaceBlittingFlags)(DSBLIT_NOFX));
                    this->windowSurface->blit(widget->surface, &srcrect, innerGeom.x, innerGeom.y);
                }
            }
            else {
                /* no parent found, use background from window */
                if (this->rootwindow) {
                    MMSFBColor bgcolor;
                    this->rootwindow->getBgColor(bgcolor);
                    if (!this->rootwindow->bgimage) {
                        /* draw background with window bgcolor */
                        if (bgcolor.a) {
                            if (this->drawable) {
                                /* clear surface */
                                this->surface->clear(bgcolor.r, bgcolor.g, bgcolor.b, bgcolor.a);
                            }
                            else {
                                /* this is for example <hbox> or <vbox> which has no own drawing */
                                this->windowSurface->clear(bgcolor.r, bgcolor.g, bgcolor.b, bgcolor.a);
                            }
                        }
                    }
                    else {
                        /* draw background with a part of window bgimage */
                        MMSFBRectangle src, dst;
                        int sw, sh;
#ifdef __PUPTRACE__
                        DEBUGOUT("copy from window, win=%s -> w=%s\n", this->rootwindow->getName().c_str(), this->getName().c_str());
#endif

                        /* get width and height of windows background image */
                        this->rootwindow->bgimage->getSize(&sw, &sh);

                        /* calculate with window width and height */
                        int f1 = (this->rootwindow->innerGeom.w * 10000) / sw;
                        int f2 = (this->rootwindow->innerGeom.h * 10000) / sh;

                        /* calculate the source rectangle */
                        src.x = (5000 + 10000 *(this->innerGeom.x - this->rootwindow->innerGeom.x)) / f1;
                        src.w = (5000 + 10000 * this->innerGeom.w) / f1;
                        src.y = (5000 + 10000 *(this->innerGeom.y - this->rootwindow->innerGeom.y)) / f2;
                        src.h = (5000 + 10000 * this->innerGeom.h) / f2;

                        if (this->drawable) {
                            /* the destination rectangle */
                            dst.x = 0;
                            dst.w = this->innerGeom.w;
                            dst.y = 0;
                            dst.h = this->innerGeom.h;

                            /* copy background from window's bgimage */
                            this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(255, (bgcolor.a)?bgcolor.a:255, 255);
                            this->surface->stretchBlit(this->rootwindow->bgimage, &src, &dst);
                        }
                        else {
                            /* the destination rectangle */
                            dst.x = this->innerGeom.x;
                            dst.w = this->innerGeom.w;
                            dst.y = this->innerGeom.y;
                            dst.h = this->innerGeom.h;

                            /* this is for example <hbox> or <vbox> which has no own drawing */
                            this->windowSurface->setBlittingFlagsByBrightnessAlphaAndOpacity(255, (bgcolor.a)?bgcolor.a:255, 255);
                            this->windowSurface->stretchBlit(this->rootwindow->bgimage, &src, &dst);
                        }
                    }
                }
            }

            if (!this->drawable) {
                /* reset the clip */
                this->windowSurface->setClip(NULL);
            }

            *backgroundFilled = true;
        }
    } while (retry);

    /* unlock */
    if (this->surface) this->surface->unlock();
    this->windowSurface->unlock();

    return true;
}
#endif

bool MMSWidget::draw(bool *backgroundFilled) {
    bool         myBackgroundFilled = false;
    bool         retry = false;


    if (!this->initialized) {
        /* init widget (e.g. load images, fonts, ...) */
        init();
        this->initialized = true;
    }

    if (backgroundFilled) {
    	if (this->has_own_surface)
    		*backgroundFilled = false;
    }
    else
        backgroundFilled = &myBackgroundFilled;

    if ((!this->geomset)||(!this->visible))
        return false;

    /* lock */
    if (this->surface) this->surface->lock();
    this->windowSurface->lock();

    /* draw background */
    do {
        /* searching for the background color or image */
        MMSFBColor col;
        MMSFBSurface *suf = NULL;
        col.a = 0;

        if (this->drawable) {
            if (isActivated()) {
                if (isSelected()) {
                    getSelBgColor(col);
                    suf = this->da->selbgimage;
                }
                else {
                    getBgColor(col);
                    suf = this->da->bgimage;
                }
            	if (isPressed()) {
                    MMSFBColor mycol;
                    if (isSelected()) {
                        getSelBgColor_p(mycol);
                        if (mycol.a>0) col=mycol;
                        if (this->da->selbgimage_p)
                        	suf = this->da->selbgimage_p;
                    }
                    else {
                        getBgColor_p(mycol);
                        if (mycol.a>0) col=mycol;
                        if (this->da->bgimage_p)
                        	suf = this->da->bgimage_p;
                    }
            	}
            }
            else {
                if (isSelected()) {
                    getSelBgColor_i(col);
                    suf = this->da->selbgimage_i;
                }
                else {
                    getBgColor_i(col);
                    suf = this->da->bgimage_i;
                }
            }
        }

        if (suf) {
            if ((*backgroundFilled)||(retry)||(!this->has_own_surface)) {
                /* prepare for blitting */
                this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(brightness, (col.a)?col.a:255, opacity);

                /* fill background */
                surface->stretchBlit(suf, NULL, &surfaceGeom);
                *backgroundFilled = true;

                /* go out of the loop */
                break;
            }
            else
                /* the color has an alpha value and I need a background before drawing the image */
                retry = true;
        }
        else
        if (col.a) {
            if ((*backgroundFilled)||(retry)||(!this->has_own_surface)||((col.a==255)&&(opacity==255))) {
                /* prepare for drawing */
                this->surface->setDrawingColorAndFlagsByBrightnessAndOpacity(col, brightness, opacity);

                /* fill background */
                this->surface->fillRectangle(surfaceGeom.x, surfaceGeom.y, surfaceGeom.w, surfaceGeom.h);
                *backgroundFilled = true;

                /* go out of the loop */
                break;
            }
            else
                /* the color has an alpha value and I need a background for it */
                retry = true;
        }
        else {
            if ((*backgroundFilled)||(!this->has_own_surface)) {
                /* go out of the loop */
            	if (!*backgroundFilled) {
            		if (this->surface) {
            			/* have no background, clear it */
            			/* this is in case of if I have no own surface */
            			this->surface->clear();
            			*backgroundFilled = true;
            		}
            	}
            	break;}
            else
                /* no color, no image, I need to search for a background */
                retry = true;
        }

        /* if not filled then */
        if (!*backgroundFilled) {
        	/* searching for the next parent widget */
            MMSWidget *widget = NULL;
            vector<MMSWidget*> wlist;
            if (this->parent)
                widget = this->parent->getDrawableParent(false, false, false, &wlist);

            /* the widget found can be to far away from this widget */
            /* if wlist is filled, i can search for a better parent which has already a own surface */
            for (unsigned int i=0; i < wlist.size(); i++) {
                MMSWidget *w = wlist.at(i);
                if ((w->drawable)&&(w->geomset)&&(w->visible)) {
                    widget = w;
                    break;
                }
            }

            /* clear it (complete transparent) */
            if (this->drawable) {
                /* my own surface */
                this->surface->clear();
            }
            else {
                /* working direct on the window surface */
                MMSFBRegion clip;
                clip.x1 = innerGeom.x;
                clip.y1 = innerGeom.y;
                clip.x2 = innerGeom.x + innerGeom.w - 1;
                clip.y2 = innerGeom.y + innerGeom.h - 1;

                this->windowSurface->setClip(&clip);
                this->windowSurface->clear();
            }

            if (widget) {
                /* drawable parent found, calculate rectangle to copy */
                MMSFBRectangle srcrect = widget->getVisibleSurfaceArea();
                srcrect.x+= this->innerGeom.x - widget->innerGeom.x;
                srcrect.y+= this->innerGeom.y - widget->innerGeom.y;
                srcrect.w = this->innerGeom.w;
                srcrect.h = this->innerGeom.h;

                if (this->drawable) {
                    /* copy background from parent */
                	this->surface->setBlittingFlags(MMSFB_BLIT_NOFX);
                    this->surface->blit(widget->surface, &srcrect, 0, 0);
                }
                else {
                    /* this is for example <hbox> or <vbox> which has no own drawing */
                    this->windowSurface->setBlittingFlags(MMSFB_BLIT_NOFX);
                    this->windowSurface->blit(widget->surface, &srcrect, innerGeom.x, innerGeom.y);
                }
            }
            else {
                /* no parent found, use background from window */
                if (this->rootwindow) {
                    MMSFBColor bgcolor;
                    this->rootwindow->getBgColor(bgcolor);
                    if (!this->rootwindow->bgimage) {
                        /* draw background with window bgcolor */
                        if (bgcolor.a) {
                            if (this->drawable) {
                                /* clear surface */
                                this->surface->clear(bgcolor.r, bgcolor.g, bgcolor.b, bgcolor.a);
                            }
                            else {
                                /* this is for example <hbox> or <vbox> which has no own drawing */
                                this->windowSurface->clear(bgcolor.r, bgcolor.g, bgcolor.b, bgcolor.a);
                            }
                        }
                    }
                    else {
                        /* draw background with a part of window bgimage */
                        MMSFBRectangle src, dst;
                        int sw, sh;

                        /* get width and height of windows background image */
                        this->rootwindow->bgimage->getSize(&sw, &sh);

                        /* calculate with window width and height */
                        int f1 = (this->rootwindow->innerGeom.w * 10000) / sw;
                        int f2 = (this->rootwindow->innerGeom.h * 10000) / sh;

                        /* calculate the source rectangle */
                        src.x = (5000 + 10000 *(this->innerGeom.x - this->rootwindow->innerGeom.x)) / f1;
                        src.w = (5000 + 10000 * this->innerGeom.w) / f1;
                        src.y = (5000 + 10000 *(this->innerGeom.y - this->rootwindow->innerGeom.y)) / f2;
                        src.h = (5000 + 10000 * this->innerGeom.h) / f2;

                        if (this->drawable) {
                            /* the destination rectangle */
                            dst.x = 0;
                            dst.w = this->innerGeom.w;
                            dst.y = 0;
                            dst.h = this->innerGeom.h;

                            /* copy background from window's bgimage */
                            this->surface->setBlittingFlagsByBrightnessAlphaAndOpacity(255, (bgcolor.a)?bgcolor.a:255, 255);
                            this->surface->stretchBlit(this->rootwindow->bgimage, &src, &dst);
                        }
                        else {
                            /* the destination rectangle */
                            dst.x = this->innerGeom.x;
                            dst.w = this->innerGeom.w;
                            dst.y = this->innerGeom.y;
                            dst.h = this->innerGeom.h;

                            /* this is for example <hbox> or <vbox> which has no own drawing */
                            this->windowSurface->setBlittingFlagsByBrightnessAlphaAndOpacity(255, (bgcolor.a)?bgcolor.a:255, 255);
                            this->windowSurface->stretchBlit(this->rootwindow->bgimage, &src, &dst);
                        }
                    }
                }
            }

            if (!this->drawable) {
                /* reset the clip */
                this->windowSurface->setClip(NULL);
            }

            *backgroundFilled = true;
        }
    } while (retry);

    /* unlock */
    if (this->surface) this->surface->unlock();
    this->windowSurface->unlock();

    return true;
}



void MMSWidget::drawMyBorder() {

    if ((!this->drawable)||(!this->geomset)||(!this->visible))
        return;

    unsigned int margin;
    if (!getMargin(margin))
    	margin = 0;

    MMSFBRectangle mygeom;
    mygeom = this->geom;
    mygeom.x+= margin;
    mygeom.y+= margin;
    mygeom.w-= 2*margin;
    mygeom.h-= 2*margin;

    unsigned int borderthickness;
    if (!getBorderThickness(borderthickness))
    	borderthickness = 0;

    bool borderrcorners;
    if (!getBorderRCorners(borderrcorners))
    	borderrcorners = false;

    if (isSelected()) {
        MMSFBColor c;
        getBorderSelColor(c);
        drawBorder(borderthickness, borderrcorners, this->da->borderselimages,
                   this->da->borderselgeom, &(this->da->borderselgeomset), this->windowSurface,
                   mygeom.x, mygeom.y, mygeom.w, mygeom.h, c, this->rootwindow->im,
                   getBrightness(), getOpacity());
    }
    else {
        MMSFBColor c;
        getBorderColor(c);
        drawBorder(borderthickness, borderrcorners, this->da->borderimages,
                   this->da->bordergeom, &(this->da->bordergeomset), this->windowSurface,
                   mygeom.x, mygeom.y, mygeom.w, mygeom.h, c, this->rootwindow->im,
                   getBrightness(), getOpacity());
    }
}



bool MMSWidget::drawDebug() {
    if ((!this->geomset)||(!this->visible))
        return false;

    bool debug;
    if (!this->getRootWindow()->getDebug(debug)) debug = false;
    if (debug) {
        this->surface->setDrawingFlagsByAlpha(255);
        this->windowSurface->setColor(255, 255, 255, 255);
        this->windowSurface->drawRectangle(this->geom.x, this->geom.y, this->geom.w, this->geom.h);
        if (this->innerGeom.x != this->geom.x) {
            this->windowSurface->setColor(200, 200, 200, 255);
            this->windowSurface->drawRectangle(this->innerGeom.x, this->innerGeom.y,
                                               this->innerGeom.w, this->innerGeom.h);
        }
    }

    if (this->drawable) {
		if (this->da->initialArrowsDrawn == false) {
			switchArrowWidgets();
			this->da->initialArrowsDrawn = true;
		}
    }

    return true;
}

void MMSWidget::drawchildren(bool toRedrawOnly, bool *backgroundFilled) {

    if ((toRedrawOnly) && (this->toRedraw==false) && (this->redrawChildren==false))
        return;

    if (!this->visible)
        return;

    bool myBackgroundFilled = false;
    if (!backgroundFilled)
        backgroundFilled = &myBackgroundFilled;

    if ((!toRedrawOnly)||(this->toRedraw))
    	this->draw(backgroundFilled);

    if ((!toRedrawOnly)||(this->toRedraw)||(this->redrawChildren)) {
	    for(unsigned int i=0;i<this->children.size();i++)
	        this->children.at(i)->drawchildren(toRedrawOnly, backgroundFilled);

    	drawMyBorder();
    }

    this->toRedraw = this->redrawChildren = false;

}

void MMSWidget::startWidgetThread(int delay) {
	if (this->da) {
		if (!this->da->widgetthread)
			this->da->widgetthread = new MMSWidgetThread(this);
		if (this->da->widgetthread)
			this->da->widgetthread->start(delay);
	}
}


void MMSWidget::add(MMSWidget *widget) {
    if (canHaveChildren())
        if(this->children.size() < 1) {

            this->children.push_back(widget);

            children.at(0)->setParent(this);
            if (this->rootwindow)
                this->rootwindow->add(widget);
        }
        else
            throw new MMSWidgetError(0,"this widget does only support one child");
    else
        throw new MMSWidgetError(0,"this widget does not support children");
}

void MMSWidget::markChildren2Redraw() {
	this->toRedraw = true;
	this->redrawChildren = true;
    for(unsigned int i=0;i<this->children.size();i++) {
        if (this->children.at(i)->isVisible())
            this->children.at(i)->markChildren2Redraw();
    }
}

MMSWidget *MMSWidget::getDrawableParent(bool mark2Redraw, bool markChildren2Redraw, bool checkborder,
                                        vector<MMSWidget*> *wlist, bool followpath) {
    if (mark2Redraw) {
        this->toRedraw = true;

		if (markChildren2Redraw) {
		    this->redrawChildren = true;

		    for(unsigned int i=0;i<this->children.size();i++)
		        if (this->children.at(i)->isVisible())
		            this->children.at(i)->markChildren2Redraw();
		}
    }

    if (followpath)
    	this->redrawChildren = true;

    if (this->needsParentDraw(checkborder)==false) {
        if (wlist) wlist->push_back(this->parent);
        return this->parent->getDrawableParent(false, false, checkborder, wlist, true);
    }
    else
    if (this->parent) {
        if (wlist) wlist->push_back(this->parent);
        return this->parent->getDrawableParent(mark2Redraw, false, checkborder, wlist, followpath);
    }

    return NULL;
}

void MMSWidget::refresh() {
    MMSFBRectangle tobeupdated;
    unsigned int margin = 0;
    MMSWindow *myroot = this->rootwindow;

    if (!this->geomset) {
    	return;
    }

    /*if (!this->visible) {
    	return;
    }*/

    if (!myroot)
   		return;

    // it makes sense that we skip all drawing requests here, if this window OR one of its parents are not shown
    if (!myroot->isShown(true)) {
    	DEBUGMSG("MMSGUI", "MMSWidget->refresh() skipped because window is not shown");
        return;
    }

    /* lock the window because only one thread can do this at the same time */
//    myroot->lock();
    this->parent_rootwindow->lock();

    if (this->drawable)
        getMargin(margin);

    tobeupdated.x = this->geom.x + margin;
    tobeupdated.y = this->geom.y + margin;
    tobeupdated.w = this->geom.w - 2*margin;
    tobeupdated.h = this->geom.h - 2*margin;


    /* e.g. for smooth scrolling menus we must recalculate children here */
    /* so if the widget is a menu and smooth scrolling is enabled, we do the recalculation */
    if (this->type == MMSWIDGETTYPE_MENU)
    	if (((MMSMenuWidget *)this)->getSmoothScrolling())
    		recalculateChildren();

   	myroot->refreshFromChild(this->getDrawableParent(true, true), &tobeupdated, false);

    switchArrowWidgets();

    /* unlock the window */
//    myroot->unlock();
    this->parent_rootwindow->unlock();
}

bool MMSWidget::isDrawable() {
    return this->drawable;
}

bool MMSWidget::needsParentDraw(bool checkborder) {

	//NEW: return true in any case, because we cannot decide it
	return true;


	//OLD code, has to be deleted...

	MMSFBColor c;

	if (this->needsparentdraw)
        return true;

    if (checkborder) {
        unsigned int borderthickness;
        if (!getBorderThickness(borderthickness))
        	borderthickness = 0;

        if (borderthickness>0) {
            bool borderrcorners;
            if (!getBorderRCorners(borderrcorners))
            	borderrcorners = false;

            if ((borderrcorners)||(getOpacity()!=255))
                return true;
            else
            if (this->selected) {
                MMSFBColor c;
                getBorderSelColor(c);
                if (c.a!=255)
                    return true;
            }
            else {
                MMSFBColor c;
                getBorderColor(c);
                if (c.a!=255)
                    return true;
            }
        }
    }

    if (this->activated) {
        if (!this->pressed) {
	        if (this->selected) {
	            getSelBgColor(c);
	            if (c.a==255)
	                return false;
	        }
	        else {
	        	getBgColor(c);
	            if (c.a==255)
	                return false;
	        }
        }
        else {
            if (this->selected) {
            	getSelBgColor_p(c);
                if (c.a==255)
                    return false;
            }
            else {
            	getBgColor_p(c);
                if (c.a==255)
                    return false;
            }
        }
    }
    else {
        if (this->selected) {
        	getSelBgColor_i(c);
            if (c.a==255)
                return false;
        }
        else {
        	getBgColor_i(c);
            if (c.a==255)
                return false;
        }
    }

    return true;
}


 bool MMSWidget::canHaveChildren() {
    return this->canhavechildren;
}

bool MMSWidget::canSelectChildren() {
    return this->canselectchildren;
}


void MMSWidget::setParent(MMSWidget *parent) {
    this->parent = parent;
    for (unsigned int i=0; i < children.size(); i++)
        children.at(i)->setParent(this);
}

MMSWidget *MMSWidget::getParent() {
    return this->parent;
}

void MMSWidget::setRootWindow(MMSWindow *root, MMSWindow *parentroot) {
	// set window on which the widget is connected
    this->rootwindow = root;

    // set the toplevel parent window
    this->parent_rootwindow = parentroot;

    if (this->rootwindow) {
    	// searching the right toplevel parent
        if (!this->parent_rootwindow) {
        	if (!this->rootwindow->parent)
        		this->parent_rootwindow = this->rootwindow;
        	else
        		this->parent_rootwindow = this->rootwindow->getParent(true);
        }

        this->windowSurface = this->rootwindow->getSurface();
        rootwindow->add(this);
    }

    // set root window to all children
    for (unsigned int i=0; i < children.size(); i++)
        children.at(i)->setRootWindow(this->rootwindow, this->parent_rootwindow);
}

void MMSWidget::recalculateChildren() {

    if(this->children.size() == 1) {
        children.at(0)->setGeometry(innerGeom);
    }

}

MMSWindow *MMSWidget::getRootWindow(MMSWindow **parentroot) {
	if (parentroot)
		*parentroot = this->parent_rootwindow;
    return this->rootwindow;
}

int MMSWidget::getId() {
    return this->id;
}

string MMSWidget::getName() {
    return this->name;
}

void MMSWidget::setName(string name) {
    this->name = name;
}

void MMSWidget::setFocus(bool set, bool refresh, MMSInputEvent *inputevent) {
    /* switch focused on/off if possible */
	bool b;
    if (!getFocusable(b))
        return;
    if (!b)
    	return;

    if (this->rootwindow)
        this->rootwindow->setFocusedWidget(this, set);

    /* the focused flag MUST BE set before all other calls (because of dim and trans functions) */
    this->focused = set;

    setSelected(set, refresh);


//    if (this->rootwindow)
//        this->rootwindow->setFocusedWidget(this, set);

    this->onFocus->emit(this, set);

    // check if we have to navigate
	if (inputevent) {
		bool scrollonfocus;
		if (getScrollOnFocus(scrollonfocus)) {
			if (scrollonfocus) {
				if (inputevent->type == MMSINPUTEVENTTYPE_KEYPRESS) {
					switch (inputevent->key) {
						case MMSKEY_CURSOR_DOWN:
							scrollDown();
						    break;
						case MMSKEY_CURSOR_UP:
							scrollUp();
						    break;
						case MMSKEY_CURSOR_RIGHT:
							scrollRight();
						    break;
						case MMSKEY_CURSOR_LEFT:
							scrollLeft();
					        break;
						default:
							break;
					}
				}
			}
		}
	}
}

bool MMSWidget::isFocused() {
    return this->focused;
}

void MMSWidget::getJoinedWigdets(MMSWidget **caller_stack) {
	if ((this->da)&&(this->da->joinedWidget)) {
		int i=0;
		while ((caller_stack[i]) && (caller_stack[i] != this) && (i < 16)) i++;
		if (!caller_stack[i]) {
			caller_stack[i] = this;
			this->da->joinedWidget->getJoinedWigdets(caller_stack);
		}
	}
}

bool MMSWidget::setSelected(bool set, bool refresh, bool *changed, bool joined) {
	if (changed)
		*changed = false;

	if (!joined) {
		if ((this->da)&&(this->da->joinedWidget)) {
			MMSWidget *caller_stack[16] = {0};
			caller_stack[0] = this;
			this->da->joinedWidget->getJoinedWigdets(caller_stack);
			int i = 16;
			while (i-- > 1) {
				if (caller_stack[i])
					caller_stack[i]->setSelected(set, refresh, NULL, true);
			}
		}
	}

    /* check if selected status already set */
    if (this->selected == set) {
        /* refresh my children */
    	if (canSelectChildren()) {
        	bool rf = false;
            for (unsigned int i=0; i < children.size(); i++)
                if (children.at(i)->setSelected(set, false))
                	rf = true;

            if (refresh && rf)
            	this->refresh();
        }
        return false;
    }

    /* get flags */
    bool selectable;
    if (!getSelectable(selectable))
    	selectable = false;
    bool canselchildren = canSelectChildren();

    /* switch selected on/off if possible */
    if (selectable) {
        this->selected=set;
    	if (changed) *changed = true;
    }

    /* refresh my children */
    if (canselchildren) {
        for (unsigned int i=0; i < children.size(); i++)
            children.at(i)->setSelected(set, false);
    }

    /* refresh widget */
    if (((selectable)||(canselchildren))&&(refresh))
    	this->refresh();

    /* emit select signal */
    if (selectable)
        if (set)
            this->onSelect->emit(this);

    if ((set)&&(refresh)) {
		// start thread which removes the focus after n seconds
		string inputmode = "";
		getInputModeEx(inputmode);
		if (strToUpr(inputmode) == "CLICK")
			startWidgetThread(150);
    }

    return true;
}

bool MMSWidget::isSelected() {
    return this->selected;
}

void MMSWidget::unsetFocusableForAllChildren(bool refresh) {
    for (unsigned int i = 0; i < children.size(); i++) {
        children.at(i)->setFocusable(false, refresh);
        children.at(i)->unsetFocusableForAllChildren(refresh);
    }
}

void MMSWidget::setActivated(bool set, bool refresh) {

    /* check if activated status already set */
    if (this->activated == set) {
        /* refresh my children */
        for (unsigned int i=0; i < children.size(); i++)
            children.at(i)->setActivated(set, false);
        if (refresh)
            this->refresh();
        return;
    }

    /* switch activated on/off */
    this->activated = set;

    /* refresh my children */
    for (unsigned int i=0; i < children.size(); i++)
        children.at(i)->setActivated(set, false);

    /* refresh widget */
    if (refresh)
        this->refresh();
}

bool MMSWidget::isActivated() {
    return this->activated;
}

void MMSWidget::setPressed(bool set, bool refresh) {

    /* check if pressed status already set */
    if (this->pressed == set) {
        /* refresh my children */
    	if (canSelectChildren()) {
			for (unsigned int i=0; i < children.size(); i++)
				children.at(i)->setPressed(set, false);
			if (refresh)
				this->refresh();
    	}
		return;
    }

    /* switch pressed on/off */
    this->pressed = set;

    /* refresh my children */
	if (canSelectChildren()) {
		for (unsigned int i=0; i < children.size(); i++)
			children.at(i)->setPressed(set, false);
	}

    /* refresh widget */
    if (refresh)
        this->refresh();
}

bool MMSWidget::isPressed() {
    return this->pressed;
}

void MMSWidget::setASelected(bool set, bool refresh) {
    setActivated(true, false);
    setSelected(set, refresh);
}

void MMSWidget::setPSelected(bool set, bool refresh) {
    setPressed(true, false);
    setSelected(set, refresh);
}

void MMSWidget::setISelected(bool set, bool refresh) {
    setActivated(false, false);
    setSelected(set, refresh);
}

#ifdef sdsds
void MMSWidget::handleNavigation(DFBInputDeviceKeySymbol key, MMSWidget *requestingchild) {
    /* give the navigation to my parent */
    this->parent->handleNavigation(key,this);
}
#endif

void MMSWidget::handleInput(MMSInputEvent *inputevent) {
	bool b;

	if (inputevent->type == MMSINPUTEVENTTYPE_KEYPRESS) {
		/* keyboard inputs */

		/* save last inputevent */
		this->da->last_inputevent = *inputevent;

		switch (inputevent->key) {
			case MMSKEY_CURSOR_DOWN:
/*PERFORMANCE TEST
				for (int ii=0; ii< 15;ii++) scrollDown();
				for (int ii=0; ii< 15;ii++) scrollUp();
				for (int ii=0; ii< 15;ii++) scrollDown();
				for (int ii=0; ii< 15;ii++) scrollUp();
				for (int ii=0; ii< 15;ii++) scrollDown();
				for (int ii=0; ii< 15;ii++) scrollUp();
				for (int ii=0; ii< 15;ii++) scrollDown();
*/

				if (scrollDown())
		            return;
		        break;
			case MMSKEY_CURSOR_UP:
		        if (scrollUp())
		            return;
		        break;
			case MMSKEY_CURSOR_RIGHT:
		        if (scrollRight())
		            return;
		        break;
			case MMSKEY_CURSOR_LEFT:
		        if (scrollLeft())
		            return;
		        break;
			case MMSKEY_RETURN:
			case MMSKEY_ZOOM:
		        if (getFocusable(b))
		        	if (b) {
		        		if (callOnReturn()) this->onReturn->emit(this);
			            return;
			        }
		        break;
                default:
                        break;
		}
	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_BUTTONPRESS) {
		/* button pressed */
		if (getClickable(b))
			if (b) {
				// save last inputevent and rectangle
				this->da->last_inputevent = *inputevent;
				this->da->pressed_inputrect = this->geom;

				/* set the pressed status */
				if (!isPressed())
					setPressed(true);

				// scroll to the position if possible and set the pressed status
				// the widget can return a specific input rectangle (e.g. item rectangle in a menu widget)
				scrollTo(inputevent->posx, inputevent->posy, true, NULL,
						 MMSWIDGET_SCROLL_MODE_SETPRESSED, &this->da->pressed_inputrect);

				return;
	        }
	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_BUTTONRELEASE) {
		// button released
        if (getClickable(b))
        	if (b) {
	    		if (this->da->last_inputevent.type == MMSINPUTEVENTTYPE_BUTTONPRESS) {

	    			// reset the pressed status
    				if (isPressed())
    					setPressed(false);

					// check if the pointer is within widget
					if   ((inputevent->posx >= this->da->pressed_inputrect.x)&&(inputevent->posy >= this->da->pressed_inputrect.y)
						&&(inputevent->posx < this->da->pressed_inputrect.x + this->da->pressed_inputrect.w)&&(inputevent->posy < this->da->pressed_inputrect.y + this->da->pressed_inputrect.h)) {
						// yes, scroll to the position if possible
						bool changed = false;
						bool st_ok = scrollTo(this->da->last_inputevent.posx, this->da->last_inputevent.posy, true, &changed,
											  MMSWIDGET_SCROLL_MODE_SETSELECTED | MMSWIDGET_SCROLL_MODE_RMPRESSED);

						// fire the onclick callback
						this->onClick->emit(this, inputevent->posx - this->geom.x, inputevent->posy - this->geom.y,
											this->geom.w, this->geom.h);

						// start thread which removes the focus after n seconds
/*						string inputmode = "";
						getInputModeEx(inputmode);
						if (strToUpr(inputmode) == "CLICK")
							startWidgetThread(500);*/

						if (changed) {
							// check if have to emit onReturn
							bool r;
							if (!getReturnOnScroll(r)) r = true;
							if (r) changed = false;
						}
						if (!changed && st_ok) {
		    		        if (getFocusable(b, false))
		    		        	if (b) {
									// emit the onReturn
									if (callOnReturn()) this->onReturn->emit(this);
		    		        	}
						}
					}
	    		}

	    		// save last inputevent
	    		this->da->last_inputevent = *inputevent;
	    		return;
			}
	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_AXISMOTION) {
		/* axis motion */
        if (getClickable(b))
        	if (b) {
	    		if (this->da->last_inputevent.type == MMSINPUTEVENTTYPE_BUTTONPRESS) {
					// check if the pointer is within widget
	    			if   ((inputevent->posx >= this->da->pressed_inputrect.x)&&(inputevent->posy >= this->da->pressed_inputrect.y)
	    				&&(inputevent->posx < this->da->pressed_inputrect.x + this->da->pressed_inputrect.w)&&(inputevent->posy < this->da->pressed_inputrect.y + this->da->pressed_inputrect.h)) {
	    				// yes, set the pressed status
	    				if (!isPressed())
	    					setPressed(true);

						// yes, scroll to the position if possible and set the pressed status
						scrollTo(this->da->last_inputevent.posx, this->da->last_inputevent.posy, true, NULL,
								 MMSWIDGET_SCROLL_MODE_SETPRESSED);
	    			}
	    			else {
		    			// no, reset the pressed status
	    				if (isPressed())
	    					setPressed(false);

						// no, scroll to the position if possible and remove the pressed status
						scrollTo(this->da->last_inputevent.posx, this->da->last_inputevent.posy, true, NULL,
								 MMSWIDGET_SCROLL_MODE_RMPRESSED);
	    			}
	    		}

	    		return;
			}
	}

    throw new MMSWidgetError(1,"input not handled");
}


/*void MMSWidget::registerInput(DFBInputDeviceKeySymbol key, GUIINPUTCALLBACK cb) {
    INPUT_CB *input = new INPUT_CB;

    input->key = key;
    input->cb = cb;

    this->inputs.push_back(input);
}*/

void MMSWidget::setBinData(void *data) {
	this->bindata = data;
}

void *MMSWidget::getBinData() {
	return this->bindata;

}

string MMSWidget::getSizeHint() {
    return this->sizehint;
}

bool MMSWidget::setSizeHint(string &hint) {
    if (getPixelFromSizeHint(NULL, hint, 10000, 0)) {
        this->sizehint = hint;
        return true;
    }
    else
        return false;
}

bool MMSWidget::isGeomSet() {
    return this->geomset;
}

void MMSWidget::setGeomSet(bool set) {
    this->geomset = set;
}


bool MMSWidget::isVisible() {
    return this->visible;
}

void MMSWidget::setVisible(bool visible, bool refresh) {

    if (this->geomset) {
        if (visible) {
            if (!this->visible) {
                if ((!this->surface)&&(surfaceGeom.w!=0)&&(surfaceGeom.h!=0)) {
                    unsigned int w,h;
                    w=surfaceGeom.w;
                    h=surfaceGeom.h;
                    surfaceGeom.w=0;
                    surfaceGeom.h=0;
                    setSurfaceGeometry(w,h);
                }
            }
        }
        else {
            if (this->visible) {
                if (this->surface) {
                    delete this->surface;
                    this->surface = NULL;
                }
            }
        }
    }


//TODO: bgimages handling like MMSImage!!!



    this->visible = visible;
    for (unsigned int i = 0; i < children.size(); i++)
        children.at(i)->setVisible(this->visible, false);
    if (refresh)
        this->refresh();
}

unsigned char MMSWidget::getBrightness() {
    return this->brightness;
}

void MMSWidget::setBrightness(unsigned char brightness, bool refresh) {
    this->brightness = brightness;
    for (unsigned int i = 0; i < children.size(); i++)
        children.at(i)->setBrightness(this->brightness, false);
    if (refresh)
        this->refresh();
}

unsigned char MMSWidget::getOpacity() {
    return this->opacity;
}

void MMSWidget::setOpacity(unsigned char opacity, bool refresh) {
    this->opacity = opacity;
    for (unsigned int i = 0; i < children.size(); i++)
        children.at(i)->setOpacity(this->opacity, false);
    if (refresh)
        this->refresh();
}


MMSWidget *MMSWidget::getNavigateUpWidget() {
    return this->da->navigateUpWidget;
}

MMSWidget *MMSWidget::getNavigateDownWidget() {
    return this->da->navigateDownWidget;
}

MMSWidget *MMSWidget::getNavigateLeftWidget() {
    return this->da->navigateLeftWidget;
}

MMSWidget *MMSWidget::getNavigateRightWidget() {
    return this->da->navigateRightWidget;
}

void MMSWidget::setNavigateUpWidget(MMSWidget *upwidget) {
	this->da->navigateUpWidget = upwidget;
}

void MMSWidget::setNavigateDownWidget(MMSWidget *downwidget) {
	this->da->navigateDownWidget = downwidget;
}

void MMSWidget::setNavigateRightWidget(MMSWidget *rightwidget) {
	this->da->navigateRightWidget = rightwidget;
}

void MMSWidget::setNavigateLeftWidget(MMSWidget *leftwidget) {
	this->da->navigateLeftWidget = leftwidget;
}

bool MMSWidget::canNavigateUp() {
    if (this->da->navigateUpWidget)
        return true;
    else
        return scrollUp(1, false, true);
}

bool MMSWidget::canNavigateDown() {
    if (this->da->navigateDownWidget)
        return true;
    else
        return scrollDown(1, false, true);
}

bool MMSWidget::canNavigateLeft() {
	if (this->da->navigateLeftWidget)
        return true;
    else
        return scrollLeft(1, false, true);
}

bool MMSWidget::canNavigateRight() {
    if (this->da->navigateRightWidget)
        return true;
    else
        return scrollRight(1, false, true);
}

/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETWIDGET(x,y) \
	if (!this->da) return false; \
	else if (this->da->myWidgetClass.is##x()) return this->da->myWidgetClass.get##x(y); \
    else if ((this->da->widgetClass)&&(this->da->widgetClass->is##x())) return this->da->widgetClass->get##x(y); \
    else if (this->da->baseWidgetClass) return this->da->baseWidgetClass->get##x(y); \
    else return this->da->myWidgetClass.get##x(y);


bool MMSWidget::getBgColor(MMSFBColor &bgcolor) {
    GETWIDGET(BgColor, bgcolor);
}

bool MMSWidget::getSelBgColor(MMSFBColor &selbgcolor) {
    GETWIDGET(SelBgColor, selbgcolor);
}

bool MMSWidget::getBgColor_p(MMSFBColor &bgcolor_p) {
    GETWIDGET(BgColor_p, bgcolor_p);
}

bool MMSWidget::getSelBgColor_p(MMSFBColor &selbgcolor_p) {
    GETWIDGET(SelBgColor_p, selbgcolor_p);
}

bool MMSWidget::getBgColor_i(MMSFBColor &bgcolor_i) {
    GETWIDGET(BgColor_i, bgcolor_i);
}

bool MMSWidget::getSelBgColor_i(MMSFBColor &selbgcolor_i) {
    GETWIDGET(SelBgColor_i, selbgcolor_i);
}

bool MMSWidget::getBgImagePath(string &bgimagepath) {
    GETWIDGET(BgImagePath,bgimagepath);
}

bool MMSWidget::getBgImageName(string &bgimagename) {
    GETWIDGET(BgImageName,bgimagename);
}

bool MMSWidget::getSelBgImagePath(string &selbgimagepath) {
    GETWIDGET(SelBgImagePath, selbgimagepath);
}

bool MMSWidget::getSelBgImageName(string &selbgimagename) {
    GETWIDGET(SelBgImageName, selbgimagename);
}

bool MMSWidget::getBgImagePath_p(string &bgimagepath_p) {
    GETWIDGET(BgImagePath_p, bgimagepath_p);
}

bool MMSWidget::getBgImageName_p(string &bgimagename_p) {
    GETWIDGET(BgImageName_p, bgimagename_p);
}

bool MMSWidget::getSelBgImagePath_p(string &selbgimagepath_p) {
    GETWIDGET(SelBgImagePath_p, selbgimagepath_p);
}

bool MMSWidget::getSelBgImageName_p(string &selbgimagename_p) {
    GETWIDGET(SelBgImageName_p, selbgimagename_p);
}

bool MMSWidget::getBgImagePath_i(string &bgimagepath_i) {
    GETWIDGET(BgImagePath_i, bgimagepath_i);
}

bool MMSWidget::getBgImageName_i(string &bgimagename_i) {
    GETWIDGET(BgImageName_i, bgimagename_i);
}

bool MMSWidget::getSelBgImagePath_i(string &selbgimagepath_i) {
    GETWIDGET(SelBgImagePath_i, selbgimagepath_i);
}

bool MMSWidget::getSelBgImageName_i(string &selbgimagename_i) {
    GETWIDGET(SelBgImageName_i, selbgimagename_i);
}

bool MMSWidget::getMargin(unsigned int &margin) {
    GETWIDGET(Margin, margin);
}

bool MMSWidget::getFocusable(bool &focusable, bool check_selectable) {
	if (check_selectable) {
		if (getSelectable(focusable)) {
		    if (focusable) {
		    	GETWIDGET(Focusable, focusable);
		    }
		}
		else {
	    	GETWIDGET(Focusable, focusable);
		}
	}
	else {
    	GETWIDGET(Focusable, focusable);
	}

	return false;
}

bool MMSWidget::getSelectable(bool &selectable) {
    GETWIDGET(Selectable, selectable);
}

bool MMSWidget::getUpArrow(string &uparrow) {
    GETWIDGET(UpArrow, uparrow);
}

bool MMSWidget::getDownArrow(string &downarrow) {
    GETWIDGET(DownArrow, downarrow);
}

bool MMSWidget::getLeftArrow(string &leftarrow) {
    GETWIDGET(LeftArrow, leftarrow);
}

bool MMSWidget::getRightArrow(string &rightarrow) {
    GETWIDGET(RightArrow, rightarrow);
}

bool MMSWidget::getData(string &data) {
    GETWIDGET(Data, data);
}

bool MMSWidget::getNavigateUp(string &navigateup) {
    GETWIDGET(NavigateUp, navigateup);
}

bool MMSWidget::getNavigateDown(string &navigatedown) {
    GETWIDGET(NavigateDown, navigatedown);
}

bool MMSWidget::getNavigateLeft(string &navigateleft) {
    GETWIDGET(NavigateLeft, navigateleft);
}

bool MMSWidget::getNavigateRight(string &navigateright) {
    GETWIDGET(NavigateRight, navigateright);
}


bool MMSWidget::getVSlider(string &vslider) {
    GETWIDGET(VSlider, vslider);
}

bool MMSWidget::getHSlider(string &hslider) {
    GETWIDGET(HSlider, hslider);
}

bool MMSWidget::getImagesOnDemand(bool &imagesondemand) {
    GETWIDGET(ImagesOnDemand, imagesondemand);
}

bool MMSWidget::getBlend(unsigned int &blend) {
    GETWIDGET(Blend, blend);
}

bool MMSWidget::getBlendFactor(double &blendfactor) {
    GETWIDGET(BlendFactor, blendfactor);
}

bool MMSWidget::getScrollOnFocus(bool &scrollonfocus) {
    GETWIDGET(ScrollOnFocus, scrollonfocus);
}

bool MMSWidget::getClickable(bool &clickable) {
    GETWIDGET(Clickable, clickable);
}

bool MMSWidget::getReturnOnScroll(bool &returnonscroll) {
    GETWIDGET(ReturnOnScroll, returnonscroll);
}

bool MMSWidget::getInputMode(string &inputmode) {
    GETWIDGET(InputMode, inputmode);
}

bool MMSWidget::getInputModeEx(string &inputmode) {
	getInputMode(inputmode);
	if (inputmode == "") {
		if (this->parent)
			return this->parent->getInputModeEx(inputmode);
		else
			inputmode = MMSWidget_inputmode;
	}
	return true;
}

bool MMSWidget::getJoinedWidget(string &joinedwidget) {
    GETWIDGET(JoinedWidget, joinedwidget);
}



#define GETBORDER(x,y) \
	if (!this->da) return false; \
	else if (this->da->myWidgetClass.border.is##x()) return this->da->myWidgetClass.border.get##x(y); \
    else if ((this->da->widgetClass)&&(this->da->widgetClass->border.is##x())) return this->da->widgetClass->border.get##x(y); \
    else return this->da->baseWidgetClass->border.get##x(y);

#define GETBORDER_IMAGES(x,p,y) \
	if (!this->da) return false; \
	else if (this->da->myWidgetClass.border.is##x()) return this->da->myWidgetClass.border.get##x(p,y); \
    else if ((this->da->widgetClass)&&(this->da->widgetClass->border.is##x())) return this->da->widgetClass->border.get##x(p,y); \
    else return this->da->baseWidgetClass->border.get##x(p,y);



bool MMSWidget::getBorderColor(MMSFBColor &color) {
    GETBORDER(Color, color);
}

bool MMSWidget::getBorderSelColor(MMSFBColor &selcolor) {
    GETBORDER(SelColor, selcolor);
}

bool MMSWidget::getBorderImagePath(string &imagepath) {
    GETBORDER(ImagePath, imagepath);
}

bool MMSWidget::getBorderImageNames(MMSBORDER_IMAGE_NUM num, string &imagename) {
    GETBORDER_IMAGES(ImageNames, num, imagename);
}

bool MMSWidget::getBorderSelImagePath(string &selimagepath) {
    GETBORDER(SelImagePath, selimagepath);
}

bool MMSWidget::getBorderSelImageNames(MMSBORDER_IMAGE_NUM num, string &selimagename) {
    GETBORDER_IMAGES(SelImageNames, num, selimagename);
}

bool MMSWidget::getBorderThickness(unsigned int &thickness) {
    GETBORDER(Thickness, thickness);
}

bool MMSWidget::getBorderMargin(unsigned int &margin) {
    GETBORDER(Margin, margin);
}

bool MMSWidget::getBorderRCorners(bool &rcorners) {
    GETBORDER(RCorners, rcorners);
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

bool MMSWidget::setBgColor(MMSFBColor bgcolor, bool refresh) {
	if (!this->da) return false;
	this->da->myWidgetClass.setBgColor(bgcolor);
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setSelBgColor(MMSFBColor selbgcolor, bool refresh) {
	if (!this->da) return false;
	this->da->myWidgetClass.setSelBgColor(selbgcolor);
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBgColor_p(MMSFBColor bgcolor_p, bool refresh) {
	if (!this->da) return false;
	this->da->myWidgetClass.setBgColor_p(bgcolor_p);
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setSelBgColor_p(MMSFBColor selbgcolor_p, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setSelBgColor(selbgcolor_p);
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBgColor_i(MMSFBColor bgcolor_i, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setBgColor_i(bgcolor_i);
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setSelBgColor_i(MMSFBColor selbgcolor_i, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setSelBgColor(selbgcolor_i);
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBgImagePath(string bgimagepath, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setBgImagePath(bgimagepath);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->bgimage);
            string path, name;
            if (!getBgImagePath(path)) path = "";
            if (!getBgImageName(name)) name = "";
            this->da->bgimage = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBgImageName(string bgimagename, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setBgImageName(bgimagename);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->bgimage);
            string path, name;
            if (!getBgImagePath(path)) path = "";
            if (!getBgImageName(name)) name = "";
            this->da->bgimage = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setSelBgImagePath(string selbgimagepath, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setSelBgImagePath(selbgimagepath);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->selbgimage);
            string path, name;
            if (!getSelBgImagePath(path)) path = "";
            if (!getSelBgImageName(name)) name = "";
            this->da->selbgimage = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setSelBgImageName(string selbgimagename, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setSelBgImageName(selbgimagename);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->selbgimage);
            string path, name;
            if (!getSelBgImagePath(path)) path = "";
            if (!getSelBgImageName(name)) name = "";
            this->da->selbgimage = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBgImagePath_p(string bgimagepath_p, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setBgImagePath_p(bgimagepath_p);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->bgimage_p);
            string path, name;
            if (!getBgImagePath_p(path)) path = "";
            if (!getBgImageName_p(name)) name = "";
            this->da->bgimage_p = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBgImageName_p(string bgimagename_p, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setBgImageName_p(bgimagename_p);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->bgimage_p);
            string path, name;
            if (!getBgImagePath_p(path)) path = "";
            if (!getBgImageName_p(name)) name = "";
            this->da->bgimage_p = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setSelBgImagePath_p(string selbgimagepath_p, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setSelBgImagePath_p(selbgimagepath_p);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->selbgimage_p);
            string path, name;
            if (!getSelBgImagePath_p(path)) path = "";
            if (!getSelBgImageName_p(name)) name = "";
            this->da->selbgimage_p = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setSelBgImageName_p(string selbgimagename_p, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setSelBgImageName_p(selbgimagename_p);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->selbgimage_p);
            string path, name;
            if (!getSelBgImagePath_p(path)) path = "";
            if (!getSelBgImageName_p(name)) name = "";
            this->da->selbgimage_p = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBgImagePath_i(string bgimagepath_i, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setBgImagePath_i(bgimagepath_i);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->bgimage_i);
            string path, name;
            if (!getBgImagePath_i(path)) path = "";
            if (!getBgImageName_i(name)) name = "";
            this->da->bgimage_i = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBgImageName_i(string bgimagename_i, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setBgImageName_i(bgimagename_i);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->bgimage_i);
            string path, name;
            if (!getBgImagePath_i(path)) path = "";
            if (!getBgImageName_i(name)) name = "";
            this->da->bgimage_i = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setSelBgImagePath_i(string selbgimagepath_i, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setSelBgImagePath_i(selbgimagepath_i);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->selbgimage_i);
            string path, name;
            if (!getSelBgImagePath_i(path)) path = "";
            if (!getSelBgImageName_i(name)) name = "";
            this->da->selbgimage_i = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setSelBgImageName_i(string selbgimagename_i, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setSelBgImageName_i(selbgimagename_i);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->im->releaseImage(this->da->selbgimage_i);
            string path, name;
            if (!getSelBgImagePath_i(path)) path = "";
            if (!getSelBgImageName_i(name)) name = "";
            this->da->selbgimage_i = this->rootwindow->im->getImage(path, name);
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setMargin(unsigned int margin, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setMargin(margin);

    setInnerGeometry();

    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setFocusable(bool focusable, bool refresh) {
	if (!this->da) return false;
    if (this->focusable_initial) {
        if ((!focusable)&&(isFocused()))
            setFocus(false, refresh);
        this->da->myWidgetClass.setFocusable(focusable);
        return true;
    }
    return false;
}

bool MMSWidget::setSelectable(bool selectable, bool refresh) {
	if (!this->da) return false;
    if (this->selectable_initial) {
        if ((!selectable)&&(isSelected()))
            setSelected(false, refresh);
        this->da->myWidgetClass.setSelectable(selectable);
        return true;
    }
    return false;
}

bool MMSWidget::setUpArrow(string uparrow, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setUpArrow(uparrow);
    this->da->upArrowWidget = NULL;
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setDownArrow(string downarrow, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setDownArrow(downarrow);
    this->da->downArrowWidget = NULL;
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setLeftArrow(string leftarrow, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setLeftArrow(leftarrow);
    this->da->leftArrowWidget = NULL;
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setRightArrow(string rightarrow, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setRightArrow(rightarrow);
    this->da->rightArrowWidget = NULL;
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setData(string data) {
	if (!this->da) return false;
    this->da->myWidgetClass.setData(data);
    return true;
}

bool MMSWidget::setNavigateUp(string navigateup) {
	if (!this->da) return false;
    this->da->myWidgetClass.setNavigateUp(navigateup);
    this->da->navigateUpWidget = NULL;
    if ((this->rootwindow)&&(navigateup!=""))
        this->da->navigateUpWidget = this->rootwindow->searchForWidget(navigateup);
    return true;
}

bool MMSWidget::setNavigateDown(string navigatedown) {
	if (!this->da) return false;
    this->da->myWidgetClass.setNavigateDown(navigatedown);
    this->da->navigateDownWidget = NULL;
    if ((this->rootwindow)&&(navigatedown!=""))
        this->da->navigateDownWidget = this->rootwindow->searchForWidget(navigatedown);
    return true;
}

bool MMSWidget::setNavigateLeft(string navigateleft) {
	if (!this->da) return false;
    this->da->myWidgetClass.setNavigateLeft(navigateleft);
    this->da->navigateLeftWidget = NULL;
    if ((this->rootwindow)&&(navigateleft!=""))
        this->da->navigateLeftWidget = this->rootwindow->searchForWidget(navigateleft);
    return true;
}

bool MMSWidget::setNavigateRight(string navigateright) {
	if (!this->da) return false;
    this->da->myWidgetClass.setNavigateRight(navigateright);
    this->da->navigateRightWidget = NULL;
    if ((this->rootwindow)&&(navigateright!=""))
        this->da->navigateRightWidget = this->rootwindow->searchForWidget(navigateright);
    return true;
}

bool MMSWidget::setVSlider(string vslider) {
	if (!this->da) return false;
    this->da->myWidgetClass.setVSlider(vslider);
    this->da->vSliderWidget = NULL;
    if ((this->rootwindow)&&(vslider!=""))
        this->da->vSliderWidget = this->rootwindow->searchForWidget(vslider);
    return true;
}

bool MMSWidget::setHSlider(string hslider) {
	if (!this->da) return false;
    this->da->myWidgetClass.setHSlider(hslider);
    this->da->hSliderWidget = NULL;
    if ((this->rootwindow)&&(hslider!=""))
        this->da->hSliderWidget = this->rootwindow->searchForWidget(hslider);
    return true;
}

bool MMSWidget::setImagesOnDemand(bool imagesondemand) {
	if (!this->da) return false;
    this->da->myWidgetClass.setImagesOnDemand(imagesondemand);
    return true;
}

bool MMSWidget::setBlend(unsigned int blend, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setBlend(blend);
    for (unsigned int i=0; i < children.size(); i++)
        children.at(i)->setBlend(blend, false);
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBlendFactor(double blendfactor, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.setBlendFactor(blendfactor);
    for (unsigned int i=0; i < children.size(); i++)
        children.at(i)->setBlendFactor(blendfactor, false);
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setScrollOnFocus(bool scrollonfocus) {
	if (!this->da) return false;
    this->da->myWidgetClass.setScrollOnFocus(scrollonfocus);
    return true;
}

bool MMSWidget::setClickable(bool clickable) {
	if (!this->da) return false;
	this->da->myWidgetClass.setClickable(clickable);
    return true;
}

bool MMSWidget::setReturnOnScroll(bool returnonscroll) {
	if (!this->da) return false;
	this->da->myWidgetClass.setClickable(returnonscroll);
    return true;
}

bool MMSWidget::setInputMode(string inputmode) {
	if (!this->da) return false;
    this->da->myWidgetClass.setInputMode(inputmode);
    return true;
}

bool MMSWidget::setJoinedWidget(string joinedwidget) {
	if (!this->da) return false;
    this->da->myWidgetClass.setJoinedWidget(joinedwidget);
    this->da->joinedWidget = NULL;
    if ((this->rootwindow)&&(joinedwidget!=""))
        this->da->joinedWidget = this->rootwindow->searchForWidget(joinedwidget);
    return true;
}

bool MMSWidget::setBorderColor(MMSFBColor bordercolor, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.border.setColor(bordercolor);
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBorderSelColor(MMSFBColor borderselcolor, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.border.setSelColor(borderselcolor);
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBorderImagePath(string borderimagepath, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.border.setImagePath(borderimagepath);
    if (load)
        if (this->rootwindow) {
            string path, name;
            if (!getBorderImagePath(path)) path = "";
            for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++) {
	            this->rootwindow->im->releaseImage(this->da->borderimages[i]);
	            if (!getBorderImageNames((MMSBORDER_IMAGE_NUM)i, name)) name = "";
	            this->da->borderimages[i] = this->rootwindow->im->getImage(path, name);
            }
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBorderImageNames(string imagename_1, string imagename_2, string imagename_3, string imagename_4,
                                    string imagename_5, string imagename_6, string imagename_7, string imagename_8,
                                    bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.border.setImageNames(imagename_1, imagename_2, imagename_3, imagename_4,
                                       imagename_5, imagename_6, imagename_7, imagename_8);
    if (load)
        if (this->rootwindow) {
            string path, name;
            if (!getBorderImagePath(path)) path = "";
            for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++) {
	            this->rootwindow->im->releaseImage(this->da->borderimages[i]);
	            if (!getBorderImageNames((MMSBORDER_IMAGE_NUM)i, name)) name = "";
	            this->da->borderimages[i] = this->rootwindow->im->getImage(path, name);
            }
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBorderSelImagePath(string borderselimagepath, bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.border.setSelImagePath(borderselimagepath);
    if (load)
        if (this->rootwindow) {
            string path, name;
            if (!getBorderSelImagePath(path)) path = "";
            for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++) {
	            this->rootwindow->im->releaseImage(this->da->borderselimages[i]);
	            if (!getBorderSelImageNames((MMSBORDER_IMAGE_NUM)i, name)) name = "";
	            this->da->borderselimages[i] = this->rootwindow->im->getImage(path, name);
            }
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBorderSelImageNames(string selimagename_1, string selimagename_2, string selimagename_3, string selimagename_4,
                                       string selimagename_5, string selimagename_6, string selimagename_7, string selimagename_8,
                                       bool load, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.border.setSelImageNames(selimagename_1, selimagename_2, selimagename_3, selimagename_4,
                                          selimagename_5, selimagename_6, selimagename_7, selimagename_8);
    if (load)
        if (this->rootwindow) {
            string path, name;
            if (!getBorderSelImagePath(path)) path = "";

            for (int i=0;i<MMSBORDER_IMAGE_NUM_SIZE;i++) {
	            this->rootwindow->im->releaseImage(this->da->borderselimages[i]);
	            if (!getBorderSelImageNames((MMSBORDER_IMAGE_NUM)i, name)) name = "";
	            this->da->borderselimages[i] = this->rootwindow->im->getImage(path, name);
            }
        }
    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBorderThickness(unsigned int borderthickness, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.border.setThickness(borderthickness);

    setInnerGeometry();

    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBorderMargin(unsigned int bordermargin, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.border.setMargin(bordermargin);

    setInnerGeometry();

    if (refresh)
        this->refresh();
    return true;
}

bool MMSWidget::setBorderRCorners(bool borderrcorners, bool refresh) {
	if (!this->da) return false;
    this->da->myWidgetClass.border.setRCorners(borderrcorners);

    if (refresh)
        this->refresh();
    return true;
}

void MMSWidget::updateFromThemeClass(MMSWidgetClass *themeClass) {

	bool 			b;
	MMSFBColor		c;
	string 			s;
	unsigned int	u;
	double			d;

    if (themeClass->getImagesOnDemand(b))
        setImagesOnDemand(b);
    if (themeClass->getBgColor(c))
        setBgColor(c);
    if (themeClass->getSelBgColor(c))
        setSelBgColor(c);
    if (themeClass->getBgColor_p(c))
        setBgColor_p(c);
    if (themeClass->getSelBgColor_p(c))
        setSelBgColor_p(c);
    if (themeClass->getBgColor_i(c))
        setBgColor_i(c);
    if (themeClass->getSelBgColor_i(c))
        setSelBgColor_i(c);
   	if (themeClass->getBgImagePath(s))
        setBgImagePath(s);
    if (themeClass->getBgImageName(s))
        setBgImageName(s);
    if (themeClass->getSelBgImagePath(s))
        setSelBgImagePath(s);
    if (themeClass->getSelBgImageName(s))
        setSelBgImageName(s);
    if (themeClass->getBgImagePath_p(s))
        setBgImagePath_p(s);
    if (themeClass->getBgImageName_p(s))
        setBgImageName_p(s);
    if (themeClass->getSelBgImagePath_p(s))
        setSelBgImagePath_p(s);
    if (themeClass->getSelBgImageName_p(s))
        setSelBgImageName_p(s);
    if (themeClass->getBgImagePath_i(s))
        setBgImagePath_i(s);
    if (themeClass->getBgImageName_i(s))
        setBgImageName_i(s);
    if (themeClass->getSelBgImagePath_i(s))
        setSelBgImagePath_i(s);
    if (themeClass->getSelBgImageName_i(s))
        setSelBgImageName_i(s);
    if (themeClass->getMargin(u))
        setMargin(u);
    if (themeClass->getFocusable(b))
        setFocusable(b);
    if (themeClass->getSelectable(b))
        setSelectable(b);
    if (themeClass->getUpArrow(s))
        setUpArrow(s);
    if (themeClass->getDownArrow(s))
        setDownArrow(s);
    if (themeClass->getLeftArrow(s))
        setLeftArrow(s);
    if (themeClass->getRightArrow(s))
        setRightArrow(s);
    if (themeClass->getData(s))
        setData(s);
    if (themeClass->getNavigateUp(s))
        setNavigateUp(s);
    if (themeClass->getNavigateDown(s))
        setNavigateDown(s);
    if (themeClass->getNavigateLeft(s))
        setNavigateLeft(s);
    if (themeClass->getNavigateRight(s))
        setNavigateRight(s);
    if (themeClass->getVSlider(s))
        setVSlider(s);
    if (themeClass->getHSlider(s))
        setHSlider(s);
    if (themeClass->getBlend(u))
        setBlend(u);
    if (themeClass->getBlendFactor(d))
        setBlendFactor(d);
    if (themeClass->getScrollOnFocus(b))
        setScrollOnFocus(b);
    if (themeClass->getClickable(b))
        setClickable(b);
    if (themeClass->getReturnOnScroll(b))
        setReturnOnScroll(b);
    if (themeClass->getInputMode(s))
        setInputMode(s);
    if (themeClass->getJoinedWidget(s))
        setJoinedWidget(s);
    if (themeClass->border.getColor(c))
        setBorderColor(c);
    if (themeClass->border.getSelColor(c))
        setBorderSelColor(c);
    if (themeClass->border.getImagePath(s))
        setBorderImagePath(s);
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
        setBorderImageNames(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
    }
    if (themeClass->border.getSelImagePath(s))
        setBorderSelImagePath(s);
    if (themeClass->border.isSelImageNames()) {
    	string s[8];
    	themeClass->border.getSelImageNames(MMSBORDER_IMAGE_NUM_TOP_LEFT, s[0]);
    	themeClass->border.getSelImageNames(MMSBORDER_IMAGE_NUM_TOP, s[1]);
    	themeClass->border.getSelImageNames(MMSBORDER_IMAGE_NUM_TOP_RIGHT, s[2]);
    	themeClass->border.getSelImageNames(MMSBORDER_IMAGE_NUM_RIGHT, s[3]);
    	themeClass->border.getSelImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_RIGHT, s[4]);
    	themeClass->border.getSelImageNames(MMSBORDER_IMAGE_NUM_BOTTOM, s[5]);
    	themeClass->border.getSelImageNames(MMSBORDER_IMAGE_NUM_BOTTOM_LEFT, s[6]);
    	themeClass->border.getSelImageNames(MMSBORDER_IMAGE_NUM_LEFT, s[7]);
        setBorderSelImageNames(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
    }
    if (themeClass->border.getThickness(u))
        setBorderThickness(u);
    if (themeClass->border.getMargin(u))
        setBorderMargin(u);
    if (themeClass->border.getRCorners(b))
        setBorderRCorners(b);
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
