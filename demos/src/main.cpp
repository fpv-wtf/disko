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
#include <disko.h>

#include "demo.h"

int main(int argc, char *argv[]) {
	string diskorc;


	diskorc="./etc/diskorc.xml";

	if(mmsInit(MMSINIT_EVENTS|MMSINIT_GRAPHICS|MMSINIT_INPUTS|MMSINIT_THEMEMANAGER|MMSINIT_WINDOWS|MMSINIT_WINDOWMANAGER, argc, argv, diskorc,
				 "Testprog", "BerLinux")) {

/*
		printf("started................\n");

		printf("main window...\n");
		MMSWindow *win;
		win = new MMSMainWindow("", "450px", "300px");
		printf("setcolor\n");
		win->setBgColor(MMSFBColor(0xff,0,0,0xbf));
		win->setBorderThickness(20);
		//win->setBorderRCorners(true);
		win->setBorderColor(MMSFBColor(0xff,0xff,0xff,0xff));

		MMSVBoxWidget *vbox = new MMSVBoxWidget(win);
		win->add(vbox);

		MMSHBoxWidget *hb = new MMSHBoxWidget(win);
		vbox->add(hb);
		MMSImageWidget *image1 = new MMSImageWidget(win, "", NULL);
		hb->add(image1);
		MMSImageWidget *image2 = new MMSImageWidget(win, "", NULL);
		hb->add(image2);
		//image2->setBgColor(MMSFBColor(0x00, 0x00, 0x00, 0xff));
		image2->setImageName("photos/fuerteventura_3.png");
		MMSImageWidget *image3 = new MMSImageWidget(win, "", NULL);
		hb->add(image3);

		MMSHBoxWidget *hbox = new MMSHBoxWidget(win);
		vbox->add(hbox);

		MMSLabelWidget *label = new MMSLabelWidget(win, "MYTESTLABEL", NULL);
		hbox->add(label);
		label->setFont("", "share/themes/default/DejaVuSansMono.ttf", 66);
		label->setColor(MMSFBColor(0xff,0xff,0xff,0xff));
		label->setText("pups");
		label->setBgColor(MMSFBColor(0x00, 0xff, 0x00, 0xff));

		MMSArrowWidget *arrow = new MMSArrowWidget(win, "", NULL);
		hbox->add(arrow);
		arrow->setBgColor(MMSFBColor(0x0,0,0xff,0x80));
		arrow->setColor(MMSFBColor(0xff,0xff,0xff,0xff));

		//win->setFadeIn(true);
		win->show();
		printf("main window...<<<\n");

		while(1)sleep(1);
		exit(0);
*/


		//////////

		Demo demo;
		demo.start();
		pause();
	}


	return 0;
}
