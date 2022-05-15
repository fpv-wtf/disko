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

#include "mmstools/mmsthread.h"
#include "mmstools/mmserror.h"
#include "mmstools/tools.h"

#ifdef __HAVE_DIRECTFB__
extern "C" {
#include <direct/debug.h>
#include <direct/thread.h>
#include <direct/trace.h>
}

D_DEBUG_DOMAIN( MMS_Thread, "MMS/Thread", "MMS Thread" );

#endif /* __HAVE_DIRECTFB__ */

static void *startmythread(void *thiz) {
	static_cast<MMSThread *>(thiz)->run();
	return NULL;
}

MMSThread::MMSThread(string identity, int priority, bool detach) {
#ifdef __HAVE_DIRECTFB__
    D_DEBUG_AT( MMS_Thread, "MMSThread( %s )\n", identity.c_str() );

    direct_trace_print_stack(NULL);
#endif /* __HAVE_DIRECTFB__ */

    this->identity = identity;
    this->priority = priority;

    this->isrunning = false;
    this->isdetached = false;
    this->autoDetach = detach;
    this->stacksize = 1000000;
}

void MMSThread::run() {
	try {
#ifdef __HAVE_DIRECTFB__
        direct_thread_set_name( this->identity.c_str() );
#endif /* __HAVE_DIRECTFB__ */
        if(this->autoDetach) {
        	this->detach();
        }
        this->isrunning = true;
		threadMain();
        this->isrunning = false;

	} catch(MMSError *error) {
        this->isrunning = false;
	    DEBUGMSG(this->identity.c_str(), "Abort due to: %s", error->getMessage().c_str());
	}
}

void MMSThread::start() {
	if (this->isrunning)
		return;

	/* initialize the priority */
    pthread_attr_init(&this->tattr);
    pthread_attr_getschedparam(&tattr, &param);
    param.sched_priority = this->priority;
    pthread_attr_setschedparam(&tattr, &param);
    pthread_attr_setstacksize(&tattr, this->stacksize);

    /* create the new thread */
    pthread_create(&this->id, &tattr, startmythread, static_cast<void *>(this));
}

void MMSThread::detach() {
	pthread_detach(this->id);
	this->isdetached = true;
}

bool MMSThread::isRunning() {
	return this->isrunning;
}

int MMSThread::cancel() {
	int status;
	if(this->isrunning)
		status = pthread_cancel(this->id);
	this->isrunning=false;
	return status;

}

void MMSThread::join() {
    if (!this->isdetached)
        pthread_join(this->id, NULL);
}

void MMSThread::setStacksize(size_t stacksize) {
	this->stacksize = stacksize;
}
