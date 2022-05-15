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

#include "oskcontrol.h"

OskControl::OskControl(MMSDialogManager &dm, string layout, bool complex, string name) {
	if(!name.empty()) {
		this->controlname = name;
	} else {
		controlname = "OskControl";
	}

	dm.loadChildDialog(config.getData() + "/oskcontrol.xml");
	MMSWindow *win = dm.getWindow();

	this->controlwin = win->findWindow("clipping_window");
	this->controlwin->setName(this->controlname);

	this->label = controlwin->findWindow("input_window");
	this->osk_de = controlwin->findWindow("oskcontrol_de");
	this->osk_ru = controlwin->findWindow("oskcontrol_ru");
	this->label->show();
	this->osk_de->show();

	this->complex = complex;
	if(complex) {
		onCharPressedComplex = new sigc::signal<void, COMPLEX_KEY*>;
	} else {
		onCharPressed = new sigc::signal<void, string*>;
	}

	modifier = MOD_NORMAL;
	findControls(osk_de);
	findControls(osk_ru);
	this->curr_osk = osk_de;
	applyLabels(osk_de);
	applyLabels(osk_ru);

	this->anim_saved_screen = NULL;
    // add animation callbacks
    this->animThread.onBeforeAnimation.connect(sigc::mem_fun(this, &OskControl::onBeforeAnimation));
    this->animThread.onAnimation.connect(sigc::mem_fun(this, &OskControl::onAnimation));

}

OskControl::~OskControl() {

}

void OskControl::parseLabelString(std::string &labelstr, std::string labels[13], int *labelcount,int *keycode) {
	std::string::size_type pos 		= std::string::npos;
	std::string::size_type nextpos 	= std::string::npos;
	std::string::size_type len = std::string::npos;

	//initialize labels;
	labels[MOD_NORMAL] = "normal=";
	labels[MOD_SHIFT] = "shift=";
    labels[MOD_ALTGR] = "altgr=";
    labels[MOD_ACUTE] = "acute=";
    labels[MOD_GRAVE] = "grave=";
    labels[MOD_CIRCUMFLEX] = "circumflex=";
    labels[MOD_TILDE] = "tilde=";
    labels[MOD_SHIFT_ALTGR] = "shift+altgr=";
    labels[MOD_SHIFT_ACUTE] = "shift+acute=";
    labels[MOD_SHIFT_GRAVE] = "shift+grave=";
    labels[MOD_SHIFT_CIRCUMFLEX] = "shift+circumflex=";
    labels[MOD_SHIFT_TILDE] = "shift+tilde=";

	//go through
    for(OSK_MODIFIERS i = MOD_NORMAL; i != MOD_MAX; i++) {
		len = labels[i].length();
		pos = labelstr.find(labels[i]);
		if (pos == std::string::npos) {
			throw MMSError(-1, "'" + labels[i] + "' not found in " + labelstr);
			return;
		}
		nextpos = labelstr.find(";",pos);
		if (nextpos == std::string::npos) {
			throw MMSError(-1, "corresponding ; for '" + labels[i] + "' not found in " + labelstr);
			return;
		}

		switch(nextpos-pos-len) {
			case 0:
				//empty or ;
				try{
					if(labelstr.at(pos+len+1)==';')
						labels[i] = ";";
					else
						labels[i] = "";
				} catch(std::exception r) {
					labels[i] = "";
				}
				break;
			case 1:
				//single character
				labels[i] = labelstr.at(pos+len);
				break;
			default:
				//more than one
				labels[i] = labelstr.substr(pos+len,nextpos - pos - len);
				break;
		}
		//get rid of the rest
		labelstr.erase(pos,nextpos-pos+1);
    }

    if(labelcount) {
		//go for labelcount
		pos = labelstr.find("labelcount=");
		if (pos == std::string::npos) {
			DEBUGMSG_OUTSTR("OskControl","'labelcount' not found in %s\n", labelstr.c_str());
			return;
		}
		nextpos = labelstr.find(";",pos);
		if (nextpos == std::string::npos) {
			DEBUGMSG_OUTSTR("OskControl","corresponding ; for 'labelcount=' not found in %s\n", labelstr.c_str());
			return;
		}
		*labelcount = atoi(labelstr.substr(pos+11,nextpos-pos-11).c_str());
		labelstr.erase(pos,nextpos-pos+1);
    }
    if(keycode) {
		//go for keycount
		pos = labelstr.find("keycode=");
		if (pos == std::string::npos) {
			DEBUGMSG_OUTSTR("OskControl","'keycode' not found in %s\n", labelstr.c_str());
			return;
		}
		nextpos = labelstr.find(";",pos);
		if (nextpos == std::string::npos) {
			DEBUGMSG_OUTSTR("OskControl","corresponding ; for 'keycode' not found in %s\n", labelstr.c_str());
			return;
		}
		*keycode= atoi(labelstr.substr(pos+8,nextpos-pos-8).c_str());
		labelstr.erase(pos,nextpos-pos+1);
    }
}

void OskControl::findControls(MMSWindow *oskwin) {
	std::string buttonname;
	std::string labelname;
	std::string data;
	MMSWidget *button;
	MMSLabelWidget *label;
	OSK_LABELMAP labmap;

	for(int i = 1; i<=55;i++) {
		buttonname = "button"  + iToStr(i);
		button = oskwin->findWidget(buttonname);
		if(!button) {
			DEBUGMSG_OUTSTR("OskControl","button %s not found", buttonname.c_str());
		} else {
			if(!button->getData(data)) {
				DEBUGMSG_OUTSTR("OskControl","data is missing for '%s'", buttonname.c_str());
			} else {
				if(data.compare("-1")!=0) {
					//the button has a format string
					try {
						OSK_MODLABELS mod;
						parseLabelString(data,mod.modifierlabels,&mod.labelcount,&mod.keycode);
						this->buttonmap.insert(std::make_pair(button,mod));
						button->onReturn->connect(sigc::mem_fun(this,&OskControl::onReturn));

						//go for labels
						for(int y=0; y<mod.labelcount;y++ ) {
							OSK_MODLABELS lab;
							labelname = "label"  + iToStr(y);
							label = dynamic_cast<MMSLabelWidget*>(button->findWidget(labelname));
							if(label) {
								if(label->getData(data)) {
									parseLabelString(data,lab.modifierlabels);
									labmap.insert(std::make_pair(label,lab));
								}
							}
						}

					} catch(MMSError e) {
						DEBUGMSG_OUTSTR("OskControl","parsing failed: %s", e.getMessage().c_str());
					}
				}
			}
		}
	}
	this->labelmap.insert(std::make_pair(oskwin, labmap));
}

void OskControl::onReturn(MMSWidget* widget) {
	OSK_BUTTONMAP::iterator it = this->buttonmap.find(widget);

	if(it == this->buttonmap.end()) {
		DEBUGMSG("OskPlugin","Pressed Button not found in Buttonmap");
		return;
	}
	string data = it->second.modifierlabels[this->modifier];
	if(data.empty()) {
		//nothing to do
		return;
	}

	// assume that keys not changed
	bool keys_changed = true;

	// data is a modifier?
	OSK_MODIFIERS tmpmod = getModifierFromString(data);
	if(tmpmod != MOD_UKN) {
		if(this->modifier != MOD_NORMAL && this->modifier !=MOD_SHIFT) {
			//add the new mod
			OSK_MODIFIERS tmpmod2 = addModifiers(this->modifier,tmpmod);

			if(this->modifier == tmpmod2) {
				//modifier is the same
				keys_changed = false;
				return;
			} else {
				this->modifier = tmpmod2;
			}
		} else {
			this->modifier = tmpmod;
		}
	} else {

		//this is a single char
		//just report this
		if(this->complex) {
			COMPLEX_KEY *key = new COMPLEX_KEY;
			key->symbol = data;
			key->modifier = this->modifier;
			key->keycode = it->second.keycode;
			this->onCharPressedComplex->emit(key);
		} else {
			string *symbol = new string;
			*symbol = data;
			this->onCharPressed->emit(symbol);
		}

		//reset modifier if necessary
		if(this->modifier != MOD_NORMAL) {
			this->modifier = MOD_NORMAL;
		}
		else {
			keys_changed = false;
		}
	}

	if (keys_changed) {
		applyLabels(this->curr_osk);
	}
}


OSK_MODIFIERS OskControl::getModifierFromString(std::string &mod) {

	if(mod.compare("normal")==0)
		return MOD_NORMAL;
	if(mod.compare("shift")==0)
		return MOD_SHIFT;
	if(mod.compare("altgr")==0)
		return MOD_ALTGR;
	if(mod.compare("acute")==0)
		return MOD_ACUTE;
	if(mod.compare("grave")==0)
		return MOD_GRAVE;
	if(mod.compare("circumflex")==0)
		return MOD_CIRCUMFLEX;
	if(mod.compare("tilde")==0)
		return MOD_TILDE;
	if(mod.compare("shift+altgr")==0)
		return MOD_SHIFT_ALTGR;
	if(mod.compare("shif+acute")==0)
		return MOD_SHIFT_ACUTE;
	if(mod.compare("shift+grave")==0)
		return MOD_SHIFT_GRAVE;
	if(mod.compare("shift+circumflex")==0)
		return MOD_SHIFT_CIRCUMFLEX;
	if(mod.compare("shift+tilde")==0)
		return MOD_SHIFT_TILDE;

	return  MOD_UKN;
}

OSK_MODIFIERS OskControl::addModifiers(OSK_MODIFIERS currentmod, OSK_MODIFIERS newmod) {
	if(currentmod == MOD_SHIFT) {
		switch(newmod) {
			case MOD_ALTGR:
				return MOD_SHIFT_ALTGR;
			case MOD_ACUTE:
				return MOD_SHIFT_ACUTE;
			case MOD_GRAVE:
				return MOD_SHIFT_GRAVE;
			case MOD_CIRCUMFLEX:
				return MOD_SHIFT_TILDE;
			case MOD_TILDE:
				return MOD_SHIFT_TILDE;
		}
	}
	if(newmod == MOD_SHIFT) {
		switch(currentmod) {
			case MOD_ALTGR:
				return MOD_SHIFT_ALTGR;
			case MOD_ACUTE:
				return MOD_SHIFT_ACUTE;
			case MOD_GRAVE:
				return MOD_SHIFT_GRAVE;
			case MOD_CIRCUMFLEX:
				return MOD_SHIFT_CIRCUMFLEX;
			case MOD_TILDE:
				return MOD_SHIFT_TILDE;
		}
	}
	return newmod;
}

void OskControl::applyLabels(MMSWindow *keyboard) {

	// set label text
	for(OSK_LABELMAP::iterator it = this->labelmap.find(keyboard)->second.begin();it!= this->labelmap.find(keyboard)->second.end();it++) {
		it->first->setText(it->second.modifierlabels[this->modifier], false);
	}

	// set clickable status, therefor we have to check if label text is empty
	for (OSK_BUTTONMAP::iterator it = this->buttonmap.begin(); it!= this->buttonmap.end(); it++) {
		string data = it->second.modifierlabels[this->modifier];
		it->first->setClickable((!data.empty()));
	}

	keyboard->refresh();
}


MMSWindow *OskControl::getLabelWindow() {
	return this->label;
}

void OskControl::switchLang() {

	if(!anim_saved_screen) {
		MMSFBLayer *layer = getGraphicsLayer();

		if (layer) {
			MMSFBSurfacePixelFormat pixelformat;
			layer->getPixelFormat(&pixelformat);
			int w, h;
			layer->getResolution(&w, &h);
			layer->createWindow(&anim_saved_screen, 0, 0, w, h,
								pixelformat, isAlphaPixelFormat(pixelformat), 0);
		}
	}
	if (anim_saved_screen) {
		// get a screenshot
		anim_saved_screen->getScreenshot();

		// show the saved screen
		anim_saved_screen->raiseToTop();
		anim_saved_screen->setOpacity(255);
		anim_saved_screen->show();
	}

	if(this->osk_de->isShown()) {
		this->osk_de->setFadeOut(false);
		this->osk_de->hide();
		this->osk_ru->setFadeIn(false);
		this->osk_ru->show();
		this->curr_osk = this->osk_ru;
	} else {
		this->osk_ru->setFadeOut(false);
		this->osk_ru->hide();
		this->osk_de->setFadeIn(false);
		this->osk_de->show();
		this->curr_osk = this->osk_de;
	}


    if (anim_saved_screen) {
    	// do the animation
    	this->animThread.setStepsPerSecond(255);
    	this->animThread.start(false);

    	// delete the temporary window
    	this->anim_saved_screen->hide();
//    	delete this->anim_saved_screen;
//    	this->anim_saved_screen = NULL;
    }


}

bool OskControl::onBeforeAnimation(MMSPulser *animThread) {
	// init animation opacity
	this->anim_opacity = 255;

	return true;
}

bool OskControl::onAnimation(MMSPulser *animThread) {
	// get new opacity
	this->anim_opacity-= animThread->getStepLength();

	// animation finished?
	if (this->anim_opacity <= 0) {
		// yes
		return false;
	}

	// set new opacity
	this->anim_saved_screen->setOpacity(this->anim_opacity);

	return true;
}
