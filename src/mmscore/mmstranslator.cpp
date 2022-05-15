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

#include "mmscore/mmstranslator.h"
#include "mmsconfig/mmsconfigdata.h"
#include "mmsconfig/mmspluginservice.h"
#include "mmstools/mmsfile.h"

#include <string.h>

MMS_LANGUAGE_TYPE MMSTranslator::sourcelang = MMSLANG_UKN;
MMS_LANGUAGE_TYPE MMSTranslator::targetlang = MMSLANG_UKN;
MMSTRANSLATION_MAP MMSTranslator::transmap;
MMSTRANSLATION_FILES MMSTranslator::files;

sigc::signal<void, MMS_LANGUAGE_TYPE>  MMSTranslator::onTargetLangChanged;
bool MMSTranslator::addtranslations;

MMSTranslator::MMSTranslator() {
	if(this->sourcelang == MMSLANG_UKN) {
		MMSConfigData config;
		this->sourcelang = config.getSourceLang();
		this->targetlang = config.getDefaultTargetLang();
		this->addtranslations = config.getAddTranslations();
		if(sourcelang != MMSLANG_UKN && targetlang != MMSLANG_UKN)
			loadTransLations();
	}
}

MMSTranslator::~MMSTranslator() {

}

void MMSTranslator::loadTransLations() {
	MMSConfigData config;
	DataSource source(config.getConfigDBDBMS(),
					  config.getConfigDBDatabase(),
					  config.getConfigDBAddress(),
					  config.getConfigDBPort(),
					  config.getConfigDBUser(),
					  config.getConfigDBPassword());

	try {
		MMSPluginService service(&source);
		vector<MMSPluginData *> data = service.getAllPlugins();

		for(vector<MMSPluginData *>::iterator it = data.begin();it!=data.end();it++) {
			MMSFileSearch search((*it)->getPath(),"translation.*",false);
			MMSFILEENTRY_LIST ret =  search.execute();
			for(MMSFILEENTRY_LIST::iterator it2 = ret.begin(); it2 != ret.end();it2++) {
				string filename = config.getLanguagefileDir() + "/" + basename((*it2)->name.c_str());
				processFile(filename);
			}
		}
	} catch (MMSError *err) {
		DEBUGMSG("MMSTranslator", "No plugins database found for translation.");
	}

	MMSFileSearch search(config.getLanguagefileDir(),"translation.*",false);
	MMSFILEENTRY_LIST ret =  search.execute();
	for(MMSFILEENTRY_LIST::iterator it2 = ret.begin(); it2 != ret.end();it2++) {
		string filename = config.getLanguagefileDir() + "/" + basename((*it2)->name.c_str());
		this->files.insert(make_pair(strToLang(filename.substr(filename.find("translation")+12).c_str()),filename));
		processFile(filename);
	}
}

void MMSTranslator::addMissing(string &phrase, bool completemiss) {
	if(completemiss) {
		//add to all language files;
		for(MMSTRANSLATION_FILES::iterator it = this->files.begin(); it != this->files.end();it++) {
			MMSFile file(it->second,MMSFM_APPEND,false);
			char line[1024];
			sprintf(line,"%s===\n", phrase.c_str());

			file.writeBuffer(line,NULL,strlen(line),1);

			MMSTRANSLATION_MAP::iterator transit =  this->transmap.find(phrase);
			if(transit != this->transmap.end()) {
				transit->second.insert(make_pair(it->first,""));
			} else{
				map<MMS_LANGUAGE_TYPE, string> mymap;
				mymap.insert(make_pair(it->first,""));
				transmap.insert(make_pair(phrase, mymap));

			}
		}

	} else {
		//check the single languages...
		MMSTRANSLATION_MAP::iterator transit =  this->transmap.find(phrase);
		if(transit == transmap.end()) {
			//big big problem here
		} else {
			for(MMSTRANSLATION_FILES::iterator it = this->files.begin(); it != this->files.end();it++) {
				if(transit->second.find(it->first) == transit->second.end()) {
					MMSFile file(it->second,MMSFM_APPEND,false);
					char line[1024];
					sprintf(line,"%s===\n", phrase.c_str());
					file.writeBuffer(line,NULL,strlen(line),1);
					transit->second.insert(make_pair(it->first,""));
				}
			}

		}
	}
}


void MMSTranslator::translate(string &source, string &dest) {
	if(targetlang != MMSLANG_UKN) {
		MMSTRANSLATION_MAP::iterator it = transmap.find(source);
		map<MMS_LANGUAGE_TYPE, string>::iterator it2;

		if(it!=transmap.end()) {
			it2 = it->second.find(targetlang);
			if(it2 != it->second.end()) {
				dest = it2->second;
				if(dest.empty()) {
					dest = source;
				}
			} else {
				if(addtranslations) {
					addMissing(source);
					dest = source;
				}
			}
		} else {
			// no translation file contains the source
			if(addtranslations) {
				addMissing(source,true);
				dest = source;
			}
		}
	} else {
		dest = source;
	}
}

void MMSTranslator::setTargetLang(MMS_LANGUAGE_TYPE lang) {
	this->targetlang = lang;
	this->onTargetLangChanged.emit(lang);
}

void MMSTranslator::processFile(string &file) {
	MMSFile transfile(file,MMSFM_READ,false);
	string line;
	string from, to;
	MMS_LANGUAGE_TYPE lang =  strToLang(file.substr(file.find("translation")+12).c_str());

	while(transfile.getLine(line)) {
		size_t pos = line.find("===");
		if(pos != string::npos) {
			from = line.substr(0, pos);
			to = line.substr(pos+3, string::npos);

			MMSTRANSLATION_MAP::iterator f = transmap.find(from);
			if(f != transmap.end()) {
				//already have the source
				DEBUGMSG("MMSTranslator", "insert: '%s'", from.c_str());
				pair<map<MMS_LANGUAGE_TYPE, string>::iterator, bool > ret = f->second.insert(make_pair(lang,to));
				if(ret.second == false) {
					DEBUGMSG("MMSTranslator", "double tranlation for: '%s'", from.c_str());
				}
			} else {
				DEBUGMSG("MMSTranslator", "fresh insert: '%s'", from.c_str());
				map<MMS_LANGUAGE_TYPE, string> mymap;
				mymap.insert(make_pair(lang,to));
				transmap.insert(make_pair(from, mymap));
			}
		}
	}
}
