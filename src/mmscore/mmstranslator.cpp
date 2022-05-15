/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2010 BerLinux Solutions GmbH                       *
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
#include <stdexcept>

string MMSTranslator::source = "";
string MMSTranslator::target = "";
int MMSTranslator::sourceIdx = -1;
int MMSTranslator::targetIdx = -1;
MMSTRANSLATION_INDEX MMSTranslator::transIdx;
MMSTRANSLATION_MAP MMSTranslator::transmap;
MMSTRANSLATION_FILES MMSTranslator::files;

sigc::signal<void, unsigned int>  MMSTranslator::onTargetLangChanged;
bool MMSTranslator::addtranslations;

MMSTranslator::MMSTranslator() {
	if(this->source.empty()) {
		MMSConfigData config;
		this->source = config.getSourceLang();
		this->target = config.getDefaultTargetLang();
		this->addtranslations = config.getAddTranslations();
	}

	if(!this->source.empty() && !this->target.empty())
		loadTranslations();
}

MMSTranslator::~MMSTranslator() {

}

void MMSTranslator::loadTranslations() {
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
			MMSFileSearch search((*it)->getPath(),"translation.??",false);
			MMSFILEENTRY_LIST ret =  search.execute();
			for(MMSFILEENTRY_LIST::iterator it2 = ret.begin(); it2 != ret.end();it2++) {
				string filename = (*it)->getPath() + "/" + basename((*it2)->name.c_str());
				processFile(filename);
			}
		}
	} catch (MMSError *err) {
		DEBUGMSG("MMSTranslator", "No plugins database found for translation.");
	}

	MMSFileSearch search(config.getLanguagefileDir(),"translation.??",false);
	MMSFILEENTRY_LIST ret =  search.execute();
	for(MMSFILEENTRY_LIST::iterator it2 = ret.begin(); it2 != ret.end();it2++) {
		processFile((*it2)->name);
	}

	this->sourceIdx = this->transIdx.find(this->source)->second;
	this->targetIdx = this->transIdx.find(this->target)->second;
}

void MMSTranslator::addMissing(const string &phrase, const bool completemiss) {
	size_t size = this->files.size();

	if(completemiss) {
		//add to all language files;
		for(unsigned int idx = 0; idx < size; ++idx) {
			MMSFile file(this->files.at(idx), MMSFM_APPEND, false);
			char line[1024];
			snprintf(line, sizeof(line) - 4, "%s===\n", phrase.c_str());
			file.writeBuffer(line, NULL, strlen(line), 1);

			MMSTRANSLATION_MAP::iterator transit = this->transmap.find(phrase);
			if(transit != this->transmap.end()) {
				transit->second.at(idx) = phrase;
			} else {
				vector<string> trans(this->files.size());
				trans.at(idx) = phrase;
				transmap.insert(make_pair(phrase, trans));
			}
		}
	} else {
		//check the single languages...
		MMSTRANSLATION_MAP::iterator transit = this->transmap.find(phrase);
		for(unsigned int idx = 0; idx < size; ++idx) {
			if(transit->second.at(idx).empty()) {
				MMSFile file(this->files.at(idx), MMSFM_APPEND, false);
				char line[1024];
				snprintf(line, sizeof(line) - 4, "%s===\n", phrase.c_str());
				file.writeBuffer(line, NULL, strlen(line), 1);
				transit->second.at(idx) = phrase;
			}
		}
	}
}


void MMSTranslator::translate(const string &source, string &dest) {
	if(this->targetIdx == -1) {
		dest = source;
		return;
	}

	MMSTRANSLATION_MAP::iterator it = this->transmap.find(source);
	if(it == this->transmap.end()) {
		dest = source;
		if(this->addtranslations) {
			addMissing(source, true);
		}
	} else {
		dest = it->second.at(this->targetIdx);
		if(dest.empty()) {
			dest = source;
			if(this->addtranslations) {
				addMissing(source);
			}
		}
	}
}

bool MMSTranslator::setTargetLang(const string &countryCode) {
	MMSTRANSLATION_INDEX::iterator it = this->transIdx.find(countryCode);
	if(it == this->transIdx.end())
		return false;

	this->targetIdx = it->second;
	this->onTargetLangChanged.emit(this->targetIdx);

	return true;
}

void MMSTranslator::processFile(const string &file) {
	MMSFile transfile(file,MMSFM_READ,false);
	string line;
	string from, to;
	size_t idx;
	string countryCode = file.substr(file.find("translation")+12);

	MMSTRANSLATION_INDEX::iterator it = this->transIdx.find(countryCode);
	if(it == this->transIdx.end()) {
		idx = this->files.size();
		this->transIdx[countryCode] = idx;
		this->files.push_back(file);
		for(MMSTRANSLATION_MAP::iterator it = this->transmap.begin(); it != this->transmap.end(); ++it) {
			it->second.resize(idx + 1, "");
		}
	} else {
		idx = it->second;
	}

	while(transfile.getLine(line)) {
		size_t pos = line.find("===");
		if(pos != string::npos) {
			from = line.substr(0, pos);
			to = line.substr(pos+3, string::npos);

			MMSTRANSLATION_MAP::iterator f = this->transmap.find(from);
			if(f != this->transmap.end()) {
				//already have the source
				DEBUGMSG("MMSTranslator", "insert: '%s'", from.c_str());
				f->second.at(idx) = to;
			} else {
				DEBUGMSG("MMSTranslator", "fresh insert: '%s'", from.c_str());
				vector<string> trans(idx + 1);
				trans.at(idx) = to;
				transmap.insert(make_pair(from, trans));
			}
		}
	}
}
