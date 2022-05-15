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

#include "mmsgui/mmsprogressbarwidget.h"

MMSProgressBarWidget::MMSProgressBarWidget(MMSWindow *root, string className, MMSTheme *theme) : MMSWidget() {
    create(root, className, theme);
}

MMSProgressBarWidget::~MMSProgressBarWidget() {
}

bool MMSProgressBarWidget::create(MMSWindow *root, string className, MMSTheme *theme) {
	this->type = MMSWIDGETTYPE_PROGRESSBAR;
    this->className = className;

    // init attributes for drawable widgets
	this->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    if (theme) this->da->theme = theme; else this->da->theme = globalTheme;
    this->progressBarWidgetClass = this->da->theme->getProgressBarWidgetClass(className);
    this->da->baseWidgetClass = &(this->da->theme->progressBarWidgetClass.widgetClass);
    if (this->progressBarWidgetClass) this->da->widgetClass = &(this->progressBarWidgetClass->widgetClass); else this->da->widgetClass = NULL;

    return MMSWidget::create(root, true, false, false, true, true, true, false);
}

MMSWidget *MMSProgressBarWidget::copyWidget() {
    /* create widget */
    MMSProgressBarWidget *newWidget = new MMSProgressBarWidget(this->rootwindow, className);

    /* copy widget */
    *newWidget = *this;

    /* copy base widget */
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    return newWidget;
}


bool MMSProgressBarWidget::init() {
    /* init widget basics */
    if (!MMSWidget::init())
        return false;

    return true;
}

bool MMSProgressBarWidget::draw(bool *backgroundFilled) {
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

        MMSFBColor color;

        if (isSelected())
            color = getSelColor();
        else
            color = getColor();

        if (color.a) {
            /* prepare for drawing */
            this->surface->setDrawingColorAndFlagsByBrightnessAndOpacity(color, getBrightness(), getOpacity());

            /* fill the rectangle */
            this->surface->fillRectangle(surfaceGeom.x, surfaceGeom.y,
                                        (int)((double)getProgress() / (double)100 * (double)surfaceGeom.w), surfaceGeom.h);
        }

        /* unlock */
        this->surface->unlock();

        /* update window surface with an area of surface */
        updateWindowSurfaceWithSurface(!*backgroundFilled);
    }

    /* draw widgets debug frame */
    return MMSWidget::drawDebug();
}

/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETPBAR(x) \
    if (this->myProgressBarWidgetClass.is##x()) return myProgressBarWidgetClass.get##x(); \
    else if ((progressBarWidgetClass)&&(progressBarWidgetClass->is##x())) return progressBarWidgetClass->get##x(); \
    else return this->da->theme->progressBarWidgetClass.get##x();

MMSFBColor MMSProgressBarWidget::getColor() {
    GETPBAR(Color);
}

MMSFBColor MMSProgressBarWidget::getSelColor() {
    GETPBAR(SelColor);
}

unsigned int MMSProgressBarWidget::getProgress() {
    GETPBAR(Progress);
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/
void MMSProgressBarWidget::setColor(MMSFBColor color, bool refresh) {
    myProgressBarWidgetClass.setColor(color);
    if (refresh)
        this->refresh();
}

void MMSProgressBarWidget::setSelColor(MMSFBColor selcolor, bool refresh) {
    myProgressBarWidgetClass.setSelColor(selcolor);
    if (refresh)
        this->refresh();
}

void MMSProgressBarWidget::setProgress(unsigned int progress, bool refresh) {
    if(progress>100)
        progress = 100;
    myProgressBarWidgetClass.setProgress(progress);
    if (refresh)
        this->refresh();
}


void MMSProgressBarWidget::updateFromThemeClass(MMSProgressBarWidgetClass *themeClass) {
    if (themeClass->isColor())
        setColor(themeClass->getColor());
    if (themeClass->isSelColor())
        setSelColor(themeClass->getSelColor());
    if (themeClass->isProgress())
        setProgress(themeClass->getProgress());

    MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
