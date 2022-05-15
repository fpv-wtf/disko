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

#include "mmsinput/mmsinputmanager.h"

MMSInputManager::MMSInputManager(string file, string name) {
	this->mapper = new MMSInputMapper(file, name);
	this->config = new MMSConfigData();
	this->buttonpress_window = NULL;
	this->button_pressed = false;
}

MMSInputManager::~MMSInputManager() {
    this->threads.clear();
    this->subscriptions.clear();
    if(this->mapper) delete this->mapper;
    if(this->config) delete this->config;
}

void MMSInputManager::handleInput(MMSInputEvent *inputevent) {
	MMSWindow *window=NULL;
	vector<MMSInputEvent> inputeventset;

	this->mutex.lock();

	if (inputevent->type == MMSINPUTEVENTTYPE_KEYPRESS) {
		/* keyboard inputs */

		/* check crtl+c and exit */
		if((inputevent->key==MMSKEY_SMALL_C)&&(this->lastkey==MMSKEY_CONTROL))
			exit(1);

		this->lastkey = inputevent->key;

		this->mapper->mapkey(inputevent, &inputeventset);

#if __ENABLE_LOG__ || __ENABLE_DEBUG__
		string symbol = mmskeys[inputevent->key];
		TRACEOUT("MMSINPUT", "KEY PRESS %d (MMSKEY_%s)", this->lastkey, symbol.c_str());
		for (int i = 0; i < (int)inputeventset.size(); i++) {
			symbol = mmskeys[inputeventset.at(i).key];
			TRACEOUT("MMSINPUT", " >MAPPED TO %d (MMSKEY_%s)", inputeventset.at(i).key, symbol.c_str());
		}
#endif

		if((inputeventset.at(0).key==MMSKEY_POWER)||(inputeventset.at(0).key==MMSKEY_POWER2)) {
			if(config->getShutdown() == true) {
				DEBUGMSG("MMSINPUTMANAGER", "executing: %s", config->getShutdownCmd().c_str());

				executeCmd(config->getShutdownCmd());
				sleep(30);
			}
			exit(0);
		}

		window = this->windowmanager->getToplevelWindow();

		if(window!=NULL) {
			/* we have a window -> lets see if there are navigation keys */
			for(unsigned y=0;y<inputeventset.size();y++) {
				if	((inputeventset.at(y).key==MMSKEY_CURSOR_DOWN)||(inputeventset.at(y).key==MMSKEY_CURSOR_UP)
					||(inputeventset.at(y).key==MMSKEY_CURSOR_LEFT)||(inputeventset.at(y).key==MMSKEY_CURSOR_RIGHT)) {
					/* ok execute input on window */
					window->handleInput(&inputeventset);
					this->mutex.unlock();
					return;
				}
			}
		}

		// have to call subscriptions?
		bool call_subscriptions = true;
		if (window) {
			bool modal = false;
			window->getModal(modal);
			if (modal)
				call_subscriptions = false;
		}

		if (call_subscriptions) {
			// go through subscriptions
			for(unsigned int i = 0; i < subscriptions.size();i++) {
				for(unsigned int y = 0; y < inputeventset.size(); y++) {
					MMSKeySymbol key;
					if (subscriptions.at(i)->getKey(key))
						if (key == inputeventset.at(y).key) {
							DEBUGMSG("MMSINPUTMANAGER", "found a subscription");
							// ok i found one execute
							subscriptions.at(i)->callback.emit(subscriptions.at(i));
							// stop it only one key per subscription
							DEBUGMSG("MMSINPUTMANAGER", "returning from handle input");
							this->mutex.unlock();
							return;
						}
				}
			}
		}

		if(window != NULL)
			window->handleInput(&inputeventset);
	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_KEYRELEASE) {
		/* keyboard inputs */
		this->mapper->mapkey(inputevent, &inputeventset);

#if __ENABLE_LOG__ || __ENABLE_DEBUG__
		string symbol = mmskeys[inputevent->key];
		TRACEOUT("MMSINPUT", "KEY RELEASE %d (MMSKEY_%s)", this->lastkey, symbol.c_str());
		for (int i = 0; i < (int)inputeventset.size(); i++) {
			symbol = mmskeys[inputeventset.at(i).key];
			TRACEOUT("MMSINPUT", " >MAPPED TO %d (MMSKEY_%s)", inputeventset.at(i).key, symbol.c_str());
		}
#endif

		window = this->windowmanager->getToplevelWindow();

		if(window != NULL)
			window->handleInput(&inputeventset);
	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_BUTTONPRESS) {
		DEBUGMSG("MMSINPUTMANAGER", "MMSInputManager:handleInput: BUTTON PRESSED AT: %d,%d", inputevent->posx, inputevent->posy);

		this->windowmanager->setPointerPosition(inputevent->posx, inputevent->posy, true);
		this->button_pressed = true;

		window = this->windowmanager->getToplevelWindow();
		if (window) {
			/* get the window rect and check if the pointer is in there */
			MMSFBRectangle rect = window->getGeometry();

			if ((inputevent->posx - rect.x < 0)||(inputevent->posy - rect.y < 0)
					||(inputevent->posx - rect.x - rect.w >= 0)||(inputevent->posy - rect.y - rect.h >= 0)) {
				/* pointer is not over the window */
				DEBUGMSG("MMSINPUTMANAGER", "MMSInputManager:handleInput: BUTTON PRESSED, NOT OVER THE WINDOW");
				this->mutex.unlock();
				return;
			}

			// save the pointer for release event
			this->buttonpress_window = window;

			/* call windows handleInput with normalized coordinates */
			MMSInputEvent ie;
			ie.type = MMSINPUTEVENTTYPE_BUTTONPRESS;
			ie.posx = inputevent->posx - rect.x;
			ie.posy = inputevent->posy - rect.y;
			inputeventset.push_back(ie);
			window->handleInput(&inputeventset);
		}
	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_BUTTONRELEASE) {
		DEBUGMSG("MMSINPUTMANAGER", "MMSInputManager:handleInput: BUTTON RELEASED AT: %d,%d", inputevent->posx, inputevent->posy);

		this->windowmanager->setPointerPosition(inputevent->posx, inputevent->posy, false);
		this->button_pressed = false;

		window = this->windowmanager->getToplevelWindow();
		if (!window)
			window = this->buttonpress_window;
		if (window) {
			/* get the window rect and check if the pointer is in there */
			MMSFBRectangle rect = window->getGeometry();

			if ((window == this->buttonpress_window)
				||   ((this->buttonpress_window)
					&&(inputevent->posx - rect.x >= 0)&&(inputevent->posy - rect.y >= 0)
					&& (inputevent->posx - rect.x - rect.w < 0)&&(inputevent->posy - rect.y - rect.h < 0))) {
				/* call windows handleInput with normalized coordinates */
				MMSInputEvent ie;
				ie.type = MMSINPUTEVENTTYPE_BUTTONRELEASE;
				ie.posx = inputevent->posx - rect.x;
				ie.posy = inputevent->posy - rect.y;
				inputeventset.push_back(ie);
				if (window->handleInput(&inputeventset)) {
					this->buttonpress_window = NULL;
					this->mutex.unlock();
					return;
				}
			}
		}
		this->buttonpress_window = NULL;


		// have to call subscriptions?
		bool call_subscriptions = true;
		if (window) {
			bool modal = false;
			window->getModal(modal);
			if (modal)
				call_subscriptions = false;
		}

		if (call_subscriptions) {
			// go through subscriptions
			for(unsigned int i = 0; i < subscriptions.size();i++) {
				for(unsigned int y = 0; y < inputeventset.size(); y++) {
					MMSFBRectangle pointer_area;
					if (subscriptions.at(i)->getPointerArea(pointer_area))
						if ((inputevent->posx >= pointer_area.x)&&(inputevent->posy >= pointer_area.y)
						  &&(inputevent->posx < pointer_area.x + pointer_area.w)&&(inputevent->posy < pointer_area.y + pointer_area.h)) {
							DEBUGMSG("MMSINPUTMANAGER", "found a subscription");
							// ok i found one execute
							subscriptions.at(i)->callback.emit(subscriptions.at(i));
							// stop it only one key per subscription
							DEBUGMSG("MMSINPUTMANAGER", "returning from handle input");
							this->mutex.unlock();
							return;
						}
				}
			}
		}

	}
	else
	if (inputevent->type == MMSINPUTEVENTTYPE_AXISMOTION) {

		this->windowmanager->setPointerPosition(inputevent->posx, inputevent->posy, this->button_pressed);


		window = this->windowmanager->getToplevelWindow();
		if (window) {
			/* get the window rect and check if the pointer is in there */
			MMSFBRectangle rect = window->getGeometry();

			if ((inputevent->posx - rect.x < 0)||(inputevent->posy - rect.y < 0)
					||(inputevent->posx - rect.x - rect.w >= 0)||(inputevent->posy - rect.y - rect.h >= 0)) {
				/* pointer is not over the window */
				this->mutex.unlock();
				return;
			}

			/* call windows handleInput with normalized coordinates */
			MMSInputEvent ie;
			ie.type = MMSINPUTEVENTTYPE_AXISMOTION;
			ie.posx = inputevent->posx - rect.x;
			ie.posy = inputevent->posy - rect.y;
			inputeventset.push_back(ie);
			window->handleInput(&inputeventset);
		}
	}

	this->mutex.unlock();
}

void MMSInputManager::addDevice(MMS_INPUT_DEVICE device, int inputinterval) {
	MMSInputThread *thread = new MMSInputThread(this, device, inputinterval);

	this->threads.push_back(thread);

}

void MMSInputManager::setWindowManager(IMMSWindowManager *wm) {
	this->windowmanager = wm;

}

void MMSInputManager::startListen() {
	for(unsigned int i=0; i<this->threads.size();i++) {
		this->threads.at(i)->start();
	}
}

void MMSInputManager::stopListen() {
	for(unsigned int i=0; i<this->threads.size();i++) {
		this->threads.at(i)->cancel();
	}
}


void MMSInputManager::addSubscription(class MMSInputSubscription *sub)  {
	this->subscriptions.push_back(sub);
}
