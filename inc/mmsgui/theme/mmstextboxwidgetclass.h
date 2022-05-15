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

#ifndef MMSTEXTBOXWIDGETCLASS_H_
#define MMSTEXTBOXWIDGETCLASS_H_

#include "mmsgui/theme/mmswidgetclass.h"

//! describe attributes for MMSTextBoxWidget which are additional to the MMSWidgetClass
namespace MMSGUI_TEXTBOXWIDGET_ATTR {

	#define MMSGUI_TEXTBOXWIDGET_ATTR_ATTRDESC \
		{ "font.path", TAFF_ATTRTYPE_STRING }, \
		{ "font.name", TAFF_ATTRTYPE_STRING }, \
		{ "font.size", TAFF_ATTRTYPE_UCHAR }, \
		{ "alignment", TAFF_ATTRTYPE_STRING }, \
		{ "wrap", TAFF_ATTRTYPE_BOOL }, \
		{ "splitwords", TAFF_ATTRTYPE_BOOL }, \
		{ "color", TAFF_ATTRTYPE_STRING }, \
		{ "color.a", TAFF_ATTRTYPE_UCHAR }, \
		{ "color.r", TAFF_ATTRTYPE_UCHAR }, \
		{ "color.g", TAFF_ATTRTYPE_UCHAR }, \
		{ "color.b", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor", TAFF_ATTRTYPE_STRING }, \
		{ "selcolor.a", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor.r", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor.g", TAFF_ATTRTYPE_UCHAR }, \
		{ "selcolor.b", TAFF_ATTRTYPE_UCHAR }, \
		{ "text", TAFF_ATTRTYPE_STRING }, \
		{ "translate", TAFF_ATTRTYPE_BOOL }

	#define MMSGUI_TEXTBOXWIDGET_ATTR_IDS \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_font_path, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_font_name, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_font_size, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_alignment, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_wrap, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_splitwords, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_color, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_color_a, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_color_r, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_color_g, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_color_b, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_selcolor, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_selcolor_a, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_selcolor_r, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_selcolor_g, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_selcolor_b, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_text, \
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS_translate

	#define MMSGUI_TEXTBOXWIDGET_ATTR_INIT { \
		MMSGUI_BASE_ATTR_ATTRDESC, \
		MMSGUI_BORDER_ATTR_ATTRDESC, \
		MMSGUI_WIDGET_ATTR_ATTRDESC, \
		MMSGUI_TEXTBOXWIDGET_ATTR_ATTRDESC, \
		{ NULL, TAFF_ATTRTYPE_NONE } \
	}

	typedef enum {
		MMSGUI_BASE_ATTR_IDS,
		MMSGUI_BORDER_ATTR_IDS,
		MMSGUI_WIDGET_ATTR_IDS,
		MMSGUI_TEXTBOXWIDGET_ATTR_IDS
	} ids;
}

extern TAFF_ATTRDESC MMSGUI_TEXTBOXWIDGET_ATTR_I[];


//! A data access class for the textbox widget.
/*!
This class is the base for the MMSTextBoxWidget class.
With this data store you have access to all changeable widget attributes.
It is also one of the base classes for MMSThemeManager and MMSDialogManager
which are main features of the MMSGUI.
\note This class will be internally used by class MMSTextBoxWidget.
\author Jens Schneider
*/
class MMSTextBoxWidgetClass {
    private:
    	//! name of the theme class
        string          className;

        //! is fontpath set?
        bool            isfontpath;

        //! path to the font
        string          fontpath;

        //! is fontname set?
        bool            isfontname;

        //! name of the font
        string          fontname;

        //! is fontsize set?
        bool            isfontsize;

        //! size of the font
        unsigned int    fontsize;

        //! is alignment set?
        bool            isalignment;

        //! alignment of the text
        MMSALIGNMENT    alignment;

        //! is wrap flag set?
        bool            iswrap;

        //! wrap (true/false) the text
        bool            wrap;

        //! is splitwords flag set?
        bool            issplitwords;

        //! splitwords (true/false)
        bool            splitwords;

        //! is color set?
        bool            iscolor;

        //! color of the text if the widget is not selected
        MMSFBColor        color;

        //! is selcolor set?
        bool            isselcolor;

        //! color of the text if the widget is selected
        MMSFBColor      selcolor;

        //! is text set?
        bool            istext;

        //! text to draw
        string          text;

        //! is translate set?
        bool            istranslate;

        //! if true the text will be translated before displayed
        bool          	translate;

        //! Read and set all attributes from the given TAFF buffer.
        /*!
        \param tafff   pointer to the TAFF buffer
        \param prefix  optional, prefix to all attribute names (<prefix><attrname>=<attrvalue>)
        \param path    optional, path needed for empty path values from the TAFF buffer
        */
        void setAttributesFromTAFF(MMSTaffFile *tafff, string *prefix = NULL, string *path = NULL);

    public:
    	//! stores base widget attributes
        MMSWidgetClass widgetClass;

        //! Constructor of class MMSTextBoxWidgetClass.
        MMSTextBoxWidgetClass();

        //! Mark all attributes as not set.
        void unsetAll();

        //! Set the name of the theme class.
        /*!
        \param classname  name of the class
        */
        void setClassName(string className);

        //! Get the name of the theme class.
        /*!
        \return name of the class
        */
        string getClassName();

        //! Check if the fontpath is set.
        bool isFontPath();

        //! Set the fontpath which is used to draw the text.
        /*!
        \param fontpath  path to the font
        */
        void setFontPath(string fontpath);

        //! Mark the fontpath as not set.
        void unsetFontPath();

        //! Get the fontpath which is used to draw the text.
        /*!
        \return path to the font
        */
        string getFontPath();

        //! Check if the fontname is set.
        bool isFontName();

        //! Set the fontname which is used to draw the text.
        /*!
        \param fontname  name of the font
        */
        void setFontName(string fontname);

        //! Mark the fontname as not set.
        void unsetFontName();

        //! Get the fontname which is used to draw the text.
        /*!
        \return name of the font
        */
        string getFontName();

        //! Check if the fontsize is set.
        bool isFontSize();

        //! Set the fontsize which is used to draw the text.
        /*!
        \param fontsize  size of the font
        */
        void setFontSize(unsigned int fontsize);

        //! Mark the fontsize as not set.
        void unsetFontSize();

        //! Get the fontsize which is used to draw the text.
        /*!
        \return size of the font
        */
        unsigned int getFontSize();

        //! Check if alignment is set.
        bool isAlignment();

        //! Set the alignment of the text (see MMSALIGNMENT values).
        /*!
        \param alignment  text alignment
        */
        void setAlignment(MMSALIGNMENT alignment);

        //! Mark the alignment as not set.
        void unsetAlignment();

        //! Get the alignment of the text (see MMSALIGNMENT values).
        /*!
        \return size of the font
        */
        MMSALIGNMENT getAlignment();

        //! Check if the wrap flag is set.
        bool isWrap();

        //! Set the wrap flag.
        /*!
        \param wrap  wrap the text if set to true
        */
        void setWrap(bool wrap);

        //! Mark the wrap flag as not set.
        void unsetWrap();

        //! Get the wrap flag.
        /*!
        \return wrap flag
        */
        bool getWrap();

        //! Check if splitwords is set.
        bool isSplitWords();

        //! Set the splitwords flag.
        /*!
        \param splitwords  split words at the end of a line
        \note This works only if the wrap flag is set.
        */
        void setSplitWords(bool splitwords);

        //! Mark splitwords as not set.
        void unsetSplitWords();

        //! Get the splitwords flag.
        /*!
        \return splitwords flag
        */
        bool getSplitWords();

        //! Check if the color is set. This color will be used for the unselected text.
        bool isColor();

        //! Set the color which is used to draw the unselected text.
        /*!
        \param color  color for unselected text
        */
        void setColor(MMSFBColor color);

        //! Mark the color as not set.
        void unsetColor();

        //! Get the color which is used to draw the unselected text.
        /*!
        \return color for unselected text
        */
        MMSFBColor getColor();

        //! Check if the color is set. This color will be used for the selected text.
        bool isSelColor();

        //! Set the color which is used to draw the selected text.
        /*!
        \param selcolor  color for selected text
        */
        void setSelColor(MMSFBColor selcolor);

        //! Mark the color as not set.
        void unsetSelColor();

        //! Get the color which is used to draw the selected text.
        /*!
        \return color for selected text
        */
        MMSFBColor getSelColor();

        //! Check if the text is set.
        bool isText();

        //! Set the text to be drawn.
        /*!
        \param text  any text string
        */
        void setText(string text);

        //! Mark the text as not set.
        void unsetText();

        //! Get the current text.
        /*!
        \return text string
        */
        string getText();

        //! Check if the translate flag is set.
        bool isTranslate();

        //! Set the translate flag.
        /*!
        \param translate  true/false
        */
        void setTranslate(bool translate);

        //! Mark the translate flag as not set.
        void unsetTranslate();

        //! Get the translate flag.
        /*!
        \return true/false
        */
        bool getTranslate();

    /* friends */
    friend class MMSThemeManager;
    friend class MMSDialogManager;
};

#endif /*MMSTEXTBOXWIDGETCLASS_H_*/
