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

#ifndef MMSFILEDIALOG_H_
#define MMSFILEDIALOG_H_

#include "mmsgui/mmswindows.h"
#include "mmsgui/mmswidgets.h"
#include "mmsgui/mmsdialogmanager.h"

class MMSFileDialog {
    private:
    	string 				dialogfile;
    	MMSDialogManager	*dm;
    	MMSWindow			*dialogwindow;

    	MMSLabelWidget		*filedialog_title;

    	MMSWidget			*filedialog_ok;
    	MMSWidget			*filedialog_cancel;

    	MMSLabelWidget		*filedialog_path;
    	MMSInputWidget		*filedialog_name;

    	MMSMenuWidget		*filedialog_filelist;

    	MMSButtonWidget		*filedialog_up;
    	MMSButtonWidget		*filedialog_down;

    	string				path;
    	string				filename;

    	void onReturn(MMSWidget *widget);
    	void onSelectItem(MMSWidget *widget);

    	bool fillMenu();

    public:
        MMSFileDialog();
        MMSFileDialog(string path, string filename, MMSWindow *dialogwindow = NULL);
        ~MMSFileDialog();
        bool loadFileDialog(MMSWindow *parent, string dialogfile = "", MMSTheme *theme = NULL);
        bool isInitialized();

        bool setTitle(string title);

        bool show();

        sigc::signal<void, MMSFileDialog*> *onOK;
        sigc::signal<void> *onCancel;
};

#endif /*MMSFILEDIALOG_H_*/
