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

#include "mmsgui/mmsdialogmanager.h"
#include "mmsgui/mmswindows.h"
#include "mmsgui/mmswidgets.h"
#include "mmsgui/theme/mmsthememanager.h"
#include <string.h>

MMSDialogManager::MMSDialogManager() {
	this->rootWindow = NULL;
	this->rootWindow_is_mine = true;
}

MMSDialogManager::MMSDialogManager(MMSWindow *rootWindow) {
	this->rootWindow = rootWindow;
	this->rootWindow_is_mine = (!this->rootWindow);
}

MMSDialogManager::~MMSDialogManager() {
	if (this->rootWindow_is_mine) {
		if (rootWindow)
			delete rootWindow;
	}
	else {
		// i should not delete the rootwindow because it was not initialized by me
		// so delete only the loaded child windows
		for (unsigned int i = 0; i < childWins.size(); i++)
			delete childWins.at(i);
	}
}

bool MMSDialogManager::isLoaded() {
	return (rootWindow)?true:false;
}

void MMSDialogManager::insertNamedWidget(MMSWidget *widget) {
    namedWidgets.push_back(widget);
}


MMSWidget* MMSDialogManager::searchForWidget(string name) {
    if (this->rootWindow)
        return this->rootWindow->searchForWidget(name);
    else
        return NULL;
}

MMSWidget* MMSDialogManager::operator[](string name) {
    MMSWidget *widget;

    if ((widget = searchForWidget(name)))
        return widget;
    throw new MMSDialogManagerError(1, "widget " + name + " not found");
}



MMSWindow* MMSDialogManager::loadDialog(string filename, MMSTheme *theme) {

	/* get taff file name */
    string tafffilename = filename + ".taff";

    //check for file
    if(!file_exist(filename))
        if(!file_exist(tafffilename))
        	throw new MMSDialogManagerError(1, "dialog file (" + filename + ") not found");

    /* open the taff file */
	MMSTaffFile *tafff = new MMSTaffFile(tafffilename, &mmsgui_taff_description,
										 filename, MMSTAFF_EXTERNAL_TYPE_XML);

	if (!tafff)
        throw new MMSDialogManagerError(1, "could not load dialog file " + filename);

	if (!tafff->isLoaded()) {
		delete tafff;
        throw new MMSDialogManagerError(1, "could not load dialog file " + filename);
	}

	/* get root tag */
	int tagid = tafff->getFirstTag();
	if (tagid < 0) {
		delete tafff;
        throw new MMSDialogManagerError(1, "invalid taff file " + tafffilename);
	}

	/* check if the correct tag */
	if (tagid != MMSGUI_TAGTABLE_TAG_MMSDIALOG) {
    	DEBUGMSG("MMSGUI", "no valid dialog file: %s", filename.c_str());
        return NULL;
    }

	/* through the doc */
    this->throughDoc(tafff, NULL, this->rootWindow, theme);

    /* free the document */
	delete tafff;

	return rootWindow;
}

MMSChildWindow* MMSDialogManager::loadChildDialog(string filename, MMSTheme *theme) {

	unsigned int cw_size = childWins.size();

	/* get taff file name */
    string tafffilename = filename + ".taff";

    //check for file
    if(!file_exist(filename))
        if(!file_exist(tafffilename))
        	throw new MMSDialogManagerError(1, "dialog file (" + filename + ") not found");

    /* open the taff file */
	MMSTaffFile *tafff = new MMSTaffFile(tafffilename, &mmsgui_taff_description,
										 filename, MMSTAFF_EXTERNAL_TYPE_XML);

	if (!tafff)
        throw new MMSDialogManagerError(1, "could not load dialog file " + filename);

	if (!tafff->isLoaded()) {
		delete tafff;
        throw new MMSDialogManagerError(1, "could not load dialog file " + filename);
	}

	/* get root tag */
	int tagid = tafff->getFirstTag();
	if (tagid < 0) {
		delete tafff;
        throw new MMSDialogManagerError(1, "invalid taff file " + tafffilename);
	}

	/* check if the correct tag */
	if (tagid != MMSGUI_TAGTABLE_TAG_MMSDIALOG) {
    	DEBUGMSG("MMSGUI", "no valid dialog file: %s", filename.c_str());
        return NULL;
    }

	/* through the doc */
//printf("loadChildDialog(), root=%x, file=%s\n", this->rootWindow, filename.c_str());
    this->throughDoc(tafff, NULL, this->rootWindow, theme);

    /* free the document */
	delete tafff;

    if (cw_size < childWins.size())
        return childWins.at(cw_size);
    else
        return NULL;
}

MMSDescriptionClass MMSDialogManager::getDescription() {
    return this->description;
}


void MMSDialogManager::throughDoc(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow,
							      MMSTheme *theme, bool only_first_child) {
    vector<string> widgetNames;
    string widgetName;
    bool loop = true;

    /* iterate through childs */
	while (loop) {
		if (only_first_child) loop = false;

		bool eof;
		int tid = tafff->getNextTag(eof);
		if (eof) break;
		if (tid < 0) break;

		switch (tid) {
		case MMSGUI_TAGTABLE_TAG_DESCRIPTION:
			getDescriptionValues(tafff, theme);
			/* get close tag of description */
			tafff->getNextTag(eof);
			break;
		case MMSGUI_TAGTABLE_TAG_MAINWINDOW:
            getMainWindowValues(tafff, theme);
            break;
		case MMSGUI_TAGTABLE_TAG_POPUPWINDOW:
            getPopupWindowValues(tafff, theme);
            break;
		case MMSGUI_TAGTABLE_TAG_ROOTWINDOW:
            getRootWindowValues(tafff, theme);
            break;
		case MMSGUI_TAGTABLE_TAG_CHILDWINDOW:
            getChildWindowValues(tafff, rootWindow, theme);
            break;
		default: {
                widgetName="";
        		switch (tid) {
        		case MMSGUI_TAGTABLE_TAG_TEMPLATE:
                    widgetName = getTemplateValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_VBOXWIDGET:
                    widgetName = getVBoxValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_HBOXWIDGET:
        	        widgetName = getHBoxValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_LABELWIDGET:
        	        widgetName = getLabelValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_BUTTONWIDGET:
        	        widgetName = getButtonValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_IMAGEWIDGET:
        	        widgetName = getImageValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_PROGRESSBARWIDGET:
        	        widgetName = getProgressBarValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_MENUWIDGET:
                    widgetName = getMenuValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_TEXTBOXWIDGET:
                    widgetName = getTextBoxValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_ARROWWIDGET:
                    widgetName = getArrowValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_SLIDERWIDGET:
                    widgetName = getSliderValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_INPUTWIDGET:
        	        widgetName = getInputValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_CHECKBOXWIDGET:
                    widgetName = getCheckBoxValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		case MMSGUI_TAGTABLE_TAG_GAPWIDGET:
                    widgetName = getGapValues(tafff, currentWidget, rootWindow, theme);
                    break;
        		}

                if(widgetName != "") {
                    /* search for duplicate names for the same parent */
                    for (unsigned int i = 0; i < widgetNames.size(); i++)
                        if (widgetNames.at(i) == widgetName)
                            throw new MMSDialogManagerError(1, "duplicate widget name: " + widgetName);
                    widgetNames.push_back(widgetName);
                }

            }
			break;
		}
	}
}

void MMSDialogManager::getDescriptionValues(MMSTaffFile *tafff, MMSTheme *theme) {

    this->description.setAttributesFromTAFF(tafff);
}

void MMSDialogManager::getMainWindowValues(MMSTaffFile *tafff, MMSTheme *theme) {
    MMSMainWindowClass themeClass;
    string             name = "", dx = "", dy = "", width = "", height = "";

    if(this->rootWindow)
        throw new MMSDialogManagerError(1, "found nested windows, new mainwindow rejected");

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    themeClass.windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.windowClass.setAttributesFromTAFF(tafff, &themePath);
    themeClass.setAttributesFromTAFF(tafff, &themePath);

    if (themeClass.windowClass.getDx(dx))
        if (getPixelFromSizeHint(NULL, dx, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window dx '" + dx + "'");

    if (themeClass.windowClass.getDy(dy))
        if (getPixelFromSizeHint(NULL, dy, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window dy '" + dy + "'");

    if (themeClass.windowClass.getWidth(width))
        if (getPixelFromSizeHint(NULL, width, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window width '" + width + "'");

    if (themeClass.windowClass.getHeight(height))
        if (getPixelFromSizeHint(NULL, height, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window height '" + height + "'");

    bool os;
    bool *osp = NULL;
    if (themeClass.windowClass.getOwnSurface(os))
    	osp = &os;

    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
        }
    }
    endTAFFScan

    MMSALIGNMENT alignment;
    if (!themeClass.windowClass.getAlignment(alignment))
    	alignment = MMSALIGNMENT_NOTSET;

    if ((themeClass.windowClass.isDx())||(themeClass.windowClass.isDy()))
        this->rootWindow = new MMSMainWindow(themeClass.getClassName(),
                                             dx,
                                             dy,
                                             width,
                                             height,
                                             alignment,
                                             MMSW_NONE,
                                             theme,
                                             osp);
    else
        this->rootWindow = new MMSMainWindow(themeClass.getClassName(),
                                             width,
                                             height,
                                             alignment,
                                             MMSW_NONE,
                                             theme,
                                             osp);

    this->rootWindow->setName(name);
    ((MMSMainWindow*)this->rootWindow)->updateFromThemeClass(&themeClass);

    throughDoc(tafff, NULL, this->rootWindow, theme);
}


void MMSDialogManager::getPopupWindowValues(MMSTaffFile *tafff, MMSTheme *theme) {
    MMSPopupWindowClass themeClass;
    string              name = "", dx = "", dy = "", width = "", height = "";

    if(this->rootWindow)
        throw new MMSDialogManagerError(1, "found nested windows, new popupwindow rejected");

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    themeClass.windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.windowClass.setAttributesFromTAFF(tafff, &themePath);
    themeClass.setAttributesFromTAFF(tafff, &themePath);

    if (themeClass.windowClass.getDx(dx))
        if (getPixelFromSizeHint(NULL, dx, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window dx '" + dx + "'");

    if (themeClass.windowClass.getDy(dy))
        if (getPixelFromSizeHint(NULL, dy, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window dy '" + dy + "'");

    if (themeClass.windowClass.getWidth(width))
        if (getPixelFromSizeHint(NULL, width, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window width '" + width + "'");

    if (themeClass.windowClass.getHeight(height))
        if (getPixelFromSizeHint(NULL, height, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window height '" + height + "'");

    bool os;
    bool *osp = NULL;
    if (themeClass.windowClass.getOwnSurface(os))
    	osp = &os;

    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
        }
    }
    endTAFFScan

    MMSALIGNMENT alignment;
    if (!themeClass.windowClass.getAlignment(alignment))
    	alignment = MMSALIGNMENT_NOTSET;

    if ((themeClass.windowClass.isDx())||(themeClass.windowClass.isDy()))
        this->rootWindow = new MMSPopupWindow(themeClass.getClassName(),
							                  dx,
								              dy,
								              width,
								              height,
								              alignment,
                                              MMSW_NONE,
                                              theme,
                                              osp,
                                              0);
    else
        this->rootWindow = new MMSPopupWindow(themeClass.getClassName(),
								              width,
								              height,
								              alignment,
                                              MMSW_NONE,
                                              theme,
                                              osp,
                                              0);

    this->rootWindow->setName(name);
    ((MMSPopupWindow*)this->rootWindow)->updateFromThemeClass(&themeClass);

    throughDoc(tafff, NULL, this->rootWindow, theme);
}

void MMSDialogManager::getRootWindowValues(MMSTaffFile *tafff, MMSTheme *theme) {
    MMSRootWindowClass themeClass;
    string             name = "", dx = "", dy = "", width = "", height = "";

    if(this->rootWindow)
        throw new MMSDialogManagerError(1, "found nested windows, new rootwindow rejected");

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    themeClass.windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.windowClass.setAttributesFromTAFF(tafff, &themePath);
    themeClass.setAttributesFromTAFF(tafff, &themePath);

    if (themeClass.windowClass.getDx(dx))
        if (getPixelFromSizeHint(NULL, dx, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window dx '" + dx + "'");

    if (themeClass.windowClass.getDy(dy))
        if (getPixelFromSizeHint(NULL, dy, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window dy '" + dy + "'");

    if (themeClass.windowClass.getWidth(width))
        if (getPixelFromSizeHint(NULL, width, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window width '" + width + "'");

    if (themeClass.windowClass.getHeight(height))
        if (getPixelFromSizeHint(NULL, height, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window height '" + height + "'");

    bool os;
    bool *osp = NULL;
    if (themeClass.windowClass.getOwnSurface(os))
    	osp = &os;

    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
        }
    }
    endTAFFScan

    MMSALIGNMENT alignment;
    if (!themeClass.windowClass.getAlignment(alignment))
    	alignment = MMSALIGNMENT_NOTSET;

    if ((themeClass.windowClass.isDx())||(themeClass.windowClass.isDy()))
        this->rootWindow = new MMSRootWindow(themeClass.getClassName(),
								             dx,
								             dy,
								             width,
								             height,
								             alignment,
                                             MMSW_NONE,
                                             theme,
                                             osp);
    else
        this->rootWindow = new MMSRootWindow(themeClass.getClassName(),
								             width,
								             height,
								             alignment,
                                             MMSW_NONE,
                                             theme,
                                             osp);

    this->rootWindow->setName(name);
    ((MMSRootWindow*)this->rootWindow)->updateFromThemeClass(&themeClass);

    throughDoc(tafff, NULL, this->rootWindow, theme);
}

void MMSDialogManager::getChildWindowValues(MMSTaffFile *tafff, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSChildWindowClass themeClass;
    MMSChildWindow      *childwin;
    string              name = "", dx = "", dy = "", width = "", height = "";
    bool                show = false;

    if(!rootWindow)
        throw new MMSDialogManagerError(1, "no parent window found, new childwindow rejected");

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    themeClass.windowClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.windowClass.setAttributesFromTAFF(tafff, &themePath);
    themeClass.setAttributesFromTAFF(tafff, &themePath);

    if (themeClass.windowClass.getDx(dx))
        if (getPixelFromSizeHint(NULL, dx, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window dx '" + dx + "'");

    if (themeClass.windowClass.getDy(dy))
        if (getPixelFromSizeHint(NULL, dy, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window dy '" + dy + "'");

    if (themeClass.windowClass.getWidth(width))
        if (getPixelFromSizeHint(NULL, width, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window width '" + width + "'");

    if (themeClass.windowClass.getHeight(height))
        if (getPixelFromSizeHint(NULL, height, 10000, 0) == false)
            throw new MMSDialogManagerError(1, "invalid window height '" + height + "'");

    bool os;
    bool *osp = NULL;
    if (themeClass.windowClass.getOwnSurface(os))
    	osp = &os;

    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_show:
            show = (attrval_int) ? true : false;
			break;
	    }
    }
    endTAFFScan

    MMSALIGNMENT alignment;
    if (!themeClass.windowClass.getAlignment(alignment))
    	alignment = MMSALIGNMENT_NOTSET;

    if ((themeClass.windowClass.isDx())||(themeClass.windowClass.isDy()))
        childwin = new MMSChildWindow(themeClass.getClassName(),
                                      rootWindow,
                                      dx,
                                      dy,
                                      width,
                                      height,
                                      alignment,
                                      MMSW_NONE,
                                      theme,
                                      osp);
    else
        childwin = new MMSChildWindow(themeClass.getClassName(),
                                      rootWindow,
                                      width,
                                      height,
                                      alignment,
                                      MMSW_NONE,
                                      theme,
                                      osp);

    // store only the 'root' child window in my list
    if (this->rootWindow == rootWindow)
    	childWins.push_back(childwin);

    childwin->setName(name);
    childwin->updateFromThemeClass(&themeClass);

    throughDoc(tafff, NULL, childwin, theme);

    if (show) childwin->show();
}

string MMSDialogManager::getTemplateValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSTemplateClass    themeClass, *templateClass;
    MMSHBoxWidget       *hbox;
    string              name = "";
    MMSTaffFile        	*tf;
    vector<string>      widgetNames;

    /* read settings from dialog */
    themeClass.setAttributesFromTAFF(tafff);

    /* is a class name given? */
    if (themeClass.getClassName()=="")
        return "";

    /* can load templateClass? */
    if (theme) {
        if (!(templateClass = theme->getTemplateClass(themeClass.getClassName())))
            return "";
    }
    else {
        if (!(templateClass = globalTheme->getTemplateClass(themeClass.getClassName())))
            return "";
    }

    /* are there any childs stored in the templateClass? */
    if (!(tf = templateClass->getTAFF()))
        return "";

    /* search for attributes which are only supported within dialog */
    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
        }
    }
    endTAFFScan
    startTAFFScan_WITHOUT_ID
    {
        if (memcmp(attrname, "widget.", 7)==0) {
            /* search for attributes which are to be set for templates child widgets */
            string widgetName = &attrname[7];
            int pos = (int)widgetName.find(".");
            if (pos > 0) {
                widgetName = widgetName.substr(0, pos);

                /* store the requested widget name here */
				bool found = false;
				for (unsigned int i = 0; i < widgetNames.size(); i++)
					if (widgetNames.at(i)==widgetName) {
						found = true;
						break;
					}
				if (!found)
					widgetNames.push_back(widgetName);
            }
        }
    }
    endTAFFScan_WITHOUT_ID

    /* create new hbox as container for the template */
    hbox = new MMSHBoxWidget(rootWindow);

    /* add to widget vector if named */
    if(name != "") {
        hbox->setName(name);
        insertNamedWidget(hbox);
    }

    if (currentWidget)
        currentWidget->add(hbox);
    else
        rootWindow->add(hbox);

    /* yes, parse the childs from templateClass */
    throughDoc(tf, hbox, rootWindow, theme);

    /* for each child widget which is named by attribute */
    for (unsigned int i = 0; i < widgetNames.size(); i++) {
        MMSWidget *mywidget = hbox->searchForWidget(widgetNames.at(i));
        if (mywidget) {
            /* widget found */
            string prefix = "widget." + widgetNames.at(i) + ".";
            switch (mywidget->getType()) {
                case MMSWIDGETTYPE_HBOX:
                    break;
                case MMSWIDGETTYPE_VBOX:
                    break;
                case MMSWIDGETTYPE_BUTTON:
                    {
                        /* read attributes from dialog */
                        MMSButtonWidgetClass themeCls;
                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.setAttributesFromTAFF(tafff, &prefix);
                        /* apply settings from dialog */
                        ((MMSButtonWidget*)mywidget)->updateFromThemeClass(&themeCls);
                    }
                    break;
                case MMSWIDGETTYPE_IMAGE:
                    {
                        /* read attributes from dialog */
                        MMSImageWidgetClass themeCls;
                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.setAttributesFromTAFF(tafff, &prefix);
                        /* apply settings from dialog */
                        ((MMSImageWidget*)mywidget)->updateFromThemeClass(&themeCls);
                    }
                    break;
                case MMSWIDGETTYPE_LABEL:
                    {
                        /* read attributes from dialog */
                        MMSLabelWidgetClass themeCls;
                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.setAttributesFromTAFF(tafff, &prefix);
                        /* apply settings from dialog */
                        ((MMSLabelWidget*)mywidget)->updateFromThemeClass(&themeCls);
                    }
                    break;
                case MMSWIDGETTYPE_MENU:
                    {
                        /* read attributes from dialog */
                        MMSMenuWidgetClass themeCls;
                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.setAttributesFromTAFF(tafff, &prefix);
                        /* apply settings from dialog */
                        ((MMSMenuWidget*)mywidget)->updateFromThemeClass(&themeCls);
                    }
                    break;
                case MMSWIDGETTYPE_PROGRESSBAR:
                    {
                        /* read attributes from dialog */
                        MMSProgressBarWidgetClass themeCls;
                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.setAttributesFromTAFF(tafff, &prefix);
                        /* apply settings from dialog */
                        ((MMSProgressBarWidget*)mywidget)->updateFromThemeClass(&themeCls);
                    }
                    break;
                case MMSWIDGETTYPE_TEXTBOX:
                    {
                        /* read attributes from dialog */
                        MMSTextBoxWidgetClass themeCls;
                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.setAttributesFromTAFF(tafff, &prefix);
                        /* apply settings from dialog */
                        ((MMSTextBoxWidget*)mywidget)->updateFromThemeClass(&themeCls);
                    }
                    break;
                case MMSWIDGETTYPE_ARROW:
                    {
                        /* read attributes from dialog */
                        MMSArrowWidgetClass themeCls;
                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.setAttributesFromTAFF(tafff, &prefix);
                        /* apply settings from dialog */
                        ((MMSArrowWidget*)mywidget)->updateFromThemeClass(&themeCls);
                    }
                    break;
                case MMSWIDGETTYPE_SLIDER:
                    {
                        /* read attributes from dialog */
                        MMSSliderWidgetClass themeCls;
                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.setAttributesFromTAFF(tafff, &prefix);
                        /* apply settings from dialog */
                        ((MMSSliderWidget*)mywidget)->updateFromThemeClass(&themeCls);
                    }
                    break;
                case MMSWIDGETTYPE_INPUT:
                    {
                        /* read attributes from dialog */
                        MMSInputWidgetClass themeCls;
                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.setAttributesFromTAFF(tafff, &prefix);
                        /* apply settings from dialog */
                        ((MMSInputWidget*)mywidget)->updateFromThemeClass(&themeCls);
                    }
                    break;
                case MMSWIDGETTYPE_CHECKBOX:
                    {
                        /* read attributes from dialog */
                        MMSCheckBoxWidgetClass themeCls;
                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
                        themeCls.setAttributesFromTAFF(tafff, &prefix);
                        /* apply settings from dialog */
                        ((MMSCheckBoxWidget*)mywidget)->updateFromThemeClass(&themeCls);
                    }
                    break;
                case MMSWIDGETTYPE_GAP:
                    break;
            }
        }
    }

    /* return the name of the widget */
    return name;
}


string MMSDialogManager::getVBoxValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSVBoxWidget *vbox;
    string  name = "";
    string  size = "";

    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

	vbox = new MMSVBoxWidget(rootWindow);

    /* add to widget vector if named */
    if(name != "") {
        vbox->setName(name);
        insertNamedWidget(vbox);
    }

	if(size != "")
	    vbox->setSizeHint(size);

	if (currentWidget)
        currentWidget->add(vbox);
    else
        rootWindow->add(vbox);

	throughDoc(tafff, vbox, rootWindow, theme);

    /* return the name of the widget */
    return name;
}

string MMSDialogManager::getHBoxValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSHBoxWidget *hbox;
    string  name = "";
    string  size = "";

    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

	hbox = new MMSHBoxWidget(rootWindow);

    /* add to widget vector if named */
    if(name != "") {
        hbox->setName(name);
        insertNamedWidget(hbox);
    }

	if(size != "")
	    hbox->setSizeHint(size);

    if (currentWidget)
        currentWidget->add(hbox);
    else
        rootWindow->add(hbox);

	throughDoc(tafff, hbox, rootWindow, theme);

    /* return the name of the widget */
    return name;
}


string MMSDialogManager::getLabelValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSLabelWidgetClass   themeClass;
    MMSLabelWidget  *label;
    string          name = "";
    string          size = "";

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    /* read settings from dialog */
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    /* create new label from theme class */
    label = new MMSLabelWidget(rootWindow, themeClass.getClassName(), theme);

    /* apply settings from dialog */
    label->updateFromThemeClass(&themeClass);

    /* search for attributes which are only supported within dialog */
    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

    /* add to widget vector if named */
    if(name != "") {
        label->setName(name);
        insertNamedWidget(label);
    }

    if(size != "") {
        if (!label->setSizeHint(size))
            throw new MMSDialogManagerError(1, "invalid widget size '" + size + "'");
    }

    if (currentWidget)
        currentWidget->add(label);
    else
        rootWindow->add(label);

    throughDoc(tafff, label, rootWindow, theme);

    /* return the name of the widget */
    return name;
}



string MMSDialogManager::getButtonValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSButtonWidgetClass  themeClass;
    MMSButtonWidget *button;
    string          name = "";
    string          size = "";

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    /* read settings from dialog */
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff,  NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    /* create new button from theme class */
    button = new MMSButtonWidget(rootWindow, themeClass.getClassName(), theme);

    /* apply settings from dialog */
    button->updateFromThemeClass(&themeClass);

    /* search for attributes which are only supported within dialog */
    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

    /* add to widget vector if named */
    if(name != "") {
        button->setName(name);
        insertNamedWidget(button);
    }

    if(size != "") {
        if (!button->setSizeHint(size))
            throw new MMSDialogManagerError(1, "invalid widget size '" + size + "'");
    }

    if (currentWidget)
        currentWidget->add(button);
    else
        rootWindow->add(button);

    throughDoc(tafff, button, rootWindow, theme);

    /* return the name of the widget */
    return name;
}

string MMSDialogManager::getImageValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSImageWidgetClass   themeClass;
    MMSImageWidget  *image;
    string          name = "";
    string          size = "";

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    /* read settings from dialog */
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    /* create new image from theme class */
    image = new MMSImageWidget(rootWindow, themeClass.getClassName(), theme);

    /* apply settings from dialog */
    image->updateFromThemeClass(&themeClass);

    /* search for attributes which are only supported within dialog */
    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

    /* add to widget vector if named */
    if(name != "") {
        image->setName(name);
        insertNamedWidget(image);
    }

    if(size != "") {
        if (!image->setSizeHint(size))
            throw new MMSDialogManagerError(1, "invalid widget size '" + size + "'");
    }

    if (currentWidget)
        currentWidget->add(image);
    else
        rootWindow->add(image);

    throughDoc(tafff, image, rootWindow, theme);

    /* return the name of the widget */
    return name;
}


string MMSDialogManager::getProgressBarValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSProgressBarWidgetClass 	themeClass;
    MMSProgressBarWidget	*pBar;
    string              	name = "";
    string              	size = "";

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    /* read settings from dialog */
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    /* create new progressbar from theme class */
    pBar = new MMSProgressBarWidget(rootWindow, themeClass.getClassName(), theme);

    /* apply settings from dialog */
    pBar->updateFromThemeClass(&themeClass);

    /* search for attributes which are only supported within dialog */
    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

    /* add to widget vector if named */
    if(name != "") {
        pBar->setName(name);
        insertNamedWidget(pBar);
    }

    if(size != "") {
        if (!pBar->setSizeHint(size))
            throw new MMSDialogManagerError(1, "invalid widget size '" + size + "'");
    }

    if (currentWidget)
        currentWidget->add(pBar);
    else
        rootWindow->add(pBar);

    throughDoc(tafff, pBar, rootWindow, theme);

    /* return the name of the widget */
    return name;
}


string MMSDialogManager::getMenuValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSMenuWidgetClass    themeClass;
    MMSMenuWidget   *menu;
    string          name = "";
    string          size = "";
    MMSTaffFile    	*tf;

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    /* read settings from dialog */
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    /* create new menu from theme class */
    menu = new MMSMenuWidget(rootWindow, themeClass.getClassName(), theme);

    /* apply settings from dialog */
    menu->updateFromThemeClass(&themeClass);

    /* search for attributes which are only supported within dialog */
    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

    /* add to widget vector if named */
    if(name != "") {
        menu->setName(name);
        insertNamedWidget(menu);
    }

    if(size != "") {
        if (!menu->setSizeHint(size))
            throw new MMSDialogManagerError(1, "invalid widget size '" + size + "'");
    }

    if (currentWidget)
        currentWidget->add(menu);
    else
        rootWindow->add(menu);

    /* set the item layout, we need a temporary parent widget */
    MMSHBoxWidget *tmpWidget = new MMSHBoxWidget(NULL);

    /* are there any childs stored in the theme? */
    if ((tf = menu->getTAFF())) {
        /* yes, parse the childs from theme */
        throughDoc(tf, tmpWidget, NULL, theme);
    }
    else {
        /* no, parse the childs from dialog file */
    	throughDoc(tafff, tmpWidget, NULL, theme, true);
    }

    bool haveItemTemplate = false;
    MMSWidget *itemTemplate = tmpWidget->disconnectChild();
    if (itemTemplate) {
        menu->setItemTemplate(itemTemplate);
        haveItemTemplate = true;
    }
    else {
        if (tf) {
            /* try with theme failed, retry with childs from dialog file */
            throughDoc(tafff, tmpWidget, NULL, theme);
            MMSWidget *itemTemplate = tmpWidget->disconnectChild();
            if (itemTemplate) {
                menu->setItemTemplate(itemTemplate);
                haveItemTemplate = true;
            }
        }
    }

    delete tmpWidget;

    if (haveItemTemplate) {
        /* have a template - search for menu items which are set in the dialog file */
        bool haveItems = false;
        bool returntag = true;

        /* iterate through childs */
    	while (1) {
    		bool eof;
    		int tid = tafff->getNextTag(eof);
    		if (eof) break;
    		if (tid < 0) {
    			if (returntag) break;
    			returntag = true;
    			continue;
    		}
    		else
    			returntag = false;

    		/* check if a <menuitem> is given */
            if (tid == MMSGUI_TAGTABLE_TAG_MENUITEM)
            {
	            /* create new menu item */
	            MMSWidget *topwidget = menu->newItem();
	            haveItems = true;

	            /* here we must loop for n widgets */
	            vector<string> wgs;
	            bool wg_break = false;
	            while (!wg_break) {
	            	wg_break = true;
		            startTAFFScan_WITHOUT_ID
		            {
		            	if (memcmp(attrname, "widget.", 7)==0) {
		                    /* search for attributes which are to be set for menu items child widgets */
		                    string widgetName = &attrname[7];
		                    int pos = (int)widgetName.find(".");
		                    if (pos > 0) {
		                        /* widget name found */
		                        widgetName = widgetName.substr(0, pos);

		                        /* check if i have already processed this widget */
		                        for (unsigned int w = 0; w < wgs.size(); w++)
		                        	if (wgs.at(w)==widgetName) {
		                        		widgetName = "";
		                        		break;
		                        	}
		                        if (widgetName == "")
		                        	continue;
		                        wg_break = false;
		                        wgs.push_back(widgetName);

		                        /* okay, searching for the widget within the new item */
		                        MMSWidget *widget;
		                        if (topwidget->getName() == widgetName)
		                            widget = topwidget;
		                        else
		                            widget = topwidget->searchForWidget(widgetName);

		                        if (widget) {
		                            /* widget found */
		                            /* add attribute to widget */
		                            string prefix = "widget." + widgetName + ".";
		                            switch (widget->getType()) {
		                                case MMSWIDGETTYPE_HBOX:
		                                    break;
		                                case MMSWIDGETTYPE_VBOX:
		                                    break;
		                                case MMSWIDGETTYPE_BUTTON:
		                                    {
		                                        /* read attributes from node */
		                                        MMSButtonWidgetClass themeCls;
		                                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.setAttributesFromTAFF(tafff, &prefix);
		                                        /* apply settings from node */
		                                        ((MMSButtonWidget*)widget)->updateFromThemeClass(&themeCls);
		                                    }
		                                    break;
		                                case MMSWIDGETTYPE_IMAGE:
		                                    {
		                                        /* read attributes from node */
		                                        MMSImageWidgetClass themeCls;
		                                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.setAttributesFromTAFF(tafff, &prefix);
		                                        /* apply settings from node */
		                                        ((MMSImageWidget*)widget)->updateFromThemeClass(&themeCls);
		                                    }
		                                    break;
		                                case MMSWIDGETTYPE_LABEL:
		                                    {
		                                        /* read attributes from node */
		                                        MMSLabelWidgetClass themeCls;
		                                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.setAttributesFromTAFF(tafff, &prefix);
		                                        /* apply settings from node */
		                                        ((MMSLabelWidget*)widget)->updateFromThemeClass(&themeCls);
		                                    }
		                                    break;
		                                case MMSWIDGETTYPE_MENU:
		                                    break;
		                                case MMSWIDGETTYPE_PROGRESSBAR:
		                                    {
		                                        /* read attributes from node */
		                                        MMSProgressBarWidgetClass themeCls;
		                                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.setAttributesFromTAFF(tafff, &prefix);
		                                        /* apply settings from node */
		                                        ((MMSProgressBarWidget*)widget)->updateFromThemeClass(&themeCls);
		                                    }
		                                    break;
		                                case MMSWIDGETTYPE_TEXTBOX:
		                                    {
		                                        /* read attributes from node */
		                                        MMSTextBoxWidgetClass themeCls;
		                                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.setAttributesFromTAFF(tafff, &prefix);
		                                        /* apply settings from node */
		                                        ((MMSTextBoxWidget*)widget)->updateFromThemeClass(&themeCls);
		                                    }
		                                    break;
		                                case MMSWIDGETTYPE_ARROW:
		                                    {
		                                        /* read attributes from node */
		                                        MMSArrowWidgetClass themeCls;
		                                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.setAttributesFromTAFF(tafff, &prefix);
		                                        /* apply settings from node */
		                                        ((MMSArrowWidget*)widget)->updateFromThemeClass(&themeCls);
		                                    }
		                                    break;
		                                case MMSWIDGETTYPE_SLIDER:
		                                    {
		                                        /* read attributes from node */
		                                        MMSSliderWidgetClass themeCls;
		                                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.setAttributesFromTAFF(tafff, &prefix);
		                                        /* apply settings from node */
		                                        ((MMSSliderWidget*)widget)->updateFromThemeClass(&themeCls);
		                                    }
		                                    break;
		                                case MMSWIDGETTYPE_INPUT:
		                                    break;
		                                case MMSWIDGETTYPE_CHECKBOX:
		                                    {
		                                        /* read attributes from node */
		                                        MMSCheckBoxWidgetClass themeCls;
		                                        themeCls.widgetClass.border.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.widgetClass.setAttributesFromTAFF(tafff, &prefix);
		                                        themeCls.setAttributesFromTAFF(tafff, &prefix);
		                                        /* apply settings from node */
		                                        ((MMSCheckBoxWidget*)widget)->updateFromThemeClass(&themeCls);
		                                    }
		                                    break;
		                                case MMSWIDGETTYPE_GAP:
		                                    break;
		                            }
		                        }
		                    }
		                }
		            }
		            endTAFFScan_WITHOUT_ID
	            }

	            startTAFFScan_WITHOUT_ID
	            {
	            	if (memcmp(attrname, "childwindow", 11)==0) {
	            		// there is a child window given which represents a sub menu
	            		menu->setSubMenuName(menu->getSize()-1, attrval_str);
	            	}
	            	else
	            	if (memcmp(attrname, "goback", 6)==0) {
	            		// if true, this item should be the go-back-item
	                    //! if the user enters this item, the parent menu (if does exist) will be shown
	            		if (memcmp(attrval_str, "true", 4)==0)
	            			menu->setBackItem(menu->getSize()-1);
	            	}
	            }
	            endTAFFScan_WITHOUT_ID

	            startTAFFScan
	            {
	                switch (attrid) {
	        		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
	        			if (*attrval_str)
	        				topwidget->setName(attrval_str);
	        			break;
	        	    }
	            }
	            endTAFFScan
            }
            else {
            	// any other widgets given in the menu, we need a temporary widget
                MMSHBoxWidget *tmpWidget = new MMSHBoxWidget(NULL);

                // parse the childs from dialog file
            	throughDoc(tafff, tmpWidget, NULL, theme);

            	if (tid == MMSGUI_TAGTABLE_TAG_SEPARATOR) {
                    // set the separator
	            	if (menu->getSize() > 0) {
		                MMSWidget *separator = tmpWidget->disconnectChild();
		                if (separator) {
		                	// separator widget(s) set
		                	separator->setFocusable(false);
		                	menu->setSeparator(menu->getSize()-1, separator);
		                }
	            	}
            	}
                delete tmpWidget;
            }
    	}

        if (haveItems)
            menu->setFocus(false, false);
    }

    /* return the name of the widget */
    return name;
}


string MMSDialogManager::getTextBoxValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSTextBoxWidgetClass 	themeClass;
    MMSTextBoxWidget	*textbox;
    string          	name = "";
    string          	size = "";

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    /* read settings from dialog */
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    /* create new textbox from theme class */
    textbox = new MMSTextBoxWidget(rootWindow, themeClass.getClassName(), theme);

    /* apply settings from dialog */
    textbox->updateFromThemeClass(&themeClass);

    /* search for attributes which are only supported within dialog */
    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

    /* add to widget vector if named */
    if(name != "") {
        textbox->setName(name);
        insertNamedWidget(textbox);
    }

    if(size != "") {
        if (!textbox->setSizeHint(size))
            throw new MMSDialogManagerError(1, "invalid widget size '" + size + "'");
    }

    if (currentWidget)
        currentWidget->add(textbox);
    else
        rootWindow->add(textbox);

    throughDoc(tafff, textbox, rootWindow, theme);

    /* return the name of the widget */
    return name;
}

string MMSDialogManager::getArrowValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSArrowWidgetClass   themeClass;
    MMSArrowWidget  *arrow;
    string          name = "";
    string          size = "";

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    /* read settings from dialog */
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    /* create new arrow from theme class */
    arrow = new MMSArrowWidget(rootWindow, themeClass.getClassName(), theme);

    /* apply settings from dialog */
    arrow->updateFromThemeClass(&themeClass);

    /* search for attributes which are only supported within dialog */
    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

    /* add to widget vector if named */
    if(name != "") {
        arrow->setName(name);
        insertNamedWidget(arrow);
    }

    if(size != "") {
        if (!arrow->setSizeHint(size))
            throw new MMSDialogManagerError(1, "invalid widget size '" + size + "'");
    }

    if (currentWidget)
        currentWidget->add(arrow);
    else
        rootWindow->add(arrow);

    throughDoc(tafff, arrow, rootWindow, theme);

    /* return the name of the widget */
    return name;
}

string MMSDialogManager::getSliderValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSSliderWidgetClass  themeClass;
    MMSSliderWidget *slider;
    string          name = "";
    string          size = "";

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    /* read settings from dialog */
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    /* create new slider from theme class */
    slider = new MMSSliderWidget(rootWindow, themeClass.getClassName(), theme);

    /* apply settings from dialog */
    slider->updateFromThemeClass(&themeClass);

    /* search for attributes which are only supported within dialog */
    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

    /* add to widget vector if named */
    if(name != "") {
        slider->setName(name);
        insertNamedWidget(slider);
    }

    if(size != "") {
        if (!slider->setSizeHint(size))
            throw new MMSDialogManagerError(1, "invalid widget size '" + size + "'");
    }

    if (currentWidget)
        currentWidget->add(slider);
    else
        rootWindow->add(slider);

    throughDoc(tafff, slider, rootWindow, theme);

    /* return the name of the widget */
    return name;
}

string MMSDialogManager::getInputValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSInputWidgetClass   themeClass;
    MMSInputWidget  *input;
    string          name = "";
    string          size = "";

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    /* read settings from dialog */
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    /* create new label from theme class */
    input = new MMSInputWidget(rootWindow, themeClass.getClassName(), theme);

    /* apply settings from dialog */
    input->updateFromThemeClass(&themeClass);

    /* search for attributes which are only supported within dialog */
    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

    /* add to widget vector if named */
    if(name != "") {
        input->setName(name);
        insertNamedWidget(input);
    }

    if(size != "") {
        if (!input->setSizeHint(size))
            throw new MMSDialogManagerError(1, "invalid widget size '" + size + "'");
    }

    if (currentWidget)
        currentWidget->add(input);
    else
        rootWindow->add(input);

    throughDoc(tafff, input, rootWindow, theme);

    /* return the name of the widget */
    return name;
}


string MMSDialogManager::getCheckBoxValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSCheckBoxWidgetClass 	themeClass;
    MMSCheckBoxWidget		*checkbox;
    string          		name = "";
    string          		size = "";

    /* get themepath */
    string themePath = "";
    if (theme)
        themePath = theme->getThemePath();
    else
        themePath = globalTheme->getThemePath();

    /* read settings from dialog */
    themeClass.widgetClass.border.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.widgetClass.setAttributesFromTAFF(tafff, NULL, &themePath);
    themeClass.setAttributesFromTAFF(tafff, NULL, &themePath);

    /* create new checkbox from theme class */
    checkbox = new MMSCheckBoxWidget(rootWindow, themeClass.getClassName(), theme);

    /* apply settings from dialog */
    checkbox->updateFromThemeClass(&themeClass);

    /* search for attributes which are only supported within dialog */
    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

    /* add to widget vector if named */
    if(name != "") {
        checkbox->setName(name);
        insertNamedWidget(checkbox);
    }

    if(size != "") {
        if (!checkbox->setSizeHint(size))
            throw new MMSDialogManagerError(1, "invalid widget size '" + size + "'");
    }

    if (currentWidget)
        currentWidget->add(checkbox);
    else
        rootWindow->add(checkbox);

    throughDoc(tafff, checkbox, rootWindow, theme);

    /* return the name of the widget */
    return name;
}

string MMSDialogManager::getGapValues(MMSTaffFile *tafff, MMSWidget *currentWidget, MMSWindow *rootWindow, MMSTheme *theme) {
    MMSGapWidget *gap;
    string  name = "";
    string  size = "";

    startTAFFScan
    {
        switch (attrid) {
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_name:
            name = attrval_str;
			break;
		case MMSGUI_BASE_ATTR::MMSGUI_BASE_ATTR_IDS_size:
	        size = attrval_str;
			break;
	    }
    }
    endTAFFScan

	gap = new MMSGapWidget(rootWindow);

    /* add to widget vector if named */
    if(name != "") {
        gap->setName(name);
        insertNamedWidget(gap);
    }

	if(size != "")
	    gap->setSizeHint(size);

	if (currentWidget)
        currentWidget->add(gap);
    else
        rootWindow->add(gap);

    throughDoc(tafff, gap, rootWindow, theme);

    /* return the name of the widget */
    return name;
}




