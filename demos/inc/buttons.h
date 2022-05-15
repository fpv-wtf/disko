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

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "diskocontrol.h"
#include "mmstools/mmspulser.h"

class Buttons : public DiskoControl {
	public:
		Buttons(MMSDialogManager &dm, string name ="");
		~Buttons();

	private:
		MMSWindow *contents;
		bool handleWindowInput(MMSWindow *win, MMSInputEvent *event);
		bool drag;
		bool first;
		bool stopAnim;
		bool onBeforeShowContent(MMSWindow *win);
		MMSProgressBarWidget *progress;
		MMSSliderWidget *slider;
		void onClick(MMSWidget *wid, int x,int y,int w,int h);
		void onRadio(MMSWidget *wid);

		int velocity;
		struct timespec dragstart;
		int starty;
		int distance;
		int direction;
		int maxy;
		int animstarty;
		MMSPulser pulser;

        int drag_threshold;
		bool dragstarted;

		bool onAnim(MMSPulser *pulser);

//		bool onBeforeAnim(MMSPulser *pulser);

		vector<MMSCheckBoxWidget *> radio;

};

#endif /* BUTTONS_H_ */
