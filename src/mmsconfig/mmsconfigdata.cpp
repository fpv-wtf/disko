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

#include "mmstools/mmstools.h"
#include "mmsconfig/mmsconfigdata.h"
#include <string.h>

// static variables
MMSConfigDataGlobal 	MMSConfigData::global;
MMSConfigDataDB     	MMSConfigData::configdb;
MMSConfigDataDB     	MMSConfigData::datadb;
MMSConfigDataGraphics   MMSConfigData::graphics;
MMSConfigDataLanguage   MMSConfigData::language;

MMSConfigData::MMSConfigData(MMSConfigDataGlobal 	global,
                             MMSConfigDataDB     	configdb,
                             MMSConfigDataDB     	datadb,
                             MMSConfigDataGraphics  graphics,
                             MMSConfigDataLanguage	language) {
    this->global   = global;
    this->configdb = configdb;
    this->datadb   = datadb;
    this->graphics = graphics;
    this->language = language;
#ifdef HARD_PREFIX
    this->global.prefix = HARD_PREFIX;
#endif
}

MMSConfigData::MMSConfigData() {
#ifdef HARD_PREFIX
    this->global.prefix = HARD_PREFIX;
#endif
}

MMSConfigData::~MMSConfigData() {
}

/* global section getters */
const string MMSConfigData::getLogfile() {
	return this->global.logfile;
}

const string MMSConfigData::getInputMap() {
	return this->global.inputmap;
}

const string MMSConfigData::getPrefix() {
    if(this->global.prefix != "")
        return this->global.prefix;

    FILE *stream;
    char prefix[1024];
    memset(prefix,0,1024);

    stream = popen("pkg-config --variable=prefix mmstools","r");
    if(stream!=NULL) {
        if(fgets(prefix,1024,stream)!=NULL) {
            prefix[strlen(prefix)-1]='/';
            fclose(stream);
            this->global.prefix = prefix;
            return this->global.prefix;
        }

    }

    stream = fopen("./bin/mmscmd.bin","r");
    if(stream != NULL) {
        sprintf(prefix,"./");
        fclose(stream);
        this->global.prefix = prefix;
        return this->global.prefix;
    }

    return this->global.prefix;
}

const string MMSConfigData::getTheme() {
    return this->global.theme;
}

const string MMSConfigData::getSysConfig(){
	return this->global.sysconfig;
}

const string MMSConfigData::getData(){
	return this->global.data;
}

const bool MMSConfigData::getStdout() {
    return this->global.stdout;
}

const int MMSConfigData::getInputInterval() {
    return this->global.inputinterval;
}

const string MMSConfigData::getFirstPlugin() {
	return this->global.firstplugin;
}

const bool   MMSConfigData::getShutdown() {
	return this->global.shutdown;
}

const string MMSConfigData::getShutdownCmd() {
	return this->global.shutdowncmd;
}

const string MMSConfigData::getInputMode() {
	return this->global.inputmode;
}

/* db section getters */
const string MMSConfigData::getConfigDBDBMS() {
	return this->configdb.dbms;
}

const string MMSConfigData::getConfigDBAddress() {
	return this->configdb.address;
}

const unsigned int MMSConfigData::getConfigDBPort() {
	return this->configdb.port;
}

const string MMSConfigData::getConfigDBUser() {
	return this->configdb.user;
}

const string MMSConfigData::getConfigDBPassword() {
	return this->configdb.password;
}

const string MMSConfigData::getConfigDBDatabase() {
    return this->configdb.database;
};

const string MMSConfigData::getDataDBDBMS() {
	return this->datadb.dbms;
}

const string MMSConfigData::getDataDBAddress() {
	return this->datadb.address;
}

const unsigned int MMSConfigData::getDataDBPort() {
	return this->datadb.port;
}

const string MMSConfigData::getDataDBUser() {
	return this->datadb.user;
}

const string MMSConfigData::getDataDBPassword() {
	return this->datadb.password;
}
const string MMSConfigData::getDataDBDatabase() {
    return this->datadb.database;
};

// graphics section getters
const MMSConfigDataLayer MMSConfigData::getVideoLayer() {
    return this->graphics.videolayer;
}

const MMSConfigDataLayer MMSConfigData::getGraphicsLayer() {
    return this->graphics.graphicslayer;
}

const MMSFBBackend MMSConfigData::getBackend() {
    return this->graphics.backend;
}

const MMSFBOutputType MMSConfigData::getOutputType() {
    return this->graphics.outputtype;
}

const MMSFBRectangle MMSConfigData::getVRect() {
    return this->graphics.vrect;
}

const MMSFBRectangle MMSConfigData::getTouchRect() {
    return this->graphics.touchrect;
}

const MMSFBPointerMode MMSConfigData::getPointer() {
    return this->graphics.pointer;
}

const MMSFBSurfacePixelFormat MMSConfigData::getGraphicsWindowPixelformat() {
    return this->graphics.graphicswindowpixelformat;
}

const MMSFBSurfacePixelFormat MMSConfigData::getGraphicsSurfacePixelformat() {
    return this->graphics.graphicssurfacepixelformat;
}

const bool MMSConfigData::getExtendedAccel() {
    return this->graphics.extendedaccel;
}

const string MMSConfigData::getAllocMethod() {
    return this->graphics.allocmethod;
}

const MMSFBFullScreenMode MMSConfigData::getFullScreen() {
	return this->graphics.fullscreen;
}

const bool   MMSConfigData::getHideApplication() {
	return this->graphics.hideapplication;
}

const bool   MMSConfigData::getTouchSwapX() {
	return this->graphics.touchSwapX;
}

const bool   MMSConfigData::getTouchSwapY() {
	return this->graphics.touchSwapY;
}

const bool   MMSConfigData::getTouchSwapXY() {
	return this->graphics.touchSwapXY;
}

const unsigned int MMSConfigData::getTouchResX() {
	return this->graphics.touchResX;
}

const unsigned int MMSConfigData::getTouchResY() {
	return this->graphics.touchResY;
}

// graphics section setters
void MMSConfigData::setVideoLayer(MMSConfigDataLayer layer) {
    this->graphics.videolayer = layer;
}

void MMSConfigData::setGraphicsLayer(MMSConfigDataLayer layer) {
    this->graphics.graphicslayer = layer;
}

void MMSConfigData::setFullScreen(MMSFBFullScreenMode fsm) {
	this->graphics.fullscreen = fsm;
}

void MMSConfigData::setHideApplication(bool hideapplication) {
	this->graphics.hideapplication = hideapplication;
}


const string MMSConfigData::getSourceLang() {
	return this->language.sourcelang;
}

const string MMSConfigData::getDefaultTargetLang() {
	return this->language.defaulttargetlang;
}

const bool MMSConfigData::getAddTranslations() {
	return this->language.addtranslations;
}

const string MMSConfigData::getLanguagefileDir() {
	return this->language.languagefiledir;
}




