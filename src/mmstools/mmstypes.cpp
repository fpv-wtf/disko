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

#include "mmstools/mmstypes.h"

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
    if(ot == MMSFB_OT_VESAFB)
        return MMSFB_OT_VESAFB_STR;
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
    return MMSFB_OT_NONE_STR;
}

MMSFBOutputType getMMSFBOutputTypeFromString(string ot) {
	if(ot == MMSFB_OT_NONE_STR)
        return MMSFB_OT_NONE;
	if(ot == MMSFB_OT_VESAFB_STR)
        return MMSFB_OT_VESAFB;
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
    if(pf == MMSFB_PF_RGB16)
        return MMSFB_PF_RGB16_STR;
    if(pf == MMSFB_PF_RGB24)
        return MMSFB_PF_RGB24_STR;
    if(pf == MMSFB_PF_RGB32)
        return MMSFB_PF_RGB32_STR;
    if(pf == MMSFB_PF_ARGB)
        return MMSFB_PF_ARGB_STR;
    if(pf == MMSFB_PF_A8)
        return MMSFB_PF_A8_STR;
    if(pf == MMSFB_PF_YUY2)
        return MMSFB_PF_YUY2_STR;
    if(pf == MMSFB_PF_UYVY)
        return MMSFB_PF_UYVY_STR;
    if(pf == MMSFB_PF_I420)
        return MMSFB_PF_I420_STR;
    if(pf == MMSFB_PF_YV12)
        return MMSFB_PF_YV12_STR;
    if(pf == MMSFB_PF_AiRGB)
        return MMSFB_PF_AiRGB_STR;
    if(pf == MMSFB_PF_A1)
        return MMSFB_PF_A1_STR;
    if(pf == MMSFB_PF_NV12)
        return MMSFB_PF_NV12_STR;
    if(pf == MMSFB_PF_NV16)
        return MMSFB_PF_NV16_STR;
    if (pf == MMSFB_PF_NV21)
        return MMSFB_PF_NV21_STR;
    if (pf == MMSFB_PF_AYUV)
        return MMSFB_PF_AYUV_STR;
    if (pf == MMSFB_PF_A4)
        return MMSFB_PF_A4_STR;
    if (pf == MMSFB_PF_ARGB1666)
        return MMSFB_PF_ARGB1666_STR;
    if (pf == MMSFB_PF_ARGB6666)
        return MMSFB_PF_ARGB6666_STR;
    if (pf == MMSFB_PF_RGB18)
        return MMSFB_PF_RGB18_STR;
    if (pf == MMSFB_PF_LUT2)
        return MMSFB_PF_LUT2_STR;
    if (pf == MMSFB_PF_RGB444)
        return MMSFB_PF_RGB444_STR;
    if (pf == MMSFB_PF_RGB555)
        return MMSFB_PF_RGB555_STR;
    if(pf == MMSFB_PF_ARGB1555)
        return MMSFB_PF_ARGB1555_STR;
    if(pf == MMSFB_PF_RGB332)
        return MMSFB_PF_RGB332_STR;
    if(pf == MMSFB_PF_ALUT44)
        return MMSFB_PF_ALUT44_STR;
    if(pf == MMSFB_PF_LUT8)
        return MMSFB_PF_LUT8_STR;
    if(pf == MMSFB_PF_ARGB2554)
        return MMSFB_PF_ARGB2554_STR;
    if(pf == MMSFB_PF_ARGB4444)
        return MMSFB_PF_ARGB4444_STR;
    if(pf == MMSFB_PF_ARGB3565)
        return MMSFB_PF_ARGB3565_STR;
    if(pf == MMSFB_PF_BGR24)
        return MMSFB_PF_BGR24_STR;
    return MMSFB_PF_NONE_STR;
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
    if(pf == MMSFB_PF_AiRGB_STR)
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
    return MMSFB_PF_NONE;
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


