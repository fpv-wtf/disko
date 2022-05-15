/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2012 BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <jens.schneider@diskohq.org>,                   *
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

#include "mmsgui/theme/mmscanvaswidgetclass.h"
#include <string.h>

//store attribute descriptions here
TAFF_ATTRDESC MMSGUI_CANVASWIDGET_ATTR_I[] = MMSGUI_CANVASWIDGET_ATTR_INIT;

// address attribute names
#define GETATTRNAME(aname) MMSGUI_CANVASWIDGET_ATTR_I[MMSGUI_CANVASWIDGET_ATTR::MMSGUI_CANVASWIDGET_ATTR_IDS_##aname].name

// address attribute types
#define GETATTRTYPE(aname) MMSGUI_CANVASWIDGET_ATTR_I[MMSGUI_CANVASWIDGET_ATTR::MMSGUI_CANVASWIDGET_ATTR_IDS_##aname].type


MMSCanvasWidgetClass::MMSCanvasWidgetClass() {
    unsetAll();
}

void MMSCanvasWidgetClass::unsetAll() {
    this->className = "";
    isattributes = false;
}

void MMSCanvasWidgetClass::setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix, string *path, bool reset_paths) {
	startTAFFScan
	{
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_class:
            setClassName(attrval_str);
			break;
		case MMSGUI_CANVASWIDGET_ATTR::MMSGUI_CANVASWIDGET_ATTR_IDS_attributes:
            setAttributes(attrval_str);
			break;
		}
	}
	endTAFFScan
}

void MMSCanvasWidgetClass::setClassName(string className) {
    this->className = className;
}

string MMSCanvasWidgetClass::getClassName() {
    return this->className;
}

bool MMSCanvasWidgetClass::isAttributes() {
	return this->isattributes;
}

void MMSCanvasWidgetClass::setAttributes(string attributes) {
	this->isattributes = true;
	this->attributes = attributes;
}

void MMSCanvasWidgetClass::unsetAttributes() {
	this->attributes = "";
	this->isattributes = false;
}

string MMSCanvasWidgetClass::getAttributes() {
	return this->attributes;
}
