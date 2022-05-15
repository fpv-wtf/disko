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

#ifndef DEMO_H_
#define DEMO_H_
#include <disko.h>
#include <vector>

#include "oskcontrol.h"
#include "carrousel.h"
#include "buttons.h"
#include "photos.h"

class Demo {
	public:
		Demo();
		~Demo();

		void start();

	private:
		MMSDialogManager dm;
		MMSWindow	     *mainwindow;
		MMSWindow 		 *mainmenubar;
		MMSWindow 		 *mainaction;
		MMSWindow 		 *prevaction;
		MMSWindow 		 *nextaction;
		MMSConfigData    config;

		OskControl 		 *osk;
		Carrousel        *carrousel;
		Buttons          *buttons;
		Photos           *photos;
		int				 currwindow;
		bool			 dragstarted;
		MMSMenuWidget    *mainmenu;

		std::vector<MMSWindow *> controls;
		MMSInputGesture	 *gesture;
//		MMSLabelWidget   *osklabel;
		MMSInputWidget   *oskinput;

		bool inMove;

		MMSWindow *EX;
		MMSWindow *exit_window;
    	MMSPulser	exitpulser;
    	MMSPulser	changepulser;
    	sigc::connection 	onBeforeAnimation_connection;
        sigc::connection 	onAnimation_connection;
        sigc::connection 	onAfterAnimation_connection;
        bool animshow;

        int distance;
        int relative_distance;
        int drag_threshold;
        bool initial_threshold;
        int velocity;
		struct timespec dragstart;
		int initialx;
		bool switch_window;

        MMSWidget *EXIT;
        MMSWidget *CONTINUE;

		void onCharEntry(string *key);
		void changeWindow(MMSINPUTGESTURE direction);
		bool handleWindowInput(MMSWindow *win, MMSInputEvent *event);
		void onMainMenuSel(MMSWidget *wid);

		MMSWidget *getWidget(MMSWindow *window, string name, MMSWIDGETTYPE type);
		bool showExitWindow_onBeforeAnimation(MMSPulser *pulser);
		bool showExitWindow_onAnimation(MMSPulser *pulser);
		void showExitWindow_onAfterAnimation(MMSPulser *pulser);

		bool switchContents_onBeforeAnimation(MMSPulser *pulser);
		bool switchContents_onAnimation(MMSPulser *pulser);
		void switchContents_onAfterAnimation(MMSPulser *pulser);
		void showExitWindow();
		void onReturn(MMSWidget* widget);

		void setActionWindows();
		void executeDrag(bool initial, int dx);
};

#endif /* DEMO_H_ */
