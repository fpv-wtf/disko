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
#include <cstdlib>

MMSTextBoxWidget::MMSTextBoxWidget(MMSWindow *root, string className, MMSTheme *theme) : MMSWidget() {
    create(root, className, theme);
}

MMSTextBoxWidget::~MMSTextBoxWidget() {
    for(vector<TEXTBOX_WORDGEOM *>::iterator it(wordgeom.begin()); it != wordgeom.end(); ++it) {
        delete(*it);
    }
    if (this->file) {
    	delete this->file;
    }
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

    // clear
    this->font = NULL;
    this->lasttext = "";
    this->surfaceChanged = true;
    this->file = NULL;

    return MMSWidget::create(root, true, false, true, true, false, false, true);
}

MMSWidget *MMSTextBoxWidget::copyWidget() {
    // create widget
    MMSTextBoxWidget *newWidget = new MMSTextBoxWidget(this->rootwindow, className);

    // copy widget
    *newWidget = *this;

    // copy base widget
    MMSWidget::copyWidget((MMSWidget*)newWidget);

    // reload my font
    newWidget->font = NULL;
    if (this->rootwindow) {
        newWidget->font = this->rootwindow->fm->getFont(newWidget->getFontPath(), newWidget->getFontName(), newWidget->getFontSize());
    }

    // reload my file
    newWidget->file = NULL;
   	newWidget->loadFile(false);

    return newWidget;
}


bool MMSTextBoxWidget::setSurfaceGeometry(unsigned int width, unsigned int height) {
   	if (MMSWidget::setSurfaceGeometry(width, height)) {
   	    this->surfaceChanged = true;
   	    return true;
   	}
   	return false;
}

bool MMSTextBoxWidget::calcWordGeom(string &text, unsigned int startWidth, unsigned int startHeight,
                              unsigned int *realWidth, unsigned int *realHeight,
                              unsigned int *scrollDX, unsigned int *scrollDY, unsigned int *lines, unsigned int *paragraphs,
                              bool wrap, bool splitwords, MMSALIGNMENT alignment) {
    int fontHeight, blankWidth;
    unsigned int x, y;

    // init
    *realWidth = startWidth;
    *realHeight = startHeight;
    *lines = 0;
    *paragraphs = 0;

    // get font height
    this->font->getHeight(&fontHeight);
    *scrollDX = fontHeight;
    *scrollDY = fontHeight;

    // has text or surface changed?
    if ((text == this->lasttext)&&(!this->surfaceChanged)) return false;
    this->lasttext = text;
    this->surfaceChanged = false;

    // clear wordgeom
    for (int i = (int)(this->wordgeom.size())-1; i >= 0; i--) {
        delete this->wordgeom.at(i);
        this->wordgeom.erase(this->wordgeom.end()-1);
    }

    // is text set?
    if (text=="")
        // no text, all is done
        return true;

    // get width of a blank character
    this->font->getStringWidth(" ", -1, &blankWidth);

    // through the text and extract single words
    int text_pos = 0;
    do {
    	// index relative to text_pos where the next line feed is found
        size_t lfindex;

    	// index relative to text_pos where the next blank is found
        size_t index;

        // searching for next line feed
        if ((lfindex = text.find('\n', text_pos)) != string::npos) lfindex-= text_pos;

        if (wrap) {
            // wrap mode, find next blank
        	if ((index = text.find(' ', text_pos)) != string::npos) index-= text_pos;
        }
        else {
        	// no wrap mode, no need to find blanks
        	index = string::npos;
        }

        if (lfindex == string::npos) {
        	// no line feed found
            if (index == string::npos) {
            	// no blank found, so we have found the very last word in the text string
            	index = text.size() - text_pos;
            }
            else
            if (index == 0) {
            	// another blank found instead of a word
            	// so we use all the blanks up to the next word as "empty word"
				if ((index = text.find_first_not_of(' ', text_pos)) != string::npos) index-= text_pos;
            	index--;
	            if (index == string::npos) {
	            	// the end of the string consists of blanks only
	            	index = text.size() - text_pos;
	            }
            }
        }
        else {
        	// line feed found
            if ((index == string::npos)||(index > lfindex)) {
            	// no blank before line feed, so we have found the last word in the line
            	// the length of the word is equal to the lfindex
                index = lfindex;
            }
            else {
            	// blank found before next line feed
                if (index == 0) {
                	// another blank found instead of a word
                	// so we use all the blanks up to the next word as "empty word"
					if ((index = text.find_first_not_of(' ', text_pos)) != string::npos) index-= text_pos;
					index--;
                }
                lfindex = string::npos;
            }
        }

        if (*lines == 0) {
            // first word
            *lines = 1;
            x = 0;
            y = 0;
        }

        // new word
        TEXTBOX_WORDGEOM *mywordgeom = new TEXTBOX_WORDGEOM;
        mywordgeom->geom.h = fontHeight;
        mywordgeom->word   = text.substr(text_pos, index);

        // get the width of the string
        this->font->getStringWidth(mywordgeom->word, -1, &mywordgeom->geom.w);

        if (x > 0)
            x += blankWidth;

        unsigned int endpos = x + mywordgeom->geom.w;

        bool gotonext = true;

        if ((wrap)&&(splitwords)) {
            // split words in wrap mode
            if ((index != string::npos)&&(mywordgeom->geom.w > (int)*realWidth)) {
                // recalculate index
                while ((index > 1)&&(mywordgeom->geom.w > (int)*realWidth)) {
                    index--;
                    mywordgeom->word = text.substr(text_pos, index);
                    this->font->getStringWidth(mywordgeom->word, -1, &mywordgeom->geom.w);
                    endpos = x + mywordgeom->geom.w;
                }

                // move the pos ahead
                text_pos+= index;
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

        if ((lfindex != string::npos)||(gotonext==false)) {
            x = 0;
            y+= fontHeight;
            (*lines)++;
            if (lfindex != string::npos) (*paragraphs)++;
        }

        // add to list
        wordgeom.push_back(mywordgeom);

        if (gotonext) {
            if (index + 1 < text.size() - text_pos) {
                // move the pos ahead
            	text_pos+= index + 1;
            }
            else {
            	// the end reached
            	text_pos = (int)text.size();
            }
        }
    } while ((int)text.size() - text_pos > 0);

    // go through the list and calculate horizontal text alignment
    unsigned int oldline = 1;
    unsigned int oldpos = 0;
    for (unsigned int i = 0; i < wordgeom.size(); i++) {
        if (wordgeom.at(i)->line != oldline) {
            if   ((alignment == MMSALIGNMENT_CENTER)||(alignment == MMSALIGNMENT_TOP_CENTER)||(alignment == MMSALIGNMENT_BOTTOM_CENTER)) {
            	// horizontal centered
                unsigned int diff = (*realWidth - wordgeom.at(i-1)->geom.x - wordgeom.at(i-1)->geom.w) / 2;
                for (unsigned int j = oldpos; j < i; j++)
                    wordgeom.at(j)->geom.x += diff;
            }
            else
            if   ((alignment == MMSALIGNMENT_RIGHT)||(alignment == MMSALIGNMENT_TOP_RIGHT)
                ||(alignment == MMSALIGNMENT_BOTTOM_RIGHT)) {
            	// right aligned
                unsigned int diff = *realWidth - wordgeom.at(i-1)->geom.x - wordgeom.at(i-1)->geom.w;
                for (unsigned int j = oldpos; j < i; j++)
                    wordgeom.at(j)->geom.x += diff;
            }
            else
            if  (((alignment == MMSALIGNMENT_JUSTIFY)||(alignment == MMSALIGNMENT_TOP_JUSTIFY)
                ||(alignment == MMSALIGNMENT_BOTTOM_JUSTIFY))&&(wordgeom.at(i)->paragraph == wordgeom.at(i-1)->paragraph)) {
            	// justified
                if (oldpos < i-1) {
                    unsigned int diff = ((*realWidth - wordgeom.at(i-1)->geom.x - wordgeom.at(i-1)->geom.w)*10) / (i-1-oldpos);
                    for (unsigned int j = oldpos + 1; j < i-1; j++) {
                        wordgeom.at(j)->geom.x += ((j - oldpos) * diff) / 10;
                    }
                    if (oldpos < i-1) {
                    	// at least two words in the line, set the last word exactly to the right side
                    	wordgeom.at(i-1)->geom.x = *realWidth - wordgeom.at(i-1)->geom.w;
                    }
                }
            }
            oldpos = i;
            oldline = wordgeom.at(i)->line;
        }
    }
    if   ((alignment == MMSALIGNMENT_CENTER)||(alignment == MMSALIGNMENT_TOP_CENTER)||(alignment == MMSALIGNMENT_BOTTOM_CENTER)) {
    	// horizontal centered
        unsigned int diff = (*realWidth - wordgeom.at(wordgeom.size()-1)->geom.x - wordgeom.at(wordgeom.size()-1)->geom.w) / 2;
        for (unsigned int j = oldpos; j < wordgeom.size(); j++)
            wordgeom.at(j)->geom.x += diff;
    }
    else
    if   ((alignment == MMSALIGNMENT_RIGHT)||(alignment == MMSALIGNMENT_TOP_RIGHT)
        ||(alignment == MMSALIGNMENT_BOTTOM_RIGHT)) {
    	// right aligned
        unsigned int diff = *realWidth - wordgeom.at(wordgeom.size()-1)->geom.x - wordgeom.at(wordgeom.size()-1)->geom.w;
        for (unsigned int j = oldpos; j < wordgeom.size(); j++)
            wordgeom.at(j)->geom.x += diff;
    }

    // go through the list and calculate vertical text alignment
    if (fontHeight * (*lines) > *realHeight)
        *realHeight = fontHeight * (*lines);
    else
    if (fontHeight * (*lines) < *realHeight) {
        if   ((alignment == MMSALIGNMENT_CENTER)||(alignment == MMSALIGNMENT_LEFT)
            ||(alignment == MMSALIGNMENT_RIGHT)||(alignment == MMSALIGNMENT_JUSTIFY)) {
        	// vertical centered
            unsigned int diff = (*realHeight - fontHeight * (*lines)) / 2;
            if (diff > 0)
                for (unsigned int i = 0; i < wordgeom.size(); i++)
                    wordgeom.at(i)->geom.y += diff;
        }
        else
        if   ((alignment == MMSALIGNMENT_BOTTOM_CENTER)||(alignment == MMSALIGNMENT_BOTTOM_LEFT)
            ||(alignment == MMSALIGNMENT_BOTTOM_RIGHT)||(alignment == MMSALIGNMENT_BOTTOM_JUSTIFY)) {
        	// bottom aligned
            unsigned int diff = (*realHeight - fontHeight * (*lines));
            if (diff > 0)
                for (unsigned int i = 0; i < wordgeom.size(); i++)
                    wordgeom.at(i)->geom.y += diff;
        }
    }

    return true;
}


bool MMSTextBoxWidget::init() {
    // init widget basics
    if (!MMSWidget::init())
        return false;

    // load font
    this->font = this->rootwindow->fm->getFont(getFontPath(), getFontName(), getFontSize());

    // load file
    this->loadFile(false);

    return true;
}

bool MMSTextBoxWidget::release() {
    // release widget basics
    if (!MMSWidget::release())
        return false;

    // release my font
    this->rootwindow->fm->releaseFont(this->font);
    this->font = NULL;

    return true;
}

bool MMSTextBoxWidget::draw(bool *backgroundFilled) {
    bool myBackgroundFilled = false;

    if (!this->initialized) {
        // init widget (e.g. load images, fonts, ...)
        init();
        this->initialized = true;
    }

    if (backgroundFilled) {
    	if (this->has_own_surface)
    		*backgroundFilled = false;
    }
    else
        backgroundFilled = &myBackgroundFilled;

    // calculate text and surface size
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
            // text has changed, reset something
        	setScrollSize(scrollDX, scrollDY);
          	setSurfaceGeometry(realWidth, realHeight);
        }
    }

    // draw widget basics
    if (MMSWidget::draw(backgroundFilled)) {

        // lock
        this->surface->lock();

        // draw my things
        if (this->font) {
        	MMSFBRectangle surfaceGeom = getSurfaceGeometry();

            MMSFBColor color;

            if (isSelected())
                color = getSelColor();
            else
                color = getColor();

            if (color.a) {
                // set the font
                this->surface->setFont(this->font);

                // prepare for drawing
                this->surface->setDrawingColorAndFlagsByBrightnessAndOpacity(color, getBrightness(), getOpacity());

                // draw single words into surface
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

        // unlock
        this->surface->unlock();

        // update window surface with an area of surface
        updateWindowSurfaceWithSurface(!*backgroundFilled);
    }

    // draw widgets debug frame
    return MMSWidget::drawDebug();
}

void MMSTextBoxWidget::targetLangChanged(int lang) {
    this->translated = false;
}

bool MMSTextBoxWidget::loadFile(bool refresh) {
	if (this->file) {
		// free the "old" file
		delete this->file;
		this->file = NULL;
	}

	// create new file instance
	this->file = new MMSFile(getFilePath() + "/" + getFileName());
	if (!this->file)
		return false;
	if (this->file->getLastError())
		return false;

	// read the file
	void *ptr;
	size_t ritems;
	if (!this->file->readBufferEx(&ptr, &ritems)) {
		delete this->file;
		this->file = NULL;
		return false;
	}

	// free old text to save memory
	setText("", false);

	// fill the text
	string text;
	text.insert(0, (const char *)ptr, ritems);
	setText(&text, refresh);

	// free buffer
	free(ptr);

	return true;
}

bool MMSTextBoxWidget::reloadFile() {
	return loadFile(true);
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

string MMSTextBoxWidget::getFilePath() {
    GETTEXTBOX(FilePath);
}

string MMSTextBoxWidget::getFileName() {
    GETTEXTBOX(FileName);
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

void MMSTextBoxWidget::setText(string *text, bool refresh) {
    myTextBoxWidgetClass.setText(text);
    this->translated = false;
    this->da->scrollPosX=0;
    this->da->scrollPosY=0;
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setText(string text, bool refresh) {
	setText(&text, refresh);
}

void MMSTextBoxWidget::setTranslate(bool translate, bool refresh) {
    myTextBoxWidgetClass.setTranslate(translate);
    this->translated = false;
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setFilePath(string filepath, bool load, bool refresh) {
    myTextBoxWidgetClass.setFilePath(filepath);
    if (load)
    	loadFile(false);
    if (refresh)
        this->refresh();
}

void MMSTextBoxWidget::setFileName(string filename, bool load, bool refresh) {
    myTextBoxWidgetClass.setFileName(filename);
    if (load)
    	loadFile(false);
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
    if (themeClass->isFilePath())
        setFilePath(themeClass->getFilePath());
    if (themeClass->isFileName())
        setFileName(themeClass->getFileName());

    MMSWidget::updateFromThemeClass(&(themeClass->widgetClass));
}

/***********************************************/
/* end of theme access methods                 */
/***********************************************/
