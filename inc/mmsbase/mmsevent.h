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

#ifndef MMSEVENT_H_
#define MMSEVENT_H_

#include "mmsbase/interfaces/immseventdispatcher.h"
#include <map>

/**
 * @file mmsevent.h
 *
 * Header file for MMSEvent class.
 *
 * @ingroup mmsbase
 */

/**
 * Implementation of communication by using events.
 *
 * It is possible to create events by defining a heading
 * used to identify received events and additional parameters
 * by using key-value-pairs.
 *
 * @note This class implements the _IMMSEvent interface.
 */
class MMSEvent : public _IMMSEvent {
    public:
		/**
		 * Constructor.
		 *
		 * @param	heading		identifier for event
		 */
        MMSEvent(string heading = "noreceiver");

        /**
         * Setter method to set heading to identify the created
         * event.
         *
         * @param	heading		identifier for event
         */
        virtual void setHeading(string heading);

        /**
         * Getter method to fetch heading to identify the event.
         *
         * @returns identifier of event
         */
        virtual string getHeading();

        /**
         * Get additional event parameters.
         *
         * Parameters consist of key-value-pairs which are both
         * string objects.
         *
         * @param	key		key to get the value for
         *
         * @returns	value for given key
         */
        virtual string getData(string key);

        /**
         * Set additional event parameters.
         *
         * Parameters consist of key-value-pairs which are both
         * string objects.
         *
         * @param	key		key to add
         * @param	value	value for given key
         */
        virtual void setData(string key, string value);

        /**
         * Clear given event parameters.
         *
         * All key-value-pairs set before are deleted.
         */
        virtual void clear();

        /**
         * Send event.
         *
         * The event will be raised by the dispatcher set in
         * setDispatcher().
         *
         * @note	It is not possible to send events from inside
         * 			the method which receives events, since the
         * 			dispatcher will block.
         */
        virtual void send();

        /**
         * Set dispatcher interface to raise events.
         *
         * @param	dispatcher	dispatcher interface
         */
        void setDispatcher(IMMSEventDispatcher *dispatcher);

        /**
         * Send event to given plugin.
         *
         * The event will be raised by the dispatcher set in
         * setDispatcher().
         *
         * @param	pluginid	id of plugin to send event to
         *
         * @note	It is not possible to send events from inside
         * 			the method which receives events, since the
         * 			dispatcher will block.
         */
        void sendTo(int pluginid);

    private:
        static IMMSEventDispatcher *dispatcher;		/**< dispatcher interface used for raising events */
        string heading;								/**< identifier for event */
        std::map<string,string> data;				/**< additional key-value-pairs as event parameters */
};

#endif /*MMSEVENT_H_*/
