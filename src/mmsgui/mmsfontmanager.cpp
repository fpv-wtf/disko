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

#include "mmsgui/mmsfontmanager.h"

MMSFontManager::MMSFontManager() {
}

MMSFontManager::~MMSFontManager() {
    /* free all fonts */
	for(vector<MMSFM_DESC>::iterator it = this->fonts.begin(); it != this->fonts.end(); ++it) {
		if(it->font) {
			delete it->font;
			break;
		}
	}

	this->fonts.clear();
}

MMSFBFont *MMSFontManager::getFont(string path, string filename, unsigned int size) {
    string          fontfile;
    MMSFM_DESC      fm_desc;

    /* build filename */
    if((path.empty() && filename.empty()) || size == 0) {
		return NULL;
	}
    
    if(path.empty()) {
		fontfile = filename;
	} else {
		fontfile = path +"/" + filename;
	}

    // lock threads
    this->lock.lock();

    /* search within fonts list */
	for(vector<MMSFM_DESC>::iterator it = this->fonts.begin(); it != this->fonts.end(); ++it) {
		if((it->fontfile == fontfile) && (it->size == size)) {
			it->refcnt++;
			this->lock.unlock();
			return it->font;
		}
	}

    /* load font */
    fm_desc.font = NULL;
    if (!loadFont(&(fm_desc.font), "", fontfile, 0, size)) {
        DEBUGMSG("MMSGUI", "cannot load font file '%s'", fontfile.c_str());
        this->lock.unlock();
        return NULL;
    }
    fm_desc.fontfile = fontfile;
    fm_desc.size = size;
    fm_desc.refcnt = 1;

    /* add to fonts list and return the font */
    this->fonts.push_back(fm_desc);
    this->lock.unlock();
    return fm_desc.font;
}

void MMSFontManager::releaseFont(string path, string filename, unsigned int size) {
    /*TODO*/
}

void MMSFontManager::releaseFont(MMSFBFont *font) {
    if(font) {
    	this->lock.lock();
    	for(vector<MMSFM_DESC>::iterator it = this->fonts.begin(); it != this->fonts.end(); ++it) {
    		if(it->font == font) {
				if(--it->refcnt == 0) {
					this->fonts.erase(it);
					delete font;
				}
    			break;
    		}
    	}
    	this->lock.unlock();
    }
}

