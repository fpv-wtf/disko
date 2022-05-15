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

#ifndef MMSLABELWIDGET_H_
#define MMSLABELWIDGET_H_

#include "mmsgui/mmswidget.h"
#include "mmsgui/mmslabelwidgetthread.h"

//! With this class you can display one line text.
/*!
You can display one line of text. If you want to have more the one line, you should use the MMSTextBox widget.
The label widget cannot be focused.
\author Jens Schneider
*/
class MMSLabelWidget : public MMSWidget {

    private:
        string         		className;
        MMSLabelWidgetClass *labelWidgetClass;
        MMSLabelWidgetClass myLabelWidgetClass;

        MMSFBFont *font;

        int slide_width;
        int slide_offset;

        unsigned int frame_delay;
        unsigned int frame_delay_set;

        class MMSLabelWidgetThread  *labelThread;

        //! the translated text will be stored here, this is used in the draw() method
        string translated_text;

        //! if true the translated_text is valid
        bool translated;

        bool create(MMSWindow *root, string className, MMSTheme *theme);

    public:
        MMSLabelWidget(MMSWindow *root, string className, MMSTheme *theme = NULL);
        ~MMSLabelWidget();

        MMSWidget *copyWidget();

        bool init();
        bool draw(bool *backgroundFilled = NULL);

    public:
		//! inform the widget, that language has changed
		void targetLangChanged(MMS_LANGUAGE_TYPE lang);

        /* theme access methods */
        string getFontPath();
        string getFontName();
        unsigned int getFontSize();
        MMSALIGNMENT getAlignment();
        MMSFBColor getColor();
        MMSFBColor getSelColor();
        string getText();
        void getText(string &text);
        bool getSlidable();
        unsigned char getSlideSpeed();
        bool getTranslate();

        void setFontPath(string fontpath, bool load = true, bool refresh = true);
        void setFontName(string fontname, bool load = true, bool refresh = true);
        void setFontSize(unsigned int  fontsize, bool load = true, bool refresh = true);
        void setFont(string fontpath, string fontname, unsigned int fontsize, bool load = true, bool refresh = true);
        void setAlignment(MMSALIGNMENT alignment, bool refresh = true);
        void setColor(MMSFBColor color, bool refresh = true);
        void setSelColor(MMSFBColor selcolor, bool refresh = true);
        void setText(string text, bool refresh = true);
        void setSlidable(bool slidable);
        void setSlideSpeed(unsigned char slidespeed);
        void setTranslate(bool translate, bool refresh = true);

        void updateFromThemeClass(MMSLabelWidgetClass *themeClass);

    friend class MMSLabelWidgetThread;
};

#endif /*MMSLABELWIDGET_H_*/
