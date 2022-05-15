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

#ifndef MMSFBMANAGER_H_
#define MMSFBMANAGER_H_

#include "mmsgui/fb/mmsfb.h"
#include "mmsconfig/mmsconfigdata.h"


class MMSFBManager {

    private:
        MMSFBLayer          *graphicslayer;
        MMSFBLayer          *videolayer;
        int                 videolayerid;
        int                 graphicslayerid;
        MMSConfigData       config;
        int                 layercount;

    public:
        /* constructor */
        MMSFBManager();

        /* destructor */
        ~MMSFBManager();

        bool init(int argc, char **argv,
				  string appl_name = "Disko Application", string appl_icon_name = "Disko Application");
        void release();

        void applySettings();
        MMSFBLayer *getVideoLayer();
        MMSFBLayer *getGraphicsLayer();

        int getLayerCount();
};

/* access to global mmsfbmanager */
extern MMSFBManager mmsfbmanager;

#endif /*MMSFBMANAGER_H_*/
