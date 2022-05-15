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

#ifndef MMSINIT_H_
#define MMSINIT_H_

#include <stdlib.h>
#include <string>
#include <mmsbase/interfaces/immsswitcher.h>

//! type of the init flags
typedef int MMSINIT_FLAGS;

//! none
#define MMSINIT_NONE			0x00000000
//! initializing the window manager
#define MMSINIT_WINDOWMANAGER 	0x00000001
//! initializing the plugin manager (the configdb where the plugins are defined is required)
#define MMSINIT_PLUGINMANAGER 	0x00000002
//! initializing the event manager
#define MMSINIT_EVENTS 			0x00000004
//! initializing the graphic backends (x11/dfb)
#define MMSINIT_GRAPHICS 		0x00000008
//! initializing the input manager
#define MMSINIT_INPUTS 			0x00000010
//! initializing the theme manager
#define MMSINIT_THEMEMANAGER	0x00000020
//! initializing the graphic backends including the window, the input and the theme manager
#define MMSINIT_WINDOWS			0x00000039
//! initializing all components
#define MMSINIT_FULL 			0x0000003f

bool mmsInit(MMSINIT_FLAGS flags, int argc = 0, char *argv[] = NULL, string configfile = "",
			 string appl_name = "Disko Application", string appl_icon_name = "Disko Application",
		     MMSConfigDataGlobal *global = NULL, MMSConfigDataDB *configdb = NULL, MMSConfigDataDB *datadb = NULL,
		     MMSConfigDataGraphics *graphics = NULL, MMSConfigDataLanguage *language = NULL);

bool mmsRelease();

bool registerSwitcher(IMMSSwitcher *switcher);

void setPluginRegisterCallback(void(*cb)(MMSPluginManager*));

IMMSWindowManager *getWindowManager();

void setPluginManager(MMSPluginManager *pm);
MMSPluginManager *getPluginManager();

//! get access to the video layer
/*!
\return pointer to the MMSFBLayer video layer object
\note If using only one layer, the graphics and video layer are the same.
*/
MMSFBLayer *getVideoLayer();

//! get access to the graphics layer
/*!
\return pointer to the MMSFBLayer graphics layer object
\note If using only one layer, the graphics and video layer are the same.
*/
MMSFBLayer *getGraphicsLayer();

#endif /*MMSINIT_H_*/
