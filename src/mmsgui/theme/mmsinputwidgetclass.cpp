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

#include "mmsgui/theme/mmsinputwidgetclass.h"
#include <string.h>

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_INPUTWIDGET_ATTR_I[] = MMSGUI_INPUTWIDGET_ATTR_INIT;

//address attribute names
#define GETATTRNAME(aname) MMSGUI_INPUTWIDGET_ATTR_I[MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_##aname].name
#define ISATTRNAME(aname) (strcmp(attrname, GETATTRNAME(aname))==0)


MMSInputWidgetClass::MMSInputWidgetClass() {
    unsetAll();
}

void MMSInputWidgetClass::unsetAll() {
    this->className = "";
    unsetFontPath();
    unsetFontName();
    unsetFontSize();
    unsetAlignment();
    unsetColor();
    unsetSelColor();
    unsetText();
    unsetCursorState();
}

void MMSInputWidgetClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path, bool reset_paths) {
    MMSFBColor color;

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// unset my paths
        unsetFontPath();
    }

    if (!prefix) {
		startTAFFScan
		{
	        switch (attrid) {
			case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_class:
	            setClassName(attrval_str);
				break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_font_path:
	            if (*attrval_str)
	                setFontPath(attrval_str);
	            else
	                setFontPath((path)?*path:"");
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_font_name:
	            setFontName(attrval_str);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_font_size:
	            setFontSize(attrval_int);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_alignment:
	            setAlignment(getAlignmentFromString(attrval_str));
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_color:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            if (getMMSFBColorFromString(attrval_str, &color))
	                setColor(color);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_color_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.a = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_color_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.r = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_color_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.g = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_color_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isColor()) color = getColor();
	            color.b = attrval_int;
	            setColor(color);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_selcolor:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            if (getMMSFBColorFromString(attrval_str, &color))
	                setSelColor(color);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_selcolor_a:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.a = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_selcolor_r:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.r = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_selcolor_g:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.g = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_selcolor_b:
				color.a = color.r = color.g = color.b = 0;
	            if (isSelColor()) color = getSelColor();
	            color.b = attrval_int;
	            setSelColor(color);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_text:
	            setText(attrval_str);
	            break;
			case MMSGUI_INPUTWIDGET_ATTR::MMSGUI_INPUTWIDGET_ATTR_IDS_cursor_state:
				if ((attrval_int & 0xff) == 0x01)
					setCursorState(MMSSTATE_AUTO);
				else
				if (attrval_int)
					setCursorState(MMSSTATE_TRUE);
				else
					setCursorState(MMSSTATE_FALSE);
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
	            if (getMMSFBColorFromString(attrval_str, &color))
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
	            if (getMMSFBColorFromString(attrval_str, &color))
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
            if (ISATTRNAME(cursor_state)) {
				if ((attrval_int & 0xff) == 0x01)
					setCursorState(MMSSTATE_AUTO);
				else
				if (attrval_int)
					setCursorState(MMSSTATE_TRUE);
				else
					setCursorState(MMSSTATE_FALSE);
	            break;
			}
    	}
    	endTAFFScan_WITHOUT_ID
    }

    if ((reset_paths)&&(path)&&(*path!="")) {
    	// set my paths
	    if (!isFontPath())
	        setFontPath(*path);
    }
}

void MMSInputWidgetClass::setClassName(string className) {
    this->className = className;
}

string MMSInputWidgetClass::getClassName() {
    return this->className;
}

bool MMSInputWidgetClass::isFontPath() {
    return this->isfontpath;
}

void MMSInputWidgetClass::setFontPath(string fontpath) {
    this->fontpath = fontpath;
    this->isfontpath = true;
}

void MMSInputWidgetClass::unsetFontPath() {
    this->isfontpath = false;
}

string MMSInputWidgetClass::getFontPath() {
    return this->fontpath;
}

bool MMSInputWidgetClass::isFontName() {
    return this->isfontname;
}

void MMSInputWidgetClass::setFontName(string fontname) {
    this->fontname = fontname;
    this->isfontname = true;
}

void MMSInputWidgetClass::unsetFontName() {
    this->isfontname = false;
}

string MMSInputWidgetClass::getFontName() {
    return this->fontname;
}

bool MMSInputWidgetClass::isFontSize() {
    return this->isfontsize;
}

void MMSInputWidgetClass::setFontSize(unsigned int fontsize) {
    this->fontsize = fontsize;
    this->isfontsize = true;
}

void MMSInputWidgetClass::unsetFontSize() {
    this->isfontsize = false;
}

unsigned int MMSInputWidgetClass::getFontSize() {
    return this->fontsize;
}

bool MMSInputWidgetClass::isAlignment() {
    return this->isalignment;
}

void MMSInputWidgetClass::setAlignment(MMSALIGNMENT alignment) {
    this->alignment = alignment;
    this->isalignment = true;
}

void MMSInputWidgetClass::unsetAlignment() {
    this->isalignment = false;
}

MMSALIGNMENT MMSInputWidgetClass::getAlignment() {
    return this->alignment;
}

bool MMSInputWidgetClass::isColor() {
    return this->iscolor;
}

void MMSInputWidgetClass::setColor(MMSFBColor color) {
    this->color = color;
    this->iscolor = true;
}

void MMSInputWidgetClass::unsetColor() {
    this->iscolor = false;
}

MMSFBColor MMSInputWidgetClass::getColor() {
    return this->color;
}

bool MMSInputWidgetClass::isSelColor() {
    return this->isselcolor;
}

void MMSInputWidgetClass::setSelColor(MMSFBColor selcolor) {
    this->selcolor = selcolor;
    this->isselcolor = true;
}

void MMSInputWidgetClass::unsetSelColor() {
    this->isselcolor = false;
}

MMSFBColor MMSInputWidgetClass::getSelColor() {
    return this->selcolor;
}

bool MMSInputWidgetClass::isText() {
    return this->istext;
}

void MMSInputWidgetClass::setText(string text) {
    this->text = text;
    this->istext = true;
}

void MMSInputWidgetClass::unsetText() {
    this->istext = false;
}

string MMSInputWidgetClass::getText() {
    return this->text;
}

bool MMSInputWidgetClass::isCursorState() {
    return this->iscursor_state;
}

void MMSInputWidgetClass::setCursorState(MMSSTATE cursor_state) {
    this->cursor_state = cursor_state;
    this->iscursor_state = true;
}

void MMSInputWidgetClass::unsetCursorState() {
    this->iscursor_state = false;
}

MMSSTATE MMSInputWidgetClass::getCursorState() {
    return this->cursor_state;
}

