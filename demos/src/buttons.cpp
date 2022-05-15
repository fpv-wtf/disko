/*
 * carrousel.cpp
 *
 *  Created on: Jan 26, 2010
 *      Author: sxs
 */

#include "buttons.h"

Buttons::Buttons(MMSDialogManager &dm, string name) : drag(false) {
	first = true;
	if(!name.empty()) {
		this->controlname = name;
	} else {
		controlname = "Buttons";
	}
	try {
		dm.loadChildDialog(config.getData() + "/buttons.xml");
		MMSWindow *win = dm.getWindow();
		this->controlwin = win->searchForWindow("buttons");
		this->controlwin->setName(this->controlname);

		this->contents = this->controlwin->findWindow("contents");
		this->contents->onBeforeHandleInput->connect(sigc::mem_fun(this,&Buttons::handleWindowInput));
		this->contents->show();

		for(int i = 1; i<4;i++) {
			progress = (MMSProgressBarWidget *)contents->findWidget("progress" + iToStr(i));
			if(progress)
				progress->onClick->connect(sigc::mem_fun(this, &Buttons::onClick));
			slider = (MMSSliderWidget *)contents->findWidget("slider" + iToStr(i));
			if(slider)
				slider->onClick->connect(sigc::mem_fun(this, &Buttons::onClick));
		}
		for(int i = 1; i<13;i++) {
			MMSCheckBoxWidget *rad = (MMSCheckBoxWidget *)contents->findWidget("radio" + iToStr(i));
			if(rad) {
				radio.push_back(rad);
				rad->onReturn->connect(sigc::mem_fun(this, &Buttons::onRadio));
			}
		}
		this->maxy = (this->contents->getGeometry().h  * -1) + 380;

	} catch(MMSDialogManagerError *err) {
		printf("error: %s\n", err->getMessage().c_str());
	}
	this->pulser.onAnimation.connect(sigc::mem_fun(this, &Buttons::onAnim));
}

Buttons::~Buttons() {

}


bool Buttons::handleWindowInput(MMSWindow *win, MMSInputEvent *event) {
	MMSFBRectangle rect = contents->getGeometry();

	if(event->type == MMSINPUTEVENTTYPE_BUTTONPRESS) {
		drag=true;
		stopAnim=true;
		this->starty=event->absy;
		clock_gettime(CLOCK_REALTIME,&(this->dragstart));
		this->distance = 0;
		this->direction = 0;

		this->drag_threshold = 0;
		this->dragstarted = false;
	}
	if(event->type == MMSINPUTEVENTTYPE_BUTTONRELEASE) {
		drag=false;
		struct timespec curr;
		clock_gettime(CLOCK_REALTIME, &curr);
		int timediff = timespecDiff(&curr, &this->dragstart) / 1000000;

		this->drag_threshold = 0;
		this->dragstarted = false;

		if (timediff > 0) {
			if(distance * 10000/ timediff >5000) {
				//velocity is big enough, commence breaking manouver
				pulser.setDuration(500);
				pulser.setMaxCPUUsage(100);
				pulser.setMaxFrameRate(50);
				pulser.setMaxOffset(800, MMSPULSER_SEQ_LOG_SOFT_END);
				this->animstarty = rect.y;
				stopAnim=false;
				pulser.start();
			}
		}

	}
	if(event->type == MMSINPUTEVENTTYPE_AXISMOTION) {
		if(drag) {
			struct timespec curr;
			clock_gettime(CLOCK_REALTIME, &curr);
			int timediff = timespecDiff(&curr, &this->dragstart) / 1000000;
			if(timediff>550) {
				this->dragstart = curr;
				this->distance=0;
				this->direction=0;
			} else {
				this->distance += abs(event->dy);
				this->direction += event->dy;
			}

			this->drag_threshold += event->dy;

			if (!this->dragstarted) {
				if (abs(this->drag_threshold) > 40) {
					this->dragstarted = true;
				}
			}

			if (this->dragstarted) {
				if (abs(this->drag_threshold) > 20) {

			//			printf("move to: %d maxy: %d\n", rect.y+event->dy, this->maxy);
	//				int to = rect.y+event->dy;
					int to = rect.y+this->drag_threshold;

					if(to > 0) {
						to = 0;
					} else if (to < this->maxy) {
						to = maxy;
					}

					contents->moveTo(0,to);

					this->drag_threshold = 0;

				}
			}
		}
	}

	return false;
}

bool Buttons::onBeforeShowContent(MMSWindow *win) {
	//printf("onBeforeShowContent\n");
	if(first) {
		contents->moveTo(0,0);
		first=false;
	}
	return true;
}

void Buttons::onClick(MMSWidget *wid, int x,int y,int w,int h) {
	if (wid->getType() == MMSWIDGETTYPE_SLIDER) {

		unsigned int pos = ((MMSSliderWidget *)wid)->getPosition();
		unsigned int newpos = (x*100)/ wid->getGeometry().w;
		if(newpos < pos) {
			while(pos>newpos) {
				if(pos>=6)
					pos-=6;
				else
					break;

				((MMSSliderWidget *)wid)->setPosition(pos);
				usleep(5000);
			}
		} else {
			while(pos<newpos) {
				pos+=6;
				slider->setPosition(pos);
				usleep(10000);
			}
		}
		if(newpos!=pos)
			((MMSSliderWidget *)wid)->setPosition(newpos);
	} else if(wid->getType() == MMSWIDGETTYPE_PROGRESSBAR){
		unsigned int pos = ((MMSProgressBarWidget*)wid)->getProgress();
		unsigned int newpos = (x*100)/((MMSProgressBarWidget*)wid)->getGeometry().w;
		if(newpos < pos) {
			while(pos>newpos) {
				if(pos>=6)
					pos-=6;
				else
					break;

				((MMSProgressBarWidget*)wid)->setProgress(pos);
				usleep(5000);
			}
		} else {
			while(pos<newpos) {
				pos+=6;
				((MMSProgressBarWidget*)wid)->setProgress(pos);
				usleep(10000);
			}
		}
		if(newpos!=pos)
			((MMSProgressBarWidget*)wid)->setProgress(newpos);
	}
}

void Buttons::onRadio(MMSWidget *wid) {
	MMSCheckBoxWidget *check = (MMSCheckBoxWidget *)wid;

	for(vector<MMSCheckBoxWidget *>::iterator it = radio.begin(); it!= radio.end();it++) {
		if(*it == check) {
			(*it)->setChecked(true);
		} else {
			(*it)->setChecked(false);
		}
	}
}

bool Buttons::onAnim(MMSPulser *pulser) {

	if(stopAnim)
		return false;

	int to;
	if(direction < 0) {
		to = this->animstarty -(int)pulser->getOffset();
	} else {
		to = this->animstarty +(int)pulser->getOffset();
	}
	if(to > 0) {
		to = 0;
		this->contents->moveTo(0,to);
		return false;
	} else if (to < this->maxy) {
		to = maxy;
		this->contents->moveTo(0,to);
		return false;
	}

	this->contents->moveTo(0,to);

	return true;
}
