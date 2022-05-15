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

#include "mmscore/mmseventsignupmanager.h"
#include "mmstools/tools.h"
#include <string.h>
#include <algorithm>

MMSEventSignupManager::MMSEventSignupManager() {
}

void MMSEventSignupManager::signup(IMMSEventSignup *signup) {
    this->signupmutex.lock();
    if(signup->isPlugin())
    	this->signups.push_back((MMSEventSignup *)signup);
    else
    	this->signals.push_back((MMSEventSignup *)signup);

    this->signupmutex.unlock();
}

vector<MMSPluginData *> MMSEventSignupManager::getReceiverPlugins(_IMMSEvent *event) {
    vector<MMSPluginData *> mydata;

    vector<string> subscriptions;
    vector<MMSEventSignup *>::iterator signit;

    string heading = event->getHeading();

    for(signit = this->signups.begin();signit != this->signups.end();signit++) {
    	(*signit)->lock();
    	subscriptions = (*signit)->getSubScriptions();

    	for(vector<string>::iterator it = subscriptions.begin(); it != subscriptions.end(); ++it) {
    		//DEBUGMSG("MMSEventSignupManager", "comparing %s - %s - %d\n", (*it).c_str(), heading.c_str(), (*it).size());
    		if(heading.compare(0, (*it).size(), *it) == 0) {
				MMSPluginData *myplugin = new MMSPluginData;
				*myplugin = (*signit)->getPluginData();
				mydata.push_back(myplugin);
    		}
    	}
    	(*signit)->unlock();
    }

    if(mydata.empty())
        throw MMSEventSignupManagerError(0,"no subscriptions found for " + heading);

    return mydata;
}

vector<sigc::signal<void, _IMMSEvent*> *> MMSEventSignupManager::getReceiverSignals(_IMMSEvent *event) {
	vector<sigc::signal<void, _IMMSEvent*> *> mysignals;
    vector<string> subscriptions;
    vector<MMSEventSignup *>::iterator signupsit;

    string heading = event->getHeading();

    for(signupsit= this->signals.begin();signupsit != this->signals.end();++signupsit) {
    	(*signupsit)->lock();
    	subscriptions = (*signupsit)->getSubScriptions();

    	for(vector<string>::iterator it = subscriptions.begin(); it != subscriptions.end(); ++it) {
    		//DEBUGMSG("MMSEventSignupManager", "comparing %s - %s - %d\n", (*it).c_str(), heading.c_str(), (*it).size());
    		if(heading.compare(0, (*it).size(), *it) == 0) {
				mysignals.push_back((*signupsit)->getSignal());
    		}
    	}
    	(*signupsit)->unlock();
    }

	if(mysignals.empty())
        throw MMSEventSignupManagerError(0,"no subscriptions found");

    return mysignals;

}
