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
#ifndef MMSCANVASWIDGET_H_
#define MMSCANVASWIDGET_H_

#include "mmsgui/mmswidget.h"


//! With this class you get a canvas that can be the base for custom widgets that can integrate  into the gui
/*!
\author Stefan Schwarzer
*/
class MMSCanvasWidget : public MMSWidget {
    private:
        string          		className;
        MMSCanvasWidgetClass  	*canvasWidgetClass;
        MMSCanvasWidgetClass 	myCanvasWidgetClass;
        bool current_fgset;

        bool create(MMSWindow *root, string className, MMSTheme *theme);
        bool init();
        bool release();
        bool draw(bool *backgroundFilled = NULL);
  //      void handleInput(MMSInputEvent *inputevent);

    protected:
        typedef map<string, string> ATTRIBUTE_MAP;
        string attributes;
        ATTRIBUTE_MAP attributemap;
        bool enableRefresh(bool enable = true);
        bool checkRefreshStatus();
        MMSFontManager *getFontManager();
        MMSImageManager *getImageManager();
        MMSFBSurface *canvasSurface;
        string factoryname;
        MMSTheme *canvastheme;

        void checkInit();

    public:
        MMSCanvasWidget(MMSWindow *root, string className, MMSTheme *theme = NULL);
        virtual ~MMSCanvasWidget();

        virtual bool drawingFunc(MMSFBSurface *surf, MMSFBRectangle surfaceGeom, bool *backgroundFilled = NULL) = 0;
        virtual bool createFunc() { return false; };
        virtual bool initFunc() { return false; };
        virtual bool releaseFunc() { return false; };
        virtual bool copyFunc(MMSWidget *wid) { return false; };
//        virtual bool handleInputFunc(MMSInputEvent *inputevent) { return false; };

        MMSWidget *copyWidget();

        void add(MMSWidget *widget);

    public:
        void setAttributes(string &attr);
        void setAttributes(string attr);



        /* theme access methods */
        void updateFromThemeClass(MMSCanvasWidgetClass *themeClass);


	friend class MMSThemeManager;
	friend class MMSDialogManager;
};



#endif /* MMSCANVASWIDGET_H_ */
