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

#include "mmsgui/theme/mmslabelwidgetclass.h"
#include <string.h>

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_LABELWIDGET_ATTR_I[] = MMSGUI_LABELWIDGET_ATTR_INIT;

//address attribute names
#define GETATTRNAME(aname) MMSGUI_LABELWIDGET_ATTR_I[MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_##aname].name
#define ISATTRNAME(aname) (strcmp(attrname, GETATTRNAME(aname))==0)


MMSLabelWidgetClass::MMSLabelWidgetClass() {
    unsetAll();
}

void MMSLabelWidgetClass::unsetAll() {
    this->className = "";
    unsetFontPath();
    unsetFontName();
    unsetFontSize();
    unsetAlignment();
    unsetColor();
    unsetSelColor();
    unsetText();
    unsetSlidable();
    unsetSlideSpeed();
    unsetTranslate();
}

void MMSLabelWidgetClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path) {
    MMSFBColor color;
    bool class_set = false;

    if (!prefix) {
		startTAFFScan
		{
	        switch (attrid) {
			case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_class:
	            setClassName(attrval_str);
	            class_set = true;
				break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_font_path:
	            if (*attrval_str)
	                setFontPath(attrval_str);
	            else
	                setFontPath((path)?*path:"");
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_font_name:
	            setFontName(attrval_str);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_font_size:
	            setFontSize(attrval_int);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_alignment:
	            setAlignment(getAlignmentFromString(attrval_str));
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_color:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            if (getColorFromString(attrval_str, &color))
	                setColor(color);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_color_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.a = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_color_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.r = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_color_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.g = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_color_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.b = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_selcolor:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            if (getColorFromString(attrval_str, &color))
	                setSelColor(color);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_selcolor_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.a = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_selcolor_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.r = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_selcolor_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.g = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_selcolor_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.b = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_text:
	            setText(attrval_str);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_slidable:
	            setSlidable((attrval_int)?true:false);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_slide_speed:
	            setSlideSpeed(attrval_int);
	            break;
			case MMSGUI_LABELWIDGET_ATTR::MMSGUI_LABELWIDGET_ATTR_IDS_translate:
	            setTranslate((attrval_int)?true:false);
	            break;
			}
		}
		endTAFFScan
    }
    else {
    	unsigned int pl = strlen(prefix->c_str());

    	startTAFFScan_WITHOUT_ID
    	{
    		/* check if attrname has correct prefix */
    		if (pl >= strlen(attrname))
        		continue;
            if (memcmp(attrname, prefix->c_str(), pl)!=0)
            	continue;
            attrname = &attrname[pl];

    		/* okay, correct prefix, check attributes now */
            if (ISATTRNAME(font_path)) {
	            if (*attrval_str)
	                setFontPath(attrval_str);
	            else
	                setFontPath((path)?*path:"");
            }
            else
            if (ISATTRNAME(font_name)) {
	            setFontName(attrval_str);
            }
            else
            if (ISATTRNAME(font_size)) {
	            setFontSize(attrval_int);
            }
            else
            if (ISATTRNAME(alignment)) {
	            setAlignment(getAlignmentFromString(attrval_str));
            }
            else
            if (ISATTRNAME(color)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            if (getColorFromString(attrval_str, &color))
	                setColor(color);
            }
            else
            if (ISATTRNAME(color_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.a = attrval_int;
	            setColor(color);
            }
            else
            if (ISATTRNAME(color_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.r = attrval_int;
	            setColor(color);
            }
            else
            if (ISATTRNAME(color_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.g = attrval_int;
	            setColor(color);
            }
            else
            if (ISATTRNAME(color_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.b = attrval_int;
	            setColor(color);
            }
            else
            if (ISATTRNAME(selcolor)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            if (getColorFromString(attrval_str, &color))
	                setSelColor(color);
            }
            else
            if (ISATTRNAME(selcolor_a)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.a = attrval_int;
	            setSelColor(color);
            }
            else
            if (ISATTRNAME(selcolor_r)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.r = attrval_int;
	            setSelColor(color);
            }
            else
            if (ISATTRNAME(selcolor_g)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.g = attrval_int;
	            setSelColor(color);
            }
            else
            if (ISATTRNAME(selcolor_b)) {
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.b = attrval_int;
	            setSelColor(color);
            }
            else
            if (ISATTRNAME(text)) {
	            setText(attrval_str);
			}
            else
            if (ISATTRNAME(slidable)) {
	            setSlidable((attrval_int)?true:false);
			}
            else
            if (ISATTRNAME(slide_speed)) {
	            setSlideSpeed(attrval_int);
			}
            else
            if (ISATTRNAME(translate)) {
	            setTranslate((attrval_int)?true:false);
			}
    	}
    	endTAFFScan_WITHOUT_ID
    }

    if ((!class_set)&&(path)&&(*path!="")) {
	    if (!isFontPath())
	        setFontPath(*path);
    }
}

void MMSLabelWidgetClass::setClassName(string className) {
    this->className = className;
}

string MMSLabelWidgetClass::getClassName() {
    return this->className;
}

bool MMSLabelWidgetClass::isFontPath() {
    return this->isfontpath;
}

void MMSLabelWidgetClass::setFontPath(string fontpath) {
    this->fontpath = fontpath;
    this->isfontpath = true;
}

void MMSLabelWidgetClass::unsetFontPath() {
    this->isfontpath = false;
}

string MMSLabelWidgetClass::getFontPath() {
    return this->fontpath;
}

bool MMSLabelWidgetClass::isFontName() {
    return this->isfontname;
}

void MMSLabelWidgetClass::setFontName(string fontname) {
    this->fontname = fontname;
    this->isfontname = true;
}

void MMSLabelWidgetClass::unsetFontName() {
    this->isfontname = false;
}

string MMSLabelWidgetClass::getFontName() {
    return this->fontname;
}

bool MMSLabelWidgetClass::isFontSize() {
    return this->isfontsize;
}

void MMSLabelWidgetClass::setFontSize(unsigned int fontsize) {
    this->fontsize = fontsize;
    this->isfontsize = true;
}

void MMSLabelWidgetClass::unsetFontSize() {
    this->isfontsize = false;
}

unsigned int MMSLabelWidgetClass::getFontSize() {
    return this->fontsize;
}

bool MMSLabelWidgetClass::isAlignment() {
    return this->isalignment;
}

void MMSLabelWidgetClass::setAlignment(MMSALIGNMENT alignment) {
    this->alignment = alignment;
    this->isalignment = true;
}

void MMSLabelWidgetClass::unsetAlignment() {
    this->isalignment = false;
}

MMSALIGNMENT MMSLabelWidgetClass::getAlignment() {
    return this->alignment;
}

bool MMSLabelWidgetClass::isColor() {
    return this->iscolor;
}

void MMSLabelWidgetClass::setColor(MMSFBColor color) {
    this->color = color;
    this->iscolor = true;
}

void MMSLabelWidgetClass::unsetColor() {
    this->iscolor = false;
}

MMSFBColor MMSLabelWidgetClass::getColor() {
    return this->color;
}

bool MMSLabelWidgetClass::isSelColor() {
    return this->isselcolor;
}

void MMSLabelWidgetClass::setSelColor(MMSFBColor selcolor) {
    this->selcolor = selcolor;
    this->isselcolor = true;
}

void MMSLabelWidgetClass::unsetSelColor() {
    this->isselcolor = false;
}

MMSFBColor MMSLabelWidgetClass::getSelColor() {
    return this->selcolor;
}

bool MMSLabelWidgetClass::isText() {
    return this->istext;
}

void MMSLabelWidgetClass::setText(string text) {
    this->text = text;
    this->istext = true;
}

void MMSLabelWidgetClass::unsetText() {
    this->istext = false;
}

string MMSLabelWidgetClass::getText() {
    return this->text;
}

bool MMSLabelWidgetClass::isSlidable() {
    return this->isslidable;
}

void MMSLabelWidgetClass::setSlidable(bool slidable) {
    this->slidable = slidable;
    this->isslidable = true;
}

void MMSLabelWidgetClass::unsetSlidable() {
    this->isslidable = false;
}

bool MMSLabelWidgetClass::getSlidable() {
    return this->slidable;
}

bool MMSLabelWidgetClass::isSlideSpeed() {
    return this->isslidespeed;
}

void MMSLabelWidgetClass::setSlideSpeed(unsigned char slidespeed) {
    this->slidespeed = slidespeed;
    this->isslidespeed = true;
}

void MMSLabelWidgetClass::unsetSlideSpeed() {
    this->isslidespeed = false;
}

unsigned char MMSLabelWidgetClass::getSlideSpeed() {
    return this->slidespeed;
}


bool MMSLabelWidgetClass::isTranslate() {
    return this->istranslate;
}

void MMSLabelWidgetClass::setTranslate(bool translate) {
    this->translate = translate;
    this->istranslate = true;
}

void MMSLabelWidgetClass::unsetTranslate() {
    this->istranslate = false;
}

bool MMSLabelWidgetClass::getTranslate() {
    return this->translate;
}

