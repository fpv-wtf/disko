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

#include "demo.h"

Demo::Demo() : currwindow(0),
			   dragstarted(false) {
	try {
		mainwindow     = dm.loadDialog(config.getData() + "/main.xml");
		mainmenubar    = mainwindow->findWindow("mainmenubar");
		MMSWindow *win = mainwindow->findWindow("greetings_clipping");
		win->onBeforeHandleInput->connect(sigc::mem_fun(this,&Demo::handleWindowInput));
		this->controls.push_back(win);
		this->mainmenu = (MMSMenuWidget *)(mainmenubar->findWidget("mainmenu"));
		this->mainmenu->onSelectItem->connect(sigc::mem_fun(this, &Demo::onMainMenuSel));
		//this->mainmenu->onSelect->connect(sigc::mem_fun(this, &Demo::onMainMenuSel));
		//usleep(50000);

		this->EX = mainwindow->findWindow("EX");
		this->exit_window = mainwindow->findWindow("exit_window");
	    this->onBeforeAnimation_connection = this->exitpulser.onBeforeAnimation.connect(sigc::mem_fun(this, &Demo::showExitWindow_onBeforeAnimation));
	    this->onAnimation_connection = this->exitpulser.onAnimation.connect(sigc::mem_fun(this, &Demo::showExitWindow_onAnimation));
	    this->onAfterAnimation_connection = this->exitpulser.onAfterAnimation.connect(sigc::mem_fun(this, &Demo::showExitWindow_onAfterAnimation));
		this->EXIT = getWidget(this->EX, "QUIT", MMSWIDGETTYPE_BUTTON);
		this->EXIT->onReturn->connect(sigc::mem_fun(this,&Demo::onReturn));
		this->EXIT = getWidget(this->exit_window, "EXIT", MMSWIDGETTYPE_BUTTON);
		this->EXIT->onReturn->connect(sigc::mem_fun(this,&Demo::onReturn));
		this->CONTINUE = getWidget(this->exit_window, "CONTINUE", MMSWIDGETTYPE_BUTTON);
		this->CONTINUE->onReturn->connect(sigc::mem_fun(this,&Demo::onReturn));


	    this->changepulser.onBeforeAnimation.connect(sigc::mem_fun(this, &Demo::switchContents_onBeforeAnimation));
	    this->changepulser.onAnimation.connect(sigc::mem_fun(this, &Demo::switchContents_onAnimation));
	    this->changepulser.onAfterAnimation.connect(sigc::mem_fun(this, &Demo::switchContents_onAfterAnimation));


	} catch(MMSError *e) {
		cout << e->getMessage() << endl;
	}

	//osk
	this->osk = new OskControl(this->dm);
	this->osk->onCharPressed->connect(sigc::mem_fun(this,&Demo::onCharEntry));
	this->osk->getWindow()->onBeforeHandleInput->connect(sigc::mem_fun(this,&Demo::handleWindowInput));
	this->controls.push_back(this->osk->getWindow());
	oskinput = (MMSInputWidget *)(this->osk->getLabelWindow()->findWidget("osk_input"));

	//carrousel
	this->carrousel = new Carrousel(this->dm);
	this->carrousel->getWindow()->onBeforeHandleInput->connect(sigc::mem_fun(this,&Demo::handleWindowInput));
	this->controls.push_back(this->carrousel->getWindow());

	//buttons
	this->buttons = new Buttons(this->dm);
	this->buttons->getWindow()->onBeforeHandleInput->connect(sigc::mem_fun(this,&Demo::handleWindowInput));
	this->controls.push_back(this->buttons->getWindow());

	//photos
	this->photos = new Photos(this->dm);
	this->photos->getWindow()->onBeforeHandleInput->connect(sigc::mem_fun(this,&Demo::handleWindowInput));
	this->controls.push_back(this->photos->getWindow());


	this->gesture = new MMSInputGesture(50);
	this->inMove=false;
	setActionWindows();
}

Demo::~Demo() {

}

MMSWidget *Demo::getWidget(MMSWindow *window, string name, MMSWIDGETTYPE type) {
	try {
		MMSWidget *widget = window->findWidgetAndType(name, type);
		if (!widget) throw new MMSError(1, "widget " + name + "(" + iToStr(type) + ") not found");
		return widget;
	}
	catch(MMSError *error) {
		fprintf(stderr, "Abort due to: %s\n", error->getMessage().c_str());
		exit(0);
	}
}


void Demo::start() {

	this->mainwindow->show();
	MMSWindow *win = controls.at(currwindow);
	win->show();
	win->waitUntilShown();

	this->EX->show();

}

vector <string> lastkeys;

void Demo::onCharEntry(string *key) {

	if(key->compare("enter")==0)
		return;

	string text;
	oskinput->getText(text);
	if(key->compare("backspace") == 0) {
		if(!text.empty()) {
			for(size_t i=0; i<(--lastkeys.end())->size();i++)
				text.erase(--text.end());
			lastkeys.pop_back();
		}
	} else if(key->compare("lang") == 0){
		this->osk->switchLang();
	} else {
		text+=*key;
		lastkeys.push_back(*key);
	}

	oskinput->setText(text);

}

#define MAX_OFFSET	30

bool Demo::showExitWindow_onBeforeAnimation(MMSPulser *pulser) {
	// prepare animation
	if (this->animshow) {
		this->exit_window->hide();
		this->exit_window->setOpacity(220);
		this->exit_window->stretch(20,130,130,20);
		this->exit_window->show();
		this->exit_window->waitUntilShown();
		this->exit_window->setFocus();
	}
	else {
		this->exit_window->stretch();
		this->exit_window->setOpacity(180);
	}
	return true;
}

bool Demo::showExitWindow_onAnimation(MMSPulser *pulser) {
//	printf("getOffset = %f\n", pulser->getOffset());

	double offs = pulser->getOffset();

	this->exit_window->stretch(20  + offs*80/MAX_OFFSET,
							   130 - offs*30/MAX_OFFSET,
							   130 - offs*30/MAX_OFFSET,
							   20  + offs*80/MAX_OFFSET);

	return true;
}

void Demo::showExitWindow_onAfterAnimation(MMSPulser *pulser) {
	if (this->animshow) {
		// stretch window to 100%
		this->exit_window->stretch();
		this->exit_window->setOpacity(255);
	}
	else {
		// hide window
		this->exit_window->hide();
		this->exit_window->waitUntilHidden();
	}
}

void Demo::showExitWindow() {
	MMSWindow *win = controls.at(currwindow);
	win->setFadeOut(false);
	win->setMoveOut(MMSDIRECTION_DOWN);
	win->hide();
	win->waitUntilHidden();

	// start show animation
	this->animshow = true;
	this->exitpulser.setStepsPerSecond(MAX_OFFSET * 5);
	this->exitpulser.setMaxOffset(MAX_OFFSET, MMSPULSER_SEQ_LOG_SOFT_END, MAX_OFFSET / 2);
	this->exitpulser.start(false);
}

void Demo::onReturn(MMSWidget* widget) {
	if (widget == this->EXIT) {
		exit(0);
	}

	if (widget == this->CONTINUE) {
		// start hide animation
		this->animshow = false;
		this->exitpulser.setStepsPerSecond(MAX_OFFSET * 7);
		this->exitpulser.setMaxOffset(MAX_OFFSET, MMSPULSER_SEQ_LINEAR_DESC, MAX_OFFSET);
		this->exitpulser.start(false);
		MMSWindow *win = controls.at(currwindow);
		win->setFadeIn(false);
		win->setMoveOut(MMSDIRECTION_NOTSET);
		win->setMoveIn(MMSDIRECTION_UP);

		win->show();
		win->waitUntilShown();
		win->setMoveIn(MMSDIRECTION_NOTSET);
		return;
	}

	showExitWindow();
}

bool Demo::handleWindowInput(MMSWindow *win, MMSInputEvent *event) {

	if(inMove) {
		gesture->reset();
		return false;
	}

	if(event->type == MMSINPUTEVENTTYPE_BUTTONPRESS) {
		executeDrag(true,0);
		return false;
	}
	if(event->type == MMSINPUTEVENTTYPE_BUTTONRELEASE) {
		if(this->dragstarted) {
			this->dragstarted = false;
			struct timespec curr;
			clock_gettime(CLOCK_REALTIME, &curr);
			int timediff = timespecDiff(&curr, &this->dragstart) / 1000000;
			if(timediff==0) timediff=1;
			velocity = distance * 10000/ timediff;
			if(velocity  >5000 || abs(this->relative_distance)>270) {
				//do the switch
//				printf("do the switch\n");
				//velocity is big enough, commence breaking manouver
				changepulser.setDuration(350);
				changepulser.setMaxCPUUsage(100);
				changepulser.setMaxFrameRate(75);
				//calc distance to travel
//				printf("distance left: %d\n", 800 - abs(this->relative_distance));
				changepulser.setMaxOffset(800 - abs(this->relative_distance), MMSPULSER_SEQ_LINEAR, 800 - abs(this->relative_distance));
				this->switch_window = true;
				inMove=true;
				changepulser.start(false);
				inMove=false;
			} else {
				//reset animation
				printf("reset to normal\n");
				changepulser.setDuration(350);
				changepulser.setMaxCPUUsage(100);
				changepulser.setMaxFrameRate(75);
				//calc distance to travel
//				printf("distance left: %d\n", abs(this->relative_distance));
				this->switch_window = false;
				changepulser.setMaxOffset(abs(this->relative_distance), MMSPULSER_SEQ_LINEAR,abs(this->relative_distance));
				inMove=true;
				changepulser.start(false);
				inMove=false;
			}
			event->posx=-1;
			event->posy=-1;
		}


#if 0
		switch(this->gesture->guess()) {
			case MMSINPUTGESTURE_DOWN:
				gesture->reset();
				return false;

			case MMSINPUTGESTURE_UP:
				gesture->reset();
				return false;

			case MMSINPUTGESTURE_LEFT:
				changeWindow(MMSINPUTGESTURE_LEFT);
				event->posx=-1;
				event->posy=-1;
				break;

			case MMSINPUTGESTURE_RIGHT:
				changeWindow(MMSINPUTGESTURE_RIGHT);
				event->posx=-1;
				event->posy=-1;
				break;

			default:
				this->gesture->reset();
				return false;
		}
		this->gesture->reset();
#endif
		return false;
	}
	if(event->type == MMSINPUTEVENTTYPE_AXISMOTION) {
		executeDrag(false, event->dx);
		struct timespec curr;
		clock_gettime(CLOCK_REALTIME, &curr);
		int timediff = timespecDiff(&curr, &this->dragstart) / 1000000;

		if(timediff>550) {
			this->dragstart = curr;
			this->distance=0;
		} else {
			this->distance += abs(event->dy);
		}


		//		this->gesture->addInfo(event);
		return false;
	}

	return false;
}

void Demo::changeWindow(MMSINPUTGESTURE direction) {
	MMSWindow *win;
	MMSWindow *win2;
	int x1,y1,x2,y2 = 0;
	int xiter = 0, yiter = 0;
	this->inMove = true;

	if(this->controls.size()>1) {
		win=this->controls.at(this->currwindow);

		if(direction == MMSINPUTGESTURE_RIGHT) {
			if(currwindow>=1)
				this->currwindow--;
			else
				this->currwindow = this->controls.size()-1;
		} else {
			currwindow++;
			if(this->currwindow >= (int)this->controls.size())
				this->currwindow = 0;
		}
		mainmenu->setSelected(this->currwindow);
		printf("currwindow %d\n", currwindow);
		win2=this->controls.at(this->currwindow);

		if(direction == MMSINPUTGESTURE_UP) {
			yiter=-60;
			y1=100;x1=0;
			y2=700;x2=0;
		} else if(direction == MMSINPUTGESTURE_DOWN) {
			yiter=60;
			y1=100;x1=0;
			y2=-500;x2=0;
		} else if(direction == MMSINPUTGESTURE_LEFT) {
			xiter=-80;
			y1=100;x1=0;
			y2=100;x2=800;
		} else if(direction == MMSINPUTGESTURE_RIGHT) {
			xiter=80;
			y1=100;x1=0;
			y2=100;x2=-800;
		}

		win->setFadeOut(false);
		win2->setFadeOut(false);
		win2->moveTo(x2,y2);
		win2->show();
		for(int i = 1; i<=10; i++) {
			//printf("move window1(%d:%d) window2(%d:%d)\n",x1+(xiter*i),y1+(yiter*i),x2+(xiter*i),y2+(yiter*i));
			win->moveTo(x1+(xiter*i),y1+(yiter*i));
			win2->moveTo(x2+(xiter*i),y2+(yiter*i));
			usleep(50);
		}
		win->hide();
	}
	this->inMove = false;
}

void Demo::onMainMenuSel(MMSWidget *wid) {
	if(!inMove) {
//	printf("hide: %d\n",this->currwindow);
		MMSWindow *win = this->controls.at(this->currwindow);
		win->setFadeOut(false);
		win->setMoveOut(MMSDIRECTION_UP);
		win->lowerToBottom();
		win->hide(false,true);
		win->waitUntilHidden();
		win->setMoveOut(MMSDIRECTION_NOTSET);

		string data;
		wid->getData(data);
		this->currwindow = strToInt(data);
//		printf("show: %d\n",this->currwindow);
		win = this->controls.at(this->currwindow);
		win->setFadeIn(false);
		win->setMoveIn(MMSDIRECTION_UP);
		win->moveTo(0,100);
		win->show();
		win->waitUntilShown();
		win->setMoveIn(MMSDIRECTION_NOTSET);
		setActionWindows();
	}

	if (!this->EX->isShown())
		this->EX->show();
	else
		this->EX->raiseToTop();

//	printf("sel %s(%s)\n", wid->getName().c_str(), data.c_str());
}

bool Demo::switchContents_onBeforeAnimation(MMSPulser *pulser) {
	this->initialx = this->mainaction->getGeometry().x;
	initial_threshold = false;
	return true;
}

bool Demo::switchContents_onAnimation(MMSPulser *pulser) {
	int dx;
	int factor = 1;

	if(!switch_window) {
		if(this->relative_distance > 0 ) {
			factor = -1;
		}
	} else {
		if(this->relative_distance < 0 ) {
			factor = -1;
		}
	}
	dx =  55 * factor * (int)pulser->getOffset() *2;

	if(switch_window) {
		if(abs(dx)>800-abs(relative_distance)) {
			if(this->relative_distance<0) {
				this->prevaction->setFadeOut(false);
				this->prevaction->setMoveOut(MMSDIRECTION_NOTSET);
				this->prevaction->hide();
				this->prevaction->waitUntilHidden();

				this->nextaction->moveTo(0,100);
				this->mainaction->setFadeOut(false);
				this->mainaction->setMoveOut(MMSDIRECTION_NOTSET);
				this->mainaction->hide();
				this->mainaction->waitUntilHidden();
			} else {
				this->prevaction->moveTo(0,100);
				this->mainaction->setFadeOut(false);
				this->mainaction->setMoveOut(MMSDIRECTION_NOTSET);
				this->mainaction->hide();
				this->mainaction->waitUntilHidden();

				this->nextaction->setFadeOut(false);
				this->nextaction->setMoveOut(MMSDIRECTION_NOTSET);
				this->nextaction->hide();
				this->nextaction->waitUntilHidden();

			}
			return false;
		}
	} else {
		if(abs(dx)>abs(relative_distance)) {
			this->prevaction->moveTo(-800,100,false);
			this->nextaction->moveTo(800,100, false);
			this->mainaction->moveTo(0,100, false);
			MMSFBRegion region(0, 100, 799, 479);
			this->mainwindow->refresh(&region);
			return false;
		}
	}
//	printf("mainaction moveto: %d\n", initialx+dx);
	this->mainaction->moveTo(initialx+dx, 100, false);
	this->prevaction->moveTo(-800 + initialx+dx, 100, false);
	this->nextaction->moveTo(800 + initialx+dx, 100, false);
	MMSFBRegion region(0, 100, 799, 479);
	this->mainwindow->refresh(&region);
	return true;
}

void Demo::switchContents_onAfterAnimation(MMSPulser *pulser) {
//	printf("onAfterAnim: %d\n", currwindow);
//	usleep(50000);
	if(switch_window) {
//		printf("switch_window: true\n");
		if(this->relative_distance>0) {
			currwindow--;
			if(currwindow<0)
				currwindow = this->controls.size()-1;
		} else {
			currwindow++;
			if(currwindow== (int)this->controls.size())
				currwindow = 0;
		}
		mainmenu->setSelected(this->currwindow);

		if(this->relative_distance<0) {
			this->prevaction->hide();
			this->prevaction->waitUntilHidden();

			this->nextaction->moveTo(0,100);
			this->mainaction->hide();
			this->mainaction->waitUntilHidden();
		} else {
			this->prevaction->moveTo(0,100);
			this->mainaction->hide();
			this->mainaction->waitUntilHidden();

			this->nextaction->hide();
			this->nextaction->waitUntilHidden();

		}
//		printf("new curr: %d\n", currwindow);
		setActionWindows();

	} else {
		this->prevaction->moveTo(-800,100, false);
		this->nextaction->moveTo(800,100, false);
		this->mainaction->moveTo(0,100, false);
		MMSFBRegion region(0, 100, 799, 479);
		this->mainwindow->refresh(&region);
	}


}

void Demo::setActionWindows() {
	 this->mainaction = this->controls.at(this->currwindow);
	 if(this->currwindow >0) {
		 this->prevaction = this->controls.at(this->currwindow-1);
	 } else {
		 this->prevaction = this->controls.at(this->controls.size()-1);
	 }
	 if(this->currwindow + 1 == (int)controls.size()) {
		 this->nextaction = this->controls.at(0);
	 } else {
		 this->nextaction = this->controls.at(this->currwindow + 1);
	 }
}

void Demo::executeDrag(bool initial, int dx) {
	MMSFBRectangle maingeom;

	if(initial) {
		distance= 0;
		relative_distance = 0;
		this->prevaction->setFadeIn(false);
		this->prevaction->setMoveIn(MMSDIRECTION_NOTSET);
		this->prevaction->moveTo(-800,100);
		this->prevaction->show();

		this->nextaction->setFadeIn(false);
		this->nextaction->setMoveIn(MMSDIRECTION_NOTSET);
		this->nextaction->moveTo(800,100);
		this->nextaction->show();
		initial_threshold=false;
		drag_threshold=0;

		return;
	}

	drag_threshold+=dx;

	if(initial_threshold == false) {
		if(abs(drag_threshold)<40) {
			return;
		} else {
			//the initial threshold is met go on
			initial_threshold = true;
			this->dragstarted = true;
		}
	} else {
		if(abs(drag_threshold)<20) {
			return;
		}
	}


//	printf("111111111111\n");

	distance+= abs(dx);
	relative_distance+= drag_threshold;

	maingeom = this->mainaction->getGeometry();
	this->mainaction->moveTo(maingeom.x+drag_threshold, 100, false);
	this->prevaction->moveTo(-800 + maingeom.x+drag_threshold, 100, false);
	this->nextaction->moveTo(800 + maingeom.x+drag_threshold, 100, false);
	MMSFBRegion region(0, 100, 799, 479);
	this->mainwindow->refresh(&region);

	this->drag_threshold = 0;
//	printf("x:%d\n", maingeom.x);
}
