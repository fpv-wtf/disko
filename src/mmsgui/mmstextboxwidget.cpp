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
#include "mmsgui/mmstextboxwidget.h"

MMSTextBoxWidget::MMSTextBoxWidget(MMSWindow *root, string className, MMSTheme *theme) : MMSWidget() {
    create(root, className, theme);
}

MMSTextBoxWidget::~MMSTextBoxWidget() {
    if (this->rootwindow) {
        this->rootwindow->fm->releaseFont(this->font);
    }
    for(vector<TEXTBOX_WORDGEOM *>::iterator it(wordgeom.begin()); it != wordgeom.end(); ++it)
        delete(*it);
}

bool MMSTextBoxWidget::create(MMSWindow *root, string className, MMSTheme *theme) {
	this->type = MMSWIDGETTYPE_TEXTBOX;
    this->className = className;

    // init attributes for drawable widgets
	this->da = new MMSWIDGET_DRAWABLE_ATTRIBUTES;
    if (theme) this->da->theme = theme; else this->da->theme = globalTheme;
    this->textBoxWidgetClass = this->da->theme->getTextBoxWidgetClass(className);
    this->da->baseWidgetClass = &(this->da->theme->textBoxWidgetClass.widgetClass);
    if (this->textBoxWidgetClass) this->da->widgetClass = &(this->textBoxWidgetClass->widgetClass); else this->da->widgetClass = NULL;

    /* clear */
    this->font = NULL;
    this->lasttext = "";
    this->surfaceChanged = true;

    return MMSWidget::create(root, true, false, true, true, false, false, true);
}

MMSWidget *MMSTextBoxWidget::copyWidget() {
    /* create widget */
    MMSTextBoxWidget *newWidget = new MMSTextBoxWidget(this->rootwindow, className);

    /* copy widget */
    *newWidget = *this;

    /* copy base widget */
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    /* reload my font */
    newWidget->font = NULL;
    if (this->rootwindow) {
        newWidget->font = this->rootwindow->fm->getFont(newWidget->getFontPath(), newWidget->getFontName(), newWidget->getFontSize());
    }

    return newWidget;
}


void MMSTextBoxWidget::setSurfaceGeometry(unsigned int width, unsigned int height) {
    this->surfaceChanged = true;
   	MMSWidget::setSurfaceGeometry(width, height);
}

bool MMSTextBoxWidget::calcWordGeom(string text, unsigned int startWidth, unsigned int startHeight,
                              unsigned int *realWidth, unsigned int *realHeight,
                              unsigned int *scrollDX, unsigned int *scrollDY, unsigned int *lines, unsigned int *paragraphs,
                              bool wrap, bool splitwords, MMSALIGNMENT alignment) {
    int fontHeight, blankWidth;
    unsigned int x, y;

    /* init */
    *realWidth = startWidth;
    *realHeight = startHeight;
    *lines = 0;
    *paragraphs = 0;

    /* get font height */
    this->font->getHeight(&fontHeight);
    *scrollDX = fontHeight;
    *scrollDY = fontHeight;

    /* has text or surface changed? */
    if ((text == this->lasttext)&&(!this->surfaceChanged)) return false;
    this->lasttext = text;
    this->surfaceChanged = false;

    /* clear wordgeom */
    for (int i = (int)(this->wordgeom.size())-1; i >= 0; i--) {
        delete this->wordgeom.at(i);
        this->wordgeom.erase(this->wordgeom.end()-1);
    }

    /* is text set? */
    if (text=="")
        /* no text, all is done */
        return true;

    /* get width of a blank character */
    this->font->getStringWidth(" ", -1, &blankWidth);

    /* through the text and extract single words */
    do {
        int lfindex;
        int index;
        lfindex = (int)text.find("\n");
        index = (int)text.find(" ");
        if (lfindex < 0) {
            if (index < 0) index = text.size();
            if (index == 0) {
                text = text.substr(index + 1);
                continue;
            }
        }
        else {
            if ((index < 0)||(index > lfindex))
                index = lfindex;
            else
            if (index < lfindex) {
                if (index < 0) index = text.size();
                if (index == 0) {
                    text = text.substr(index + 1);
                    continue;
                }
                lfindex = -1;
            }
        }

        if (*lines == 0) {
            /* first word */
            *lines = 1;
            x = 0;
            y = 0;
        }

        /* new word */
        TEXTBOX_WORDGEOM *mywordgeom = new TEXTBOX_WORDGEOM;
        mywordgeom->geom.h=fontHeight;
        mywordgeom->word  =text.substr(0, index);


        this->font->getStringWidth(mywordgeom->word, -1, &mywordgeom->geom.w);

        if (x > 0)
            x += blankWidth;

        unsigned int endpos = x + mywordgeom->geom.w;

        bool gotonext = true;

        if ((wrap)&&(splitwords)) {
            /* split words in wrap mode */
            if ((index > 1)&&(mywordgeom->geom.w > (int)*realWidth)) {
                /* recalculate index */
                while ((index > 1)&&(mywordgeom->geom.w > (int)*realWidth)) {
                    index--;
                    mywordgeom->word = text.substr(0, index);
                    this->font->getStringWidth(mywordgeom->word, -1, &mywordgeom->geom.w);
                    endpos = x + mywordgeom->geom.w;
                }

                text = text.substr(index);
                gotonext = false;
            }
        }

        if ((x==0)||(endpos <= *realWidth)||(wrap==false)) {
            if (endpos > *realWidth) {
                if (wrap==false)
                    *realWidth = endpos;
                else {
                    mywordgeom->geom.w-= endpos - *realWidth;
                    endpos = *realWidth;
                }
            }

            mywordgeom->geom.x = x;
            mywordgeom->geom.y = y;

            x = endpos;

            mywordgeom->line = *lines;
            mywordgeom->paragraph = *paragraphs;
        }
        else
        {
            x = 0;
            y+= fontHeight;
            (*lines)++;

            mywordgeom->geom.x = x;
            mywordgeom->geom.y = y;

            x += mywordgeom->geom.w;

            mywordgeom->line = *lines;
            mywordgeom->paragraph = *paragraphs;
        }

        if ((lfindex >= 0)||(gotonext==false)) {
            x = 0;
            y+= fontHeight;
            (*lines)++;
            if (lfindex >= 0) (*paragraphs)++;
        }

        /* add to list */
        wordgeom.push_back(mywordgeom);

        if (gotonext) {
            if (index + 1 < (int)text.size())
                text = text.substr(index + 1);
            else
                text = "";
        }
    } while (text != "");

    /* go through the list and calculate horizontal text alignment */
    unsigned int oldline = 1;
    unsigned int oldpos = 0;
    for (unsigned int i = 0; i < wordgeom.size(); i++) {
        if (wordgeom.at(i)->line != oldline) {
            if   ((alignment == MMSALIGNMENT_CENTER)||(alignment == MMSALIGNMENT_TOP_CENTER)||(alignment == MMSALIGNMENT_BOTTOM_CENTER)) {
                unsigned int diff = (*realWidth - wordgeom.at(i-1)->geom.x - wordgeom.at(i-1)->geom.w) / 2;
                for (unsigned int j = oldpos; j < i; j++)
                    wordgeom.at(j)->geom.x += diff;
            }
            else
            if   ((alignment == MMSALIGNMENT_RIGHT)||(alignment == MMSALIGNMENT_TOP_RIGHT)
                ||(alignment == MMSALIGNMENT_BOTTOM_RIGHT)) {
                unsigned int diff = *realWidth - wordgeom.at(i-1)->geom.x - wordgeom.at(i-1)->geom.w;
                for (unsigned int j = oldpos; j < i; j++)
                    wordgeom.at(j)->geom.x += diff;
            }
            else
            if  (((alignment == MMSALIGNMENT_JUSTIFY)||(alignment == MMSALIGNMENT_TOP_JUSTIFY)
                ||(alignment == MMSALIGNMENT_BOTTOM_JUSTIFY))&&(wordgeom.at(i)->paragraph == wordgeom.at(i-1)->paragraph)) {
                if (oldpos < i-1) {
                    unsigned int diff = (*realWidth - wordgeom.at(i-1)->geom.x - wordgeom.at(i-1)->geom.w) / (i-1-oldpos);
                    for (unsigned int j = oldpos; j < i; j++) {
                        wordgeom.at(j)->geom.x += (j - oldpos) * diff;
                    }
                }
            }
            oldpos = i;
            oldline = wordgeom.at(i)->line;
        }
    }
    if   ((alignment == MMSALIGNMENT_CENTER)||(alignment == MMSALIGNMENT_TOP_CENTER)||(alignment == MMSALIGNMENT_BOTTOM_CENTER)) {
        unsigned int diff = (*realWidth - wordgeom.at(wordgeom.size()-1)->geom.x - wordgeom.at(wordgeom.size()-1)->geom.w) / 2;
        for (unsigned int j = oldpos; j < wordgeom.size(); j++)
            wordgeom.at(j)->geom.x += diff;
    }
    else
    if   ((alignment == MMSALIGNMENT_RIGHT)||(alignment == MMSALIGNMENT_TOP_RIGHT)
        ||(alignment == MMSALIGNMENT_BOTTOM_RIGHT)) {
        unsigned int diff = *realWidth - wordgeom.at(wordgeom.size()-1)->geom.x - wordgeom.at(wordgeom.size()-1)->geom.w;
        for (unsigned int j = oldpos; j < wordgeom.size(); j++)
            wordgeom.at(j)->geom.x += diff;
    }

    /* go through the list and calculate vertical text alignment */
    if (fontHeight * (*lines) > *realHeight)
        *realHeight = fontHeight * (*lines);
    else
    if (fontHeight * (*lines) < *realHeight) {
        if   ((alignment == MMSALIGNMENT_CENTER)||(alignment == MMSALIGNMENT_LEFT)
            ||(alignment == MMSALIGNMENT_RIGHT)||(alignment == MMSALIGNMENT_JUSTIFY)) {
            unsigned int diff = (*realHeight - fontHeight * (*lines)) / 2;
            if (diff > 0)
                for (unsigned int i = 0; i < wordgeom.size(); i++)
                    wordgeom.at(i)->geom.y += diff;
        }
        else
        if   ((alignment == MMSALIGNMENT_BOTTOM_CENTER)||(alignment == MMSALIGNMENT_BOTTOM_LEFT)
            ||(alignment == MMSALIGNMENT_BOTTOM_RIGHT)||(alignment == MMSALIGNMENT_BOTTOM_JUSTIFY)) {
            unsigned int diff = (*realHeight - fontHeight * (*lines));
            if (diff > 0)
                for (unsigned int i = 0; i < wordgeom.size(); i++)
                    wordgeom.at(i)->geom.y += diff;
        }
    }

    return true;
}


bool MMSTextBoxWidget::init() {
    /* init widget basics */
    if (!MMSWidget::init())
        return false;

    /* load font */
    this->font = this->rootwindow->fm->getFont(getFontPath(), getFontName(), getFontSize());

    return true;
}


bool MMSTextBoxWidget::draw(bool *backgroundFilled) {
    bool myBackgroundFilled = false;

    if (!this->initialized) {
        /* init widget (e.g. load images, fonts, ...) */
        init();
        this->initialized = true;
    }

    if (backgroundFilled) {
    	if (this->has_own_surface)
    		*backgroundFilled = false;
    }
    else
        backgroundFilled = &myBackgroundFilled;

    /* calculate text and surface size */
    if (this->font) {
        unsigned int realWidth, realHeight, scrollDX, scrollDY, lines, paragraphs;

        if (!this->translated) {
        	if ((this->rootwindow)&&(this->rootwindow->windowmanager)&&(getTranslate())) {
				// translate the text
        		string source;
        		getText(source);
        		this->rootwindow->windowmanager->getTranslator()->translate(source, this->translated_text);
        	}
        	else {
        		// text can not or should not translated
				getText(this->translated_text);
        	}

        	// mark as translated
        	this->translated = true;
        }

        if (calcWordGeom(this->translated_text, getInnerGeometry().w, getInnerGeometry().h, &realWidth, &realHeight, &scrollDX, &scrollDY,
                         &lines, &paragraphs, getWrap(), getSplitWords(), getAlignment())) {
            /* text has changed, reset something */
        	setScrollSize(scrollDX, scrollDY);
          	setSurfaceGeometry(realWidth, realHeight);
        }
    }

    /* draw widget basics */
    if (MMSWidget::draw(backgroundFilled)) {

        /* lock */
        this->surface->lock();

        /* draw my things */
        if (this->font) {
        	MMSFBRectangle surfaceGeom = getSurfaceGeometry();

            MMSFBColor color;

            if (isSelected())
                color = getSelColor();
            else
                color = getColor();

            if (color.a) {
                /* set the font */
                this->surface->setFont(this->font);

                /* prepare for drawing */
                this->surface->setDrawingColorAndFlagsByBrightnessAndOpacity(color, getBrightness(), getOpacity());

                /* draw single words into surface */
                for (unsigned int i = 0; i < this->wordgeom.size(); i++)
                {
					if (this->has_own_surface)
						this->surface->drawString(this->wordgeom.at(i)->word, -1,
						                          surfaceGeom.x + this->wordgeom.at(i)->geom.x,
						                          surfaceGeom.y + this->wordgeom.at(i)->geom.y);
					else
						this->surface->drawString(this->wordgeom.at(i)->word, -1,
						                          surfaceGeom.x + this->wordgeom.at(i)->geom.x - this->da->scrollPosX,
						                          surfaceGeom.y + this->wordgeom.at(i)->geom.y - this->da->scrollPosY);
                }
            }
        }

        /* unlock */
        this->surface->unlock();

        /* update window surface with an area of surface */
        updateWindowSurfaceWithSurface(!*backgroundFilled);
    }

    /* draw widgets debug frame */
    return MMSWidget::drawDebug();
}


void MMSTextBoxWidget::targetLangChanged(MMS_LANGUAGE_TYPE lang) {
    this->translated = false;
}

/***********************************************/
/* begin of theme access methods (get methods) */
/***********************************************/

#define GETTEXTBOX(x) \
    if (this->myTextBoxWidgetClass.is##x()) return myTextBoxWidgetClass.get##x(); \
    else if ((textBoxWidgetClass)&&(textBoxWidgetClass->is##x())) return textBoxWidgetClass->get##x(); \
    else return this->da->theme->textBoxWidgetClass.get##x();

#define GETTEXTBOX2(x, y) \
    if (this->myTextBoxWidgetClass.is##x()) y=myTextBoxWidgetClass.get##x(); \
    else if ((textBoxWidgetClass)&&(textBoxWidgetClass->is##x())) y=textBoxWidgetClass->get##x(); \
    else y=this->da->theme->textBoxWidgetClass.get##x();

string MMSTextBoxWidget::getFontPath() {
    GETTEXTBOX(FontPath);
}

string MMSTextBoxWidget::getFontName() {
    GETTEXTBOX(FontName);
}

unsigned int MMSTextBoxWidget::getFontSize() {
    GETTEXTBOX(FontSize);
}

MMSALIGNMENT MMSTextBoxWidget::getAlignment() {
    GETTEXTBOX(Alignment);
}

bool MMSTextBoxWidget::getWrap() {
    GETTEXTBOX(Wrap);
}

bool MMSTextBoxWidget::getSplitWords() {
    GETTEXTBOX(SplitWords);
}

MMSFBColor MMSTextBoxWidget::getColor() {
    GETTEXTBOX(Color);
}

MMSFBColor MMSTextBoxWidget::getSelColor() {
    GETTEXTBOX(SelColor);
}

string MMSTextBoxWidget::getText() {
    GETTEXTBOX(Text);
}

void MMSTextBoxWidget::getText(string &text) {
    GETTEXTBOX2(Text, text);
}

bool MMSTextBoxWidget::getTranslate() {
    GETTEXTBOX(Translate);
}

/***********************************************/
/* begin of theme access methods (set methods) */
/***********************************************/

void MMSTextBoxWidget::setFontPath(string fontpath, bool load, bool refresh) {
    myTextBoxWidgetClass.setFontPath(fontpath);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->fm->releaseFont(this->font);
            this->font = this->rootwindow->fm->getFont(getFontPath(), getFontName(), getFontSize());
        }
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setFontName(string fontname, bool load, bool refresh) {
    myTextBoxWidgetClass.setFontName(fontname);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->fm->releaseFont(this->font);
            this->font = this->rootwindow->fm->getFont(getFontPath(), getFontName(), getFontSize());
        }
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setFontSize(unsigned int fontsize, bool load, bool refresh) {
    myTextBoxWidgetClass.setFontSize(fontsize);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->fm->releaseFont(this->font);
            this->font = this->rootwindow->fm->getFont(getFontPath(), getFontName(), getFontSize());
        }
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setFont(string fontpath, string fontname, unsigned int fontsize, bool load, bool refresh) {
    myTextBoxWidgetClass.setFontPath(fontpath);
    myTextBoxWidgetClass.setFontName(fontname);
    myTextBoxWidgetClass.setFontSize(fontsize);
    if (load)
        if (this->rootwindow) {
            this->rootwindow->fm->releaseFont(this->font);
            this->font = this->rootwindow->fm->getFont(getFontPath(), getFontName(), getFontSize());
        }
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setAlignment(MMSALIGNMENT alignment, bool refresh) {
    myTextBoxWidgetClass.setAlignment(alignment);
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setWrap(bool wrap, bool refresh) {
    myTextBoxWidgetClass.setWrap(wrap);
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setSplitWords(bool splitwords, bool refresh) {
    myTextBoxWidgetClass.setSplitWords(splitwords);
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setColor(MMSFBColor color, bool refresh) {
    myTextBoxWidgetClass.setColor(color);
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setSelColor(MMSFBColor selcolor, bool refresh) {
    myTextBoxWidgetClass.setSelColor(selcolor);
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setText(string text, bool refresh) {
    myTextBoxWidgetClass.setText(text);
    this->translated = false;
    this->da->scrollPosX=0;
    this->da->scrollPosY=0;
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setTranslate(bool translate, bool refresh) {
    myTextBoxWidgetClass.setTranslate(translate);
    this->translated = false;
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::updateFromThemeClass(MMSTextBoxWidgetClass *themeClass) {
    if (themeClass->isFontPath())
        setFontPath(themeClass->getFontPath());
    if (themeClass->isFontName())
        setFontName(themeClass->getFontName());
    if (themeClass->isFontSize())
        setFontSize(themeClass->getFontSize());
    if (themeClass->isAlignment())
        setAlignment(themeClass->getAlignment());
    if (themeClass->isWrap())
        setWrap(themeClass->getWrap());
    if (themeClass->isSplitWords())
        setSplitWords(themeClass->getSplitWords());
    if (themeClass->isColor())
        setColor(themeClass->getColor());
    if (themeClass->isSelColor())
        setSelColor(themeClass->getSelColor());
    if (themeClass->isText())
        setText(themeClass->getText());
    if (themeClass->isTranslate())
        setTranslate(themeClass->getTranslate());

    MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
