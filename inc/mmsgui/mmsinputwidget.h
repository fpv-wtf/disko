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

#ifndef MMSINPUTWIDGET_H_
#define MMSINPUTWIDGET_H_

#include "mmsgui/mmswidget.h"

//! With this class you can display and edit one line text.
/*!
You can display and edit one line of text.
\author Jens Schneider
*/
class MMSInputWidget : public MMSWidget {

    private:
        string         		className;
        MMSInputWidgetClass *inputWidgetClass;
        MMSInputWidgetClass myInputWidgetClass;

        MMSFBFont 		*font;
        int				cursor_pos;
        bool			cursor_on;
        int 			scroll_x;

        class MMSInputWidgetThread	*iwt;

        bool create(MMSWindow *root, string className, MMSTheme *theme);

        void handleInput(MMSInputEvent *inputevent);

        bool init();
        bool draw(bool *backgroundFilled = NULL);
        void drawCursor(bool cursor_on);

    public:
        MMSInputWidget(MMSWindow *root, string className, MMSTheme *theme = NULL);
        ~MMSInputWidget();

        MMSWidget *copyWidget();

        void setCursorPos(int cursor_pos, bool refresh = true);
        void addTextAfterCursorPos(string text, bool refresh = true);
        void removeTextBeforeCursorPos(int textlen, bool refresh = true);

    public:
    	/* theme access methods */
        string getFontPath();
        string getFontName();
        unsigned int getFontSize();
        MMSALIGNMENT getAlignment();
        MMSFBColor getColor();
        MMSFBColor getSelColor();
        string getText();

        void setFontPath(string fontpath, bool load = true, bool refresh = true);
        void setFontName(string fontname, bool load = true, bool refresh = true);
        void setFontSize(unsigned int  fontsize, bool load = true, bool refresh = true);
        void setFont(string fontpath, string fontname, unsigned int fontsize, bool load = true, bool refresh = true);
        void setAlignment(MMSALIGNMENT alignment, bool refresh = true);
        void setColor(MMSFBColor color, bool refresh = true);
        void setSelColor(MMSFBColor selcolor, bool refresh = true);
        void setText(string text, bool refresh = true, bool reset_cursor = true);

        void updateFromThemeClass(MMSInputWidgetClass *themeClass);

    friend class MMSInputWidgetThread;
};

#endif /*MMSINPUTWIDGET_H_*/
