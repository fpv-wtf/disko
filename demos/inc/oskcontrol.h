/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2011 BerLinux Solutions GmbH                       *
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

#ifndef OSKCONTROL_H_
#define OSKCONTROL_H_

#include "diskocontrol.h"
typedef enum {
	MOD_UKN=0,
	MOD_NORMAL=1,
	MOD_SHIFT=2,
	MOD_ALTGR=3,
	MOD_ACUTE=4,
	MOD_GRAVE=5,
	MOD_CIRCUMFLEX=6,
	MOD_TILDE=7,
	MOD_SHIFT_ALTGR=8,
	MOD_SHIFT_ACUTE=9,
	MOD_SHIFT_GRAVE=10,
	MOD_SHIFT_CIRCUMFLEX=11,
	MOD_SHIFT_TILDE=12,
	MOD_MAX=13
} OSK_MODIFIERS;

inline OSK_MODIFIERS operator ++ (OSK_MODIFIERS &a, int unused) {
	switch(a) {
		case MOD_UKN:
			a = MOD_NORMAL;
			return MOD_NORMAL;
		case MOD_NORMAL:
			a = MOD_SHIFT;
			return MOD_SHIFT;
		case MOD_SHIFT:
			a = MOD_ALTGR;
			return MOD_ALTGR;
		case MOD_ALTGR:
			a = MOD_ACUTE;
			return MOD_ACUTE;
		case MOD_ACUTE:
			a = MOD_GRAVE;
			return MOD_GRAVE;
		case MOD_GRAVE:
			a = MOD_CIRCUMFLEX;
			return MOD_CIRCUMFLEX;
		case MOD_CIRCUMFLEX:
			a = MOD_TILDE;
			return MOD_TILDE;
		case MOD_TILDE:
			a = MOD_SHIFT_ALTGR;
			return MOD_SHIFT_ALTGR;
		case MOD_SHIFT_ALTGR:
			a = MOD_SHIFT_ACUTE;
			return MOD_SHIFT_ACUTE;
		case MOD_SHIFT_ACUTE:
			a = MOD_SHIFT_GRAVE;
			return MOD_SHIFT_GRAVE;
		case MOD_SHIFT_GRAVE:
			a = MOD_SHIFT_CIRCUMFLEX;
			return MOD_SHIFT_CIRCUMFLEX;
		case MOD_SHIFT_CIRCUMFLEX:
			a = MOD_SHIFT_TILDE;
			return MOD_SHIFT_TILDE;
		case MOD_SHIFT_TILDE:
			a = MOD_MAX;
			return MOD_MAX;
		case MOD_MAX:
			return MOD_MAX;
	}
}

typedef struct {
	string modifierlabels[13];
	int labelcount;
	int keycode;
} OSK_MODLABELS;

typedef struct {
	string symbol;
	int keycode;
	OSK_MODIFIERS modifier;
} COMPLEX_KEY;

//global labelmap
typedef map<MMSLabelWidget*, OSK_MODLABELS> OSK_LABELMAP;

typedef map<MMSWindow *, OSK_LABELMAP> OSK_LANG_LABELMAP;

//global buttonmap
typedef map<MMSWidget*, OSK_MODLABELS> OSK_BUTTONMAP;

class OskControl : public DiskoControl {
	public:
		OskControl(MMSDialogManager &dm, string layout = "en", bool complex = false, string name ="");
		~OskControl();

        sigc::signal<void, string*> *onCharPressed;
        sigc::signal<void, COMPLEX_KEY*> *onCharPressedComplex;

        MMSWindow *getLabelWindow();

        void switchLang();

	private:
		string layout;
		OSK_LANG_LABELMAP labelmap;
		OSK_BUTTONMAP buttonmap;
		OSK_MODIFIERS modifier;
		bool complex;
		void parseLabelString(std::string &labelstr, std::string labels[13], int *labelcount = NULL, int *keycode = NULL);
		void findControls(MMSWindow *oskwin);
		void onReturn(MMSWidget*);
		void applyLabels(MMSWindow *);
		OSK_MODIFIERS addModifiers(OSK_MODIFIERS currentmod, OSK_MODIFIERS newmod);
		OSK_MODIFIERS getModifierFromString(std::string &mod);

		MMSWindow *label;
		MMSWindow *osk_de;
		MMSWindow *osk_ru;
		MMSWindow *curr_osk;

		MMSFBWindow *anim_saved_screen;
		bool onBeforeAnimation(MMSPulser *animThread);
		bool onAnimation(MMSPulser *animThread);
		MMSPulser animThread;
    	//! temporary opacity of anim_saved_screen window for animated theme switch
    	int					anim_opacity;

};

#endif /* OSKCONTROL_H_ */
