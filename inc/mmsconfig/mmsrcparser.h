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

#ifndef MMSMORPHINERCPARSER_H_
#define MMSMORPHINERCPARSER_H_

#include "mmstools/mmstools.h"
#include "mmsconfig/mmsconfigdata.h"
#include <libxml/parser.h>
#include <libxml/tree.h>


class MMSRcParser {
	private:
		typedef enum {
			THROUGH_GRAPHICS_MODE_NORMAL = 0,
			THROUGH_GRAPHICS_MODE_VIDEOLAYER,
			THROUGH_GRAPHICS_MODE_GRAPHICSLAYER
		} THROUGH_GRAPHICS_MODE;

		MMSConfigDataGlobal		global;
    	MMSConfigDataDB     	configdb, datadb;
    	MMSConfigDataGraphics	graphics;
    	MMSConfigDataLanguage	language;

    	void checkVersion(xmlNode* node);
    	void throughGlobal(xmlNode* node);
    	void throughDBSettings(xmlNode* node);
    	void throughGraphics(xmlNode* node, THROUGH_GRAPHICS_MODE mode = THROUGH_GRAPHICS_MODE_NORMAL);
    	void throughLanguage(xmlNode* node);
    	void throughFile(xmlNode* node);

    	/* helper */

	public:
		MMSRcParser();
		~MMSRcParser();

		void parseFile(string filename);
		void getMMSRc(MMSConfigDataGlobal 	**global,
			          MMSConfigDataDB     	**configdb,
			          MMSConfigDataDB     	**datadb,
			          MMSConfigDataGraphics	**graphics,
			          MMSConfigDataLanguage **language);
};

MMS_CREATEERROR(MMSRcParserError);

#endif /*MMSMORPHINERCPARSER_H_*/
