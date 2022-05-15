/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2011 BerLinux Solutions GmbH                       *
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

#ifndef PHOTOS_H_
#define PHOTOS_H_

#include "diskocontrol.h"

typedef enum {
	ANIM_MODE_ZOOM_IN = 0,
	ANIM_MODE_ZOOM_OUT
} ANIM_MODE;

class Photos : public DiskoControl {
	public:
		Photos(MMSDialogManager &dm, string name ="");
		~Photos();

	private:
		MMSWindow		*photos_menuwin;
		MMSMenuWidget	*photos_menu;
		MMSWindow		*photos_zoomwin;
		MMSImageWidget	*photos_image;
		MMSLabelWidget	*photos_image_caption;

		MMSWindow		*photos_finalwin;
		MMSImageWidget	*photos_fimage;
		MMSLabelWidget	*photos_fimage_caption;

        //! pulser object
    	MMSPulser		pulser;

        //! connection object for MMSPulser::onBeforeAnimation callback
        sigc::connection 	onBeforeAnimation_connection;

        //! connection object for MMSPulser::onAnimation callback
        sigc::connection 	onAnimation_connection;

        //! connection object for MMSPulser::onAfterAnimation callback
        sigc::connection 	onAfterAnimation_connection;

        bool animIsRunning;

        double sleft, sup, sright, sdown;
        double fleft, fup, fright, fdown;

        ANIM_MODE animMode;

		bool onBeforeAnimation(MMSPulser *pulser);
		bool onAnimation(MMSPulser *pulser);
		void onAfterAnimation(MMSPulser *pulser);

		void onReturn(MMSWidget* widget);
		bool onHandleInput(MMSWindow *win, MMSInputEvent *event);
};

#endif /* PHOTOS_H_ */
