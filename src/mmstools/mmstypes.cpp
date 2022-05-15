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

#include "mmstools/mmstypes.h"
#include "mmstools/tools.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>


string getMMSFBBackendString(MMSFBBackend be) {
    if(be == MMSFB_BE_DFB)
        return MMSFB_BE_DFB_STR;
    if(be == MMSFB_BE_X11)
        return MMSFB_BE_X11_STR;
    if(be == MMSFB_BE_FBDEV)
        return MMSFB_BE_FBDEV_STR;
    return MMSFB_BE_NONE_STR;
}

MMSFBBackend getMMSFBBackendFromString(string be) {
    if(be == MMSFB_BE_NONE_STR)
        return MMSFB_BE_NONE;
    if(be == MMSFB_BE_DFB_STR)
        return MMSFB_BE_DFB;
    if(be == MMSFB_BE_X11_STR)
        return MMSFB_BE_X11;
    if(be == MMSFB_BE_FBDEV_STR)
        return MMSFB_BE_FBDEV;
    return MMSFB_BE_NONE;
}

string getMMSFBOutputTypeString(MMSFBOutputType ot) {
    if(ot == MMSFB_OT_STDFB)
        return MMSFB_OT_STDFB_STR;
    if(ot == MMSFB_OT_MATROXFB)
        return MMSFB_OT_MATROXFB_STR;
    if(ot == MMSFB_OT_VIAFB)
        return MMSFB_OT_VIAFB_STR;
    if(ot == MMSFB_OT_X11)
        return MMSFB_OT_X11_STR;
    if(ot == MMSFB_OT_XSHM)
        return MMSFB_OT_XSHM_STR;
    if(ot == MMSFB_OT_XVSHM)
        return MMSFB_OT_XVSHM_STR;
    if(ot == MMSFB_OT_DAVINCIFB)
        return MMSFB_OT_DAVINCIFB_STR;
    if(ot == MMSFB_OT_OMAPFB)
        return MMSFB_OT_OMAPFB_STR;
    if(ot == MMSFB_OT_OGL)
        return MMSFB_OT_OGL_STR;
    return MMSFB_OT_NONE_STR;
}

MMSFBOutputType getMMSFBOutputTypeFromString(string ot) {
	if(ot == MMSFB_OT_NONE_STR)
        return MMSFB_OT_NONE;
	if(ot == MMSFB_OT_STDFB_STR)
        return MMSFB_OT_STDFB;
	if(ot == MMSFB_OT_MATROXFB_STR)
        return MMSFB_OT_MATROXFB;
	if(ot == MMSFB_OT_VIAFB_STR)
        return MMSFB_OT_VIAFB;
	if(ot == MMSFB_OT_X11_STR)
        return MMSFB_OT_X11;
	if(ot == MMSFB_OT_XSHM_STR)
        return MMSFB_OT_XSHM;
	if(ot == MMSFB_OT_XVSHM_STR)
        return MMSFB_OT_XVSHM;
	if(ot == MMSFB_OT_DAVINCIFB_STR)
        return MMSFB_OT_DAVINCIFB;
	if(ot == MMSFB_OT_OMAPFB_STR)
        return MMSFB_OT_OMAPFB;
	if(ot == MMSFB_OT_OGL_STR)
        return MMSFB_OT_OGL;
    return MMSFB_OT_NONE;
}

string getMMSFBFullScreenModeString(MMSFBFullScreenMode fsm) {
    if(fsm == MMSFB_FSM_FALSE)
        return MMSFB_FSM_FALSE_STR;
    if(fsm == MMSFB_FSM_TRUE)
        return MMSFB_FSM_TRUE_STR;
    if(fsm == MMSFB_FSM_ASPECT_RATIO)
        return MMSFB_FSM_ASPECT_RATIO_STR;
    return MMSFB_FSM_NONE_STR;
}

MMSFBFullScreenMode getMMSFBFullScreenModeFromString(string fsm) {
    if(fsm == MMSFB_FSM_NONE_STR)
        return MMSFB_FSM_NONE;
    if(fsm == MMSFB_FSM_FALSE_STR)
        return MMSFB_FSM_FALSE;
    if(fsm == MMSFB_FSM_TRUE_STR)
        return MMSFB_FSM_TRUE;
    if(fsm == MMSFB_FSM_ASPECT_RATIO_STR)
        return MMSFB_FSM_ASPECT_RATIO;
    return MMSFB_FSM_NONE;
}



string getMMSFBPixelFormatString(MMSFBSurfacePixelFormat pf) {
	switch (pf) {
    case MMSFB_PF_RGB16:	return MMSFB_PF_RGB16_STR;
    case MMSFB_PF_RGB24:	return MMSFB_PF_RGB24_STR;
    case MMSFB_PF_RGB32:	return MMSFB_PF_RGB32_STR;
    case MMSFB_PF_ARGB:		return MMSFB_PF_ARGB_STR;
    case MMSFB_PF_A8:		return MMSFB_PF_A8_STR;
    case MMSFB_PF_YUY2:		return MMSFB_PF_YUY2_STR;
    case MMSFB_PF_UYVY:		return MMSFB_PF_UYVY_STR;
    case MMSFB_PF_I420:		return MMSFB_PF_I420_STR;
    case MMSFB_PF_YV12:		return MMSFB_PF_YV12_STR;
    case MMSFB_PF_AiRGB:	return MMSFB_PF_AiRGB_STR;
    case MMSFB_PF_A1:		return MMSFB_PF_A1_STR;
    case MMSFB_PF_NV12:		return MMSFB_PF_NV12_STR;
    case MMSFB_PF_NV16:		return MMSFB_PF_NV16_STR;
    case MMSFB_PF_NV21:		return MMSFB_PF_NV21_STR;
    case MMSFB_PF_AYUV:		return MMSFB_PF_AYUV_STR;
    case MMSFB_PF_A4:		return MMSFB_PF_A4_STR;
    case MMSFB_PF_ARGB1666:return MMSFB_PF_ARGB1666_STR;
    case MMSFB_PF_ARGB6666:return MMSFB_PF_ARGB6666_STR;
    case MMSFB_PF_RGB18:	return MMSFB_PF_RGB18_STR;
    case MMSFB_PF_LUT2:		return MMSFB_PF_LUT2_STR;
    case MMSFB_PF_RGB444:	return MMSFB_PF_RGB444_STR;
    case MMSFB_PF_RGB555:	return MMSFB_PF_RGB555_STR;
    case MMSFB_PF_ARGB1555:return MMSFB_PF_ARGB1555_STR;
    case MMSFB_PF_RGB332:	return MMSFB_PF_RGB332_STR;
    case MMSFB_PF_ALUT44:	return MMSFB_PF_ALUT44_STR;
    case MMSFB_PF_LUT8:		return MMSFB_PF_LUT8_STR;
    case MMSFB_PF_ARGB2554:return MMSFB_PF_ARGB2554_STR;
    case MMSFB_PF_ARGB4444:return MMSFB_PF_ARGB4444_STR;
    case MMSFB_PF_ARGB3565:return MMSFB_PF_ARGB3565_STR;
    case MMSFB_PF_BGR24:	return MMSFB_PF_BGR24_STR;
    case MMSFB_PF_BGR555:	return MMSFB_PF_BGR555_STR;
    case MMSFB_PF_ABGR:		return MMSFB_PF_ABGR_STR;
    default:				return MMSFB_PF_NONE_STR;
	}
}

MMSFBSurfacePixelFormat getMMSFBPixelFormatFromString(string pf) {
    if(pf == MMSFB_PF_NONE_STR)
        return MMSFB_PF_NONE;
    if(pf == MMSFB_PF_RGB16_STR)
        return MMSFB_PF_RGB16;
    if(pf == MMSFB_PF_RGB24_STR)
        return MMSFB_PF_RGB24;
    if(pf == MMSFB_PF_RGB32_STR)
        return MMSFB_PF_RGB32;
    if(pf == MMSFB_PF_ARGB_STR)
        return MMSFB_PF_ARGB;
    if(pf == MMSFB_PF_A8_STR)
        return MMSFB_PF_A8;
    if(pf == MMSFB_PF_YUY2_STR)
        return MMSFB_PF_YUY2;
    if(pf == MMSFB_PF_UYVY_STR)
        return MMSFB_PF_UYVY;
    if(pf == MMSFB_PF_I420_STR)
        return MMSFB_PF_I420;
    if(pf == MMSFB_PF_YV12_STR)
        return MMSFB_PF_YV12;
    if(pf == strToUpr(MMSFB_PF_AiRGB_STR))
        return MMSFB_PF_AiRGB;
    if(pf == MMSFB_PF_A1_STR)
        return MMSFB_PF_A1;
    if(pf == MMSFB_PF_NV12_STR)
        return MMSFB_PF_NV12;
    if(pf == MMSFB_PF_NV16_STR)
        return MMSFB_PF_NV16;
    if(pf == MMSFB_PF_NV21_STR)
        return MMSFB_PF_NV21;
    if(pf == MMSFB_PF_AYUV_STR)
        return MMSFB_PF_AYUV;
    if(pf == MMSFB_PF_A4_STR)
        return MMSFB_PF_A4;
    if(pf == MMSFB_PF_ARGB1666_STR)
        return MMSFB_PF_ARGB1666;
    if(pf == MMSFB_PF_ARGB6666_STR)
        return MMSFB_PF_ARGB6666;
    if(pf == MMSFB_PF_RGB18_STR)
        return MMSFB_PF_RGB18;
    if(pf == MMSFB_PF_LUT2_STR)
        return MMSFB_PF_LUT2;
    if(pf == MMSFB_PF_RGB444_STR)
        return MMSFB_PF_RGB444;
    if(pf == MMSFB_PF_RGB555_STR)
        return MMSFB_PF_RGB555;
    if(pf == MMSFB_PF_ARGB1555_STR)
        return MMSFB_PF_ARGB1555;
    if(pf == MMSFB_PF_RGB332_STR)
        return MMSFB_PF_RGB332;
    if(pf == MMSFB_PF_ALUT44_STR)
        return MMSFB_PF_ALUT44;
    if(pf == MMSFB_PF_LUT8_STR)
        return MMSFB_PF_LUT8;
    if(pf == MMSFB_PF_ARGB2554_STR)
        return MMSFB_PF_ARGB2554;
    if(pf == MMSFB_PF_ARGB4444_STR)
        return MMSFB_PF_ARGB4444;
    if(pf == MMSFB_PF_ARGB3565_STR)
        return MMSFB_PF_ARGB3565;
    if(pf == MMSFB_PF_BGR24_STR)
        return MMSFB_PF_BGR24;
    if(pf == MMSFB_PF_BGR555_STR)
        return MMSFB_PF_BGR555;
    if(pf == MMSFB_PF_ABGR_STR)
        return MMSFB_PF_ABGR;
    return MMSFB_PF_NONE;
}


bool getMMSFBColorFromString(string input, MMSFBColor *color) {
	// check ret ptr
	if (!color)
		return false;

	// reset color
    color->r = 0;
    color->g = 0;
    color->b = 0;
    color->a = 0;

    // check input string
    if (input == "")
        return false;

    if (input.size()!=9)
        return false;

    if (input.substr(0,1)!="#")
        return false;

    // set color values
    color->r = hexToInt(input.substr(1,2).c_str());
    color->g = hexToInt(input.substr(3,2).c_str());
    color->b = hexToInt(input.substr(5,2).c_str());
    color->a = hexToInt(input.substr(7,2).c_str());

    return true;
}


string getMMSFBColorString(MMSFBColor color) {
	string ret = "#";
	ret+= ucharToHex(color.r);
	ret+= ucharToHex(color.g);
	ret+= ucharToHex(color.b);
	ret+= ucharToHex(color.a);
	return ret;
}



string getMMSFBPointerModeString(MMSFBPointerMode pm) {
    if(pm == MMSFB_PM_FALSE)
        return MMSFB_PM_FALSE_STR;
    if(pm == MMSFB_PM_TRUE)
        return MMSFB_PM_TRUE_STR;
    if(pm == MMSFB_PM_EXTERNAL)
        return MMSFB_PM_EXTERNAL_STR;
    return MMSFB_PM_NONE_STR;
}

MMSFBPointerMode getMMSFBPointerModeFromString(string pm) {
    if(pm == MMSFB_PM_NONE_STR)
        return MMSFB_PM_NONE;
    if(pm == MMSFB_PM_FALSE_STR)
        return MMSFB_PM_FALSE;
    if(pm == MMSFB_PM_TRUE_STR)
        return MMSFB_PM_TRUE;
    if(pm == MMSFB_PM_EXTERNAL_STR)
        return MMSFB_PM_EXTERNAL;
    return MMSFB_PM_NONE;
}


const char *convertMMSKeySymbolToXKeysymString(MMSKeySymbol key) {
	switch (key) {
	case MMSKEY_CURSOR_LEFT:	return "Left";
	case MMSKEY_CURSOR_RIGHT:	return "Right";
	case MMSKEY_CURSOR_UP:		return "Up";
	case MMSKEY_CURSOR_DOWN:	return "Down";
	case MMSKEY_0:				return "0";
	case MMSKEY_1:				return "1";
	case MMSKEY_2:				return "2";
	case MMSKEY_3:				return "3";
	case MMSKEY_4:				return "4";
	case MMSKEY_5:				return "5";
	case MMSKEY_6:				return "6";
	case MMSKEY_7:				return "7";
	case MMSKEY_8:				return "8";
	case MMSKEY_9:				return "9";
	case MMSKEY_CAPITAL_A:		return "A";
	case MMSKEY_CAPITAL_B:		return "B";
	case MMSKEY_CAPITAL_C:		return "C";
	case MMSKEY_CAPITAL_D:		return "D";
	case MMSKEY_CAPITAL_E:		return "E";
	case MMSKEY_CAPITAL_F:		return "F";
	case MMSKEY_CAPITAL_G:		return "G";
	case MMSKEY_CAPITAL_H:		return "H";
	case MMSKEY_CAPITAL_I:		return "I";
	case MMSKEY_CAPITAL_J:		return "J";
	case MMSKEY_CAPITAL_K:		return "K";
	case MMSKEY_CAPITAL_L:		return "L";
	case MMSKEY_CAPITAL_M:		return "M";
	case MMSKEY_CAPITAL_N:		return "N";
	case MMSKEY_CAPITAL_O:		return "O";
	case MMSKEY_CAPITAL_P:		return "P";
	case MMSKEY_CAPITAL_Q:		return "Q";
	case MMSKEY_CAPITAL_R:		return "R";
	case MMSKEY_CAPITAL_S:		return "S";
	case MMSKEY_CAPITAL_T:		return "T";
	case MMSKEY_CAPITAL_U:		return "U";
	case MMSKEY_CAPITAL_V:		return "V";
	case MMSKEY_CAPITAL_W:		return "W";
	case MMSKEY_CAPITAL_X:		return "X";
	case MMSKEY_CAPITAL_Y:		return "Y";
	case MMSKEY_CAPITAL_Z:		return "Z";
	case MMSKEY_SMALL_A:		return "a";
	case MMSKEY_SMALL_B:		return "b";
	case MMSKEY_SMALL_C:		return "c";
	case MMSKEY_SMALL_D:		return "d";
	case MMSKEY_SMALL_E:		return "e";
	case MMSKEY_SMALL_F:		return "f";
	case MMSKEY_SMALL_G:		return "g";
	case MMSKEY_SMALL_H:		return "h";
	case MMSKEY_SMALL_I:		return "i";
	case MMSKEY_SMALL_J:		return "j";
	case MMSKEY_SMALL_K:		return "k";
	case MMSKEY_SMALL_L:		return "l";
	case MMSKEY_SMALL_M:		return "m";
	case MMSKEY_SMALL_N:		return "n";
	case MMSKEY_SMALL_O:		return "o";
	case MMSKEY_SMALL_P:		return "p";
	case MMSKEY_SMALL_Q:		return "q";
	case MMSKEY_SMALL_R:		return "r";
	case MMSKEY_SMALL_S:		return "s";
	case MMSKEY_SMALL_T:		return "t";
	case MMSKEY_SMALL_U:		return "u";
	case MMSKEY_SMALL_V:		return "v";
	case MMSKEY_SMALL_W:		return "w";
	case MMSKEY_SMALL_X:		return "x";
	case MMSKEY_SMALL_Y:		return "y";
	case MMSKEY_SMALL_Z:		return "z";
	case MMSKEY_F1:				return "F1";
	case MMSKEY_F2:				return "F2";
	case MMSKEY_F3:				return "F3";
	case MMSKEY_F4:				return "F4";
	case MMSKEY_F5:				return "F5";
	case MMSKEY_F6:				return "F6";
	case MMSKEY_F7:				return "F7";
	case MMSKEY_F8:				return "F8";
	case MMSKEY_F9:				return "F9";
	case MMSKEY_F10:			return "F10";
	case MMSKEY_F11:			return "F11";
	case MMSKEY_F12:			return "F12";
	case MMSKEY_CAPS_LOCK:		return "Caps_Lock";
	case MMSKEY_NUM_LOCK:		return "Num_Lock";
	case MMSKEY_SCROLL_LOCK:	return "Scroll_Lock";
	case MMSKEY_ESCAPE:			return "Escape";
	case MMSKEY_TAB:			return "Tab";
	case MMSKEY_RETURN:			return "Return";
	case MMSKEY_SPACE:			return "space";
	case MMSKEY_BACKSPACE:		return "BackSpace";
	case MMSKEY_INSERT:			return "Insert";
	case MMSKEY_DELETE:			return "Delete";
	case MMSKEY_HOME:			return "Home";
	case MMSKEY_END:			return "End";
	case MMSKEY_PAGE_UP:		return "Prior";
	case MMSKEY_PAGE_DOWN:		return "Next";
	case MMSKEY_PRINT:			return "Print";
	case MMSKEY_PAUSE:			return "Pause";
	case MMSKEY_MINUS_SIGN:		return "minus";
	case MMSKEY_EQUALS_SIGN:	return "equal";
	case MMSKEY_BACKSLASH:		return "backslash";
	case MMSKEY_SEMICOLON:		return "semicolon";
	case MMSKEY_COMMA:			return "comma";
	case MMSKEY_PERIOD:			return "period";
	case MMSKEY_SLASH:			return "slash";
	case MMSKEY_SHIFT:			return "Shift";
	case MMSKEY_CONTROL:		return "Control";
	case MMSKEY_ALT:			return "Alt";
	case MMSKEY_META:			return "Meta";
	case MMSKEY_SUPER:			return "Super";
	case MMSKEY_HYPER:			return "Hyper";
	case MMSKEY_ALTGR:			return "Mode_switch";
	case MMSKEY_ASTERISK:		return "Multiply";
	case MMSKEY_PLUS_SIGN:		return "Add";
	case MMSKEY_COLON:			return "colon";
	default:					return "";
	}
}


string getMMSLanguageString(MMSLanguage lang) {
    if (lang == MMSLANG_DE)
        return MMSLANG_DE_STR;
    if (lang == MMSLANG_EN)
        return MMSLANG_EN_STR;
    if (lang == MMSLANG_DK)
        return MMSLANG_DK_STR;
    if (lang == MMSLANG_ES)
        return MMSLANG_ES_STR;
    if (lang == MMSLANG_FI)
        return MMSLANG_FI_STR;
    if (lang == MMSLANG_FR)
        return MMSLANG_FR_STR;
    if (lang == MMSLANG_IT)
        return MMSLANG_IT_STR;
    if (lang == MMSLANG_NL)
        return MMSLANG_NL_STR;
    if (lang == MMSLANG_NO)
        return MMSLANG_NO_STR;
    if (lang == MMSLANG_SE)
        return MMSLANG_SE_STR;
    if (lang == MMSLANG_TR)
        return MMSLANG_TR_STR;
    if (lang == MMSLANG_CN)
        return MMSLANG_CN_STR;
    if (lang == MMSLANG_IL)
        return MMSLANG_IL_STR;
    if (lang == MMSLANG_AR)
        return MMSLANG_AR_STR;
    if (lang == MMSLANG_CS)
        return MMSLANG_CS_STR;
    if (lang == MMSLANG_RU)
        return MMSLANG_RU_STR;
    return MMSLANG_NONE_STR;
}

MMSLanguage getMMSLanguageFromString(string lang) {
	strToUpr(&lang);
    if (lang == MMSLANG_NONE_STR)
        return MMSLANG_NONE;
    if (lang == MMSLANG_DE_STR)
        return MMSLANG_DE;
    if (lang == MMSLANG_EN_STR)
        return MMSLANG_EN;
    if (lang == MMSLANG_DK_STR)
        return MMSLANG_DK;
    if (lang == MMSLANG_ES_STR)
        return MMSLANG_ES;
    if (lang == MMSLANG_FI_STR)
        return MMSLANG_FI;
    if (lang == MMSLANG_FR_STR)
        return MMSLANG_FR;
    if (lang == MMSLANG_IT_STR)
        return MMSLANG_IT;
    if (lang == MMSLANG_NL_STR)
        return MMSLANG_NL;
    if (lang == MMSLANG_NO_STR)
        return MMSLANG_NO;
    if (lang == MMSLANG_SE_STR)
        return MMSLANG_SE;
    if (lang == MMSLANG_TR_STR)
        return MMSLANG_TR;
    if (lang == MMSLANG_CN_STR)
        return MMSLANG_CN;
    if (lang == MMSLANG_IL_STR)
        return MMSLANG_IL;
    if (lang == MMSLANG_AR_STR)
        return MMSLANG_AR;
    if (lang == MMSLANG_CS_STR)
        return MMSLANG_CS;
    if (lang == MMSLANG_RU_STR)
        return MMSLANG_RU;
    return MMSLANG_NONE;
}




MMS_HALF_FLOAT convertFloat2HalfFloat(float f) {
	unsigned int x = *(unsigned int *)&f;
	unsigned int sign = (unsigned short)(x >> 31);
	unsigned int mantissa;
	unsigned int exp;
	MMS_HALF_FLOAT hf;

	// get mantissa
	mantissa = x & ((1 << 23) - 1);

	// get exponent bits
	exp = x & FLOAT_MAX_BIASED_EXP;

	if (exp >= HALF_FLOAT_MAX_BIASED_EXP_AS_SINGLE_FP_EXP) {
		// check if the original single precision float number is a NaN
		if (mantissa && (exp == FLOAT_MAX_BIASED_EXP)) {
			// we have a single precision NaN
			mantissa = (1 << 23) - 1;
		}
		else {
			// 16-bit half-float representation stores number as Inf
			mantissa = 0;
		}

		hf = (((MMS_HALF_FLOAT)sign) << 15) | (MMS_HALF_FLOAT)(HALF_FLOAT_MAX_BIASED_EXP) | (MMS_HALF_FLOAT)(mantissa >> 13);
	}
	// check if exponent is <= -15
	else if (exp <= HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP) {
		// store a denorm half-float value or zero
		exp = (HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP - exp) >> 23;
		mantissa >>= (14 + exp);
		hf = (((MMS_HALF_FLOAT)sign) << 15) | (MMS_HALF_FLOAT)(mantissa);
	}
	else {
		hf = (((MMS_HALF_FLOAT)sign) << 15) | (MMS_HALF_FLOAT)((exp - HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP) >> 13) |
			   (MMS_HALF_FLOAT)(mantissa >> 13);
	}

	return hf;
}

float convertHalfFloat2Float(MMS_HALF_FLOAT hf) {
	unsigned int sign = (unsigned int)(hf >> 15);
	unsigned int mantissa = (unsigned int)(hf & ((1 << 10) - 1));
	unsigned int exp = (unsigned int)(hf & HALF_FLOAT_MAX_BIASED_EXP);
	unsigned int f;

	if (exp == HALF_FLOAT_MAX_BIASED_EXP) {
		// we have a half-float NaN or Inf
		// half-float NaNs will be converted to a single precision NaN
		// half-float Infs will be converted to a single precision Inf
		exp = FLOAT_MAX_BIASED_EXP;
		if (mantissa)
			mantissa = (1 << 23) - 1; // set all bits to indicate a NaN
	}
	else if (exp == 0x0) {
		// convert half-float zero/denorm to single precision value
		if (mantissa) {
			mantissa <<= 1;
			exp = HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP;

			// check for leading 1 in denorm mantissa
			while ((mantissa & (1 << 10)) == 0) {
				// for every leading 0, decrement single precision exponent by 1
				// and shift half-float mantissa value to the left
				mantissa <<= 1;
				exp -= (1 << 23);
			}

			// clamp the mantissa to 10-bits
			mantissa &= ((1 << 10) - 1);
			// shift left to generate single-precision mantissa of 23-bits
			mantissa <<= 13;
		}
	}
	else {
		// shift left to generate single-precision mantissa of 23-bits
		mantissa <<= 13;
		// generate single precision biased exponent value
		exp = (exp << 13) + HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP;
	}

	f = (sign << 31) | exp | mantissa;
	return *((float *)&f);
}



bool initVertexArray(MMS_VERTEX_ARRAY *array, int eSize, int eNum,
					 MMS_VERTEX_DATA_TYPE dtype, void *data) {
	if (!array) return false;
	array->dtype = dtype;
	array->data = data;
	if (eSize <= 0 || eNum <= 0) {
		array->eSize = 0;
		array->eNum = 0;
		return false;
	}
	array->eSize = eSize;
	array->eNum = eNum;

	if (!array->data) {
		// allocate space for vertex data
		unsigned int len = getVertexArraySize(array);
		if (!len) return false;
		array->data = malloc(len);
	}

	return true;
}

void freeVertexArray(MMS_VERTEX_ARRAY *array) {
	if (array && array->data) {
		free(array->data);
		array->data = NULL;
	}
}

unsigned int getVertexArraySize(MMS_VERTEX_ARRAY *array) {
	switch (array->dtype) {
	case MMS_VERTEX_DATA_TYPE_FLOAT:
		return sizeof(float) * array->eSize * array->eNum;
	case MMS_VERTEX_DATA_TYPE_HALF_FLOAT:
		return sizeof(MMS_HALF_FLOAT) * array->eSize * array->eNum;
	default:
		return 0;
	}
}


bool initIndexArray(MMS_INDEX_ARRAY *array, MMS_INDEX_ARRAY_TYPE type, int eNum, unsigned int *data) {
	if (!array) return false;
	array->type = type;
	array->data = data;
	if (eNum < 0) {
		array->eNum = 0;
		return false;
	}
	array->eNum = eNum;

	if (!array->data) {
		// allocate space for index data
		// note: it is possible to have an index array size of 0
		//       this means, that no index data is needed to draw primitives specified with array->type
		unsigned int len = getIndexArraySize(array);
		if (len) {
			array->data = (unsigned int *)malloc(len);
		}
	}

	return true;
}

void freeIndexArray(MMS_INDEX_ARRAY *array) {
	if (array && array->data) {
		free(array->data);
		array->data = NULL;
	}
}

unsigned int getIndexArraySize(MMS_INDEX_ARRAY *array) {
	return sizeof(unsigned int) * array->eNum;
}


void multiplyMatrix(MMSMatrix result, MMSMatrix srcA, MMSMatrix srcB) {
    MMSMatrix    tmp;

	for (int i = 0; i < 4; i++) {
		tmp[i][0] =	(srcA[i][0] * srcB[0][0]) +
					(srcA[i][1] * srcB[1][0]) +
					(srcA[i][2] * srcB[2][0]) +
					(srcA[i][3] * srcB[3][0]);

		tmp[i][1] =	(srcA[i][0] * srcB[0][1]) +
					(srcA[i][1] * srcB[1][1]) +
					(srcA[i][2] * srcB[2][1]) +
					(srcA[i][3] * srcB[3][1]);

		tmp[i][2] =	(srcA[i][0] * srcB[0][2]) +
					(srcA[i][1] * srcB[1][2]) +
					(srcA[i][2] * srcB[2][2]) +
					(srcA[i][3] * srcB[3][2]);

		tmp[i][3] =	(srcA[i][0] * srcB[0][3]) +
					(srcA[i][1] * srcB[1][3]) +
					(srcA[i][2] * srcB[2][3]) +
					(srcA[i][3] * srcB[3][3]);
	}
    memcpy(result, tmp, sizeof(MMSMatrix));
}


void copyMatrix(MMSMatrix result, MMSMatrix src) {
    memcpy(result, src, sizeof(MMSMatrix));
}

bool equalMatrix(MMSMatrix result, MMSMatrix src) {
    return (memcmp(result, src, sizeof(MMSMatrix)) == 0);
}



void loadIdentityMatrix(MMSMatrix result) {
    memset(result, 0x0, sizeof(MMSMatrix));
    result[0][0] = 1.0f;
    result[1][1] = 1.0f;
    result[2][2] = 1.0f;
    result[3][3] = 1.0f;
}


void scaleMatrix(MMSMatrix result, float sx, float sy, float sz) {
    result[0][0] *= sx;
    result[0][1] *= sx;
    result[0][2] *= sx;
    result[0][3] *= sx;

    result[1][0] *= sy;
    result[1][1] *= sy;
    result[1][2] *= sy;
    result[1][3] *= sy;

    result[2][0] *= sz;
    result[2][1] *= sz;
    result[2][2] *= sz;
    result[2][3] *= sz;
}


void translateMatrix(MMSMatrix result, float tx, float ty, float tz) {
    result[3][0] += (result[0][0] * tx + result[1][0] * ty + result[2][0] * tz);
    result[3][1] += (result[0][1] * tx + result[1][1] * ty + result[2][1] * tz);
    result[3][2] += (result[0][2] * tx + result[1][2] * ty + result[2][2] * tz);
    result[3][3] += (result[0][3] * tx + result[1][3] * ty + result[2][3] * tz);
}


void rotateMatrix(MMSMatrix result, float angle, float x, float y, float z) {
   float sinAngle, cosAngle;
   float mag = sqrtf(x * x + y * y + z * z);

   sinAngle = sinf (angle * MMS_PI / 180.0f);
   cosAngle = cosf (angle * MMS_PI / 180.0f);
   if (mag > 0.0f) {
      float xx, yy, zz, xy, yz, zx, xs, ys, zs;
      float oneMinusCos;
      MMSMatrix rotMat;

      x /= mag;
      y /= mag;
      z /= mag;

      xx = x * x;
      yy = y * y;
      zz = z * z;
      xy = x * y;
      yz = y * z;
      zx = z * x;
      xs = x * sinAngle;
      ys = y * sinAngle;
      zs = z * sinAngle;
      oneMinusCos = 1.0f - cosAngle;

      rotMat[0][0] = (oneMinusCos * xx) + cosAngle;
      rotMat[0][1] = (oneMinusCos * xy) - zs;
      rotMat[0][2] = (oneMinusCos * zx) + ys;
      rotMat[0][3] = 0.0f;

      rotMat[1][0] = (oneMinusCos * xy) + zs;
      rotMat[1][1] = (oneMinusCos * yy) + cosAngle;
      rotMat[1][2] = (oneMinusCos * yz) - xs;
      rotMat[1][3] = 0.0f;

      rotMat[2][0] = (oneMinusCos * zx) - ys;
      rotMat[2][1] = (oneMinusCos * yz) + xs;
      rotMat[2][2] = (oneMinusCos * zz) + cosAngle;
      rotMat[2][3] = 0.0f;

      rotMat[3][0] = 0.0f;
      rotMat[3][1] = 0.0f;
      rotMat[3][2] = 0.0f;
      rotMat[3][3] = 1.0f;

      multiplyMatrix(result, rotMat, result);
   }
}


void frustumMatrix(MMSMatrix result, float left, float right, float bottom, float top, float nearZ, float farZ) {
    float	deltaX = right - left;
    float	deltaY = top - bottom;
    float	deltaZ = farZ - nearZ;

    if ( (nearZ <= 0.0f) || (farZ <= 0.0f) ||
         (deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f) )
         return;

    MMSMatrix frust;
    frust[0][0] = 2.0f * nearZ / deltaX;
    frust[0][1] = frust[0][2] = frust[0][3] = 0.0f;

    frust[1][1] = 2.0f * nearZ / deltaY;
    frust[1][0] = frust[1][2] = frust[1][3] = 0.0f;

    frust[2][0] = (right + left) / deltaX;
    frust[2][1] = (top + bottom) / deltaY;
    frust[2][2] = -(nearZ + farZ) / deltaZ;
    frust[2][3] = -1.0f;

    frust[3][2] = -2.0f * nearZ * farZ / deltaZ;
    frust[3][0] = frust[3][1] = frust[3][3] = 0.0f;

    multiplyMatrix(result, frust, result);
}


void perspectiveMatrix(MMSMatrix result, float fovy, float aspect, float nearZ, float farZ) {
   float frustumW, frustumH;

   frustumH = tanf(fovy / 360.0f * MMS_PI) * nearZ;
   frustumW = frustumH * aspect;

   frustumMatrix(result, -frustumW, frustumW, -frustumH, frustumH, nearZ, farZ);
}

void orthoMatrix(MMSMatrix result, float left, float right, float bottom, float top, float nearZ, float farZ) {
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;

    if ((deltaX == 0.0f) || (deltaY == 0.0f) || (deltaZ == 0.0f))
        return;

    MMSMatrix ortho;
    loadIdentityMatrix(ortho);
    ortho[0][0] = 2.0f / deltaX;
    ortho[3][0] = -(right + left) / deltaX;
    ortho[1][1] = 2.0f / deltaY;
    ortho[3][1] = -(top + bottom) / deltaY;
    ortho[2][2] = -2.0f / deltaZ;
    ortho[3][2] = -(nearZ + farZ) / deltaZ;

    multiplyMatrix(result, ortho, result);
}


bool isMMS3DObjectShown(MMS3D_OBJECT *object) {
	if (!object->shown) {
		return false;
	}
	if (object->parent) {
		return isMMS3DObjectShown(object->parent);
	}
	return true;
}




