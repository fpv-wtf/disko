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

#include "mmstools/mmstimer.h"
#include <cerrno>
#include <cstring>

extern "C" {
#include <sys/time.h>
#include <time.h>
}

MMSTimer::MMSTimer(bool singleShot) :
	MMSThread("MMSTimer"), repeat(false), quit(false), secs(0),
			nSecs(0), threadID(0)
{
	this->singleShot = singleShot;

	MMSThread::setStacksize(131072);

	pthread_mutex_init(&interruptMutex, NULL);
	pthread_mutex_init(&startMutex, NULL);
	pthread_mutex_init(&stopMutex, NULL);

	pthread_cond_init(&interruptCond, NULL);
	pthread_cond_init(&startCond, NULL);
	pthread_cond_init(&stopCond, NULL);
}

MMSTimer::~MMSTimer()
{
	quit = true;
	stop();
	join();

	pthread_mutex_destroy(&interruptMutex);
	pthread_mutex_destroy(&startMutex);
	pthread_mutex_destroy(&stopMutex);

	pthread_cond_destroy(&interruptCond);
	pthread_cond_destroy(&startCond);
	pthread_cond_destroy(&stopCond);
}

void MMSTimer::start(unsigned int milliSeconds)
{
	if(isRunning()) {
		return;
	}

	nSecs = (milliSeconds % 1000) * 1000000;
	secs = milliSeconds / 1000;

	repeat = true;
	MMSThread::start();
}

void MMSTimer::restart()
{
	/* see comment in stop() */
	if(!isRunning() || pthread_self() == threadID) {
		return;
	}

	stopWithoutCheck();

	pthread_mutex_lock(&interruptMutex);
	repeat = true;
	pthread_mutex_unlock(&interruptMutex);


	/* signal to outer thread loop to restart */
	pthread_mutex_lock(&startMutex);
	pthread_cond_signal(&startCond);
	pthread_mutex_unlock(&startMutex);
}

void MMSTimer::stop()
{
	if(!isRunning()) {
		return;
	}

	/*
	 * This happens if timeOut.emit() is called in threadMain and its
	 * signal handler contains a call of this function. It causes
	 * deadlocks so we have to return here.
	 */
	if(pthread_self() == threadID) {
		return;
	}

	stopWithoutCheck();
}

void MMSTimer::stopWithoutCheck()
{
	/* signal to inner thread loop to stop waiting for timeout */
	pthread_mutex_lock(&interruptMutex);
	pthread_cond_signal(&interruptCond);
	pthread_mutex_unlock(&interruptMutex);


	/* make sure inner thread loop is left */
	pthread_mutex_lock(&stopMutex);
	/* TODO: Check what happens on compiler optimizations. */
	//printf("stopped\n");
	pthread_mutex_unlock(&stopMutex);
}

void MMSTimer::threadMain()
{
	threadID = pthread_self();
	if(secs == 0 && nSecs == 0) {
		return;
	}

	struct timespec absTime;
	struct timeval  absTimeGet;
	int rc = 0;
	while(!quit) {
		pthread_mutex_lock(&stopMutex);
		while(repeat) {
			gettimeofday(&absTimeGet, NULL);
			absTime.tv_sec  = absTimeGet.tv_sec + secs;
			absTime.tv_nsec = (absTimeGet.tv_usec * 1000) + nSecs;

			if(absTime.tv_nsec > 999999999) {
				absTime.tv_sec += 1;
				absTime.tv_nsec -= 999999999;
			}

			pthread_mutex_lock(&interruptMutex);
			rc = pthread_cond_timedwait(&interruptCond,
					&interruptMutex, &absTime);

			if(rc == ETIMEDOUT) {
				//printf("timer: emit\n");
				timeOut.emit();
			} else if(rc == 0) {
				//printf("timer: cond_signal\n");
				repeat = false;
			} else {
				//printf("timer: error: %d %s\n", rc, strerror(rc));
			}

			if(singleShot) {
				repeat = false;
			}
			pthread_mutex_unlock(&interruptMutex);
		}
		pthread_mutex_unlock(&stopMutex);

		if(quit) {
			break;
		}

		/* wait for a restart request */
		pthread_mutex_lock(&startMutex);
		pthread_cond_wait(&startCond, &startMutex);
		pthread_mutex_unlock(&startMutex);
	}
}
