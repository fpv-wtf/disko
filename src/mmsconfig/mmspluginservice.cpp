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

#include "mmsconfig/mmspluginservice.h"
#include "mmstools/interfaces/immsdb.h"
#include "mmsconfig/mmsplugindao.h"
#include "mmsconfig/mmspluginpropertydao.h"
#include "mmsconfig/mmsimportpropertydao.h"

MMSPluginService::MMSPluginService(DataSource *datasource) :
    dbconn(NULL) {
    MMSDBConnMgr connMgr(datasource);
    if((this->dbconn = connMgr.getConnection()))
        this->dbconn->connect();
}

MMSPluginService::~MMSPluginService() {
	if(this->dbconn) {
	    this->dbconn->disconnect();
	    delete this->dbconn;
	}
}

void MMSPluginService::setPlugin(MMSPluginData *data) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    myPluginDAO.saveOrUpdate(data);

    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
    myPropertyDAO.saveOrUpdate(data);
}

void MMSPluginService::setPlugin(vector<MMSPluginData *> dataList) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    myPluginDAO.saveOrUpdate(dataList);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
    myPropertyDAO.saveOrUpdate(dataList);
}

MMSPluginData *MMSPluginService::getPluginByName(string name) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);

    MMSPluginData *plugin = myPluginDAO.findPluginByName(name);
    if(plugin == NULL) {
        return NULL;
    }

    vector <MMSPropertyData *> properties;
    properties = myPropertyDAO.findAllPluginPropertiesByPlugin(plugin);
    plugin->setProperties(properties);

    return plugin;
}

MMSPluginData *MMSPluginService::getPluginByID(int id) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);

    MMSPluginData *plugin = myPluginDAO.findPluginByID(id);

    vector <MMSPropertyData *> properties;
    properties = myPropertyDAO.findAllPluginPropertiesByPlugin(plugin);
    plugin->setProperties(properties);

    return plugin;
}

vector<MMSPluginData*> MMSPluginService::getAllPlugins(const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);
	DEBUGMSG("PLUGINSERVICE", "create property dao");
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);

    DEBUGMSG("PLUGINSERVICE", "get all plugins");
    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPlugins(inactiveToo);

    DEBUGMSG("PLUGINSERVICE", "go through");
    for(unsigned i=0; i<pluginList.size(); i++) {
        vector <MMSPropertyData *> properties;
        properties = myPropertyDAO.findAllPluginPropertiesByPlugin(pluginList.at(i));
        pluginList.at(i)->setProperties(properties);
    }

    return pluginList;
}

vector<MMSPluginData *> MMSPluginService::getOSDPlugins(const bool inactiveToo) {

	DEBUGMSG("PLUGINSERVICE", "create dao");
    MMSPluginDAO myPluginDAO(this->dbconn);

    DEBUGMSG("PLUGINSERVICE", "create property dao");
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);

    DEBUGMSG("PLUGINSERVICE", "get all OSD plugins");
    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByType(PT_OSD_PLUGIN, inactiveToo);

    for(unsigned i=0; i<pluginList.size(); i++) {
        vector <MMSPropertyData *> properties;
        properties = myPropertyDAO.findAllPluginPropertiesByPlugin(pluginList.at(i));
        DEBUGMSG("PLUGINSERVICE", "get the properties of: %s (%d)", pluginList.at(i)->getName().c_str(), pluginList.at(i)->getId());
        pluginList.at(i)->setProperties(properties);
    }

    DEBUGMSG("PLUGINSERVICE", "Working with %d OSD plugins", pluginList.size());

    return pluginList;
}

vector<MMSPluginData *> MMSPluginService::getCentralPlugins(const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);

    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByType(PT_CENTRAL_PLUGIN, inactiveToo);

    DEBUGMSG("PLUGINSERVICE", "get the properties of the central plugins.");
    for(unsigned i=0; i<pluginList.size(); i++) {
        vector <MMSPropertyData *> properties;
        properties = myPropertyDAO.findAllPluginPropertiesByPlugin(pluginList.at(i));
        pluginList.at(i)->setProperties(properties);
    }

    return pluginList;
}

vector<MMSPluginData *> MMSPluginService::getImportPlugins(const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);

    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByType(PT_IMPORT_PLUGIN, inactiveToo);

    DEBUGMSG("PLUGINSERVICE", "get the properties of the import plugins.");
    for(unsigned i=0; i<pluginList.size(); i++) {
        vector <MMSPropertyData *> properties;
        properties = myPropertyDAO.findAllPluginPropertiesByPlugin(pluginList.at(i));
        pluginList.at(i)->setProperties(properties);
    }

    return pluginList;
}

vector<MMSPluginData *> MMSPluginService::getBackendPlugins(const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);

    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByType(PT_BACKEND_PLUGIN, inactiveToo);

    for(unsigned i=0; i<pluginList.size(); i++) {
        vector <MMSPropertyData *> properties;
        properties = myPropertyDAO.findAllPluginPropertiesByPlugin(pluginList.at(i));
        pluginList.at(i)->setProperties(properties);
    }

    return pluginList;
}

/* getAllPluginsByCategory */
vector<MMSPluginData*> MMSPluginService::getPluginsByCategory(MMSPluginCategoryData *category, const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
    MMSImportPropertyDAO myImportPropertyDAO(this->dbconn);
    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByCategory(category, inactiveToo);

    for(unsigned i=0; i<pluginList.size(); i++) {
        if (pluginList.at(i)->getType()->getName() != PT_IMPORT_PLUGIN) {

            /* select the plugin properties*/
            vector <MMSPropertyData *> properties;
            properties = myPropertyDAO.findAllPluginPropertiesByPlugin(pluginList.at(i));
            pluginList.at(i)->setProperties(properties);
        }
        else {
            /* select the plugin import properties*/
            MMSImportPropertyData *i_properties;
            i_properties = myImportPropertyDAO.findImportPropertyByPlugin(pluginList.at(i));
            pluginList.at(i)->setImportProperties(i_properties);
        }
    }

    return pluginList;
}
vector<MMSPluginData*> MMSPluginService::getPluginsByType(MMSPluginTypeData *type, const bool inactiveToo) {
    MMSPluginDAO myPluginDAO(this->dbconn);
    MMSPluginPropertyDAO myPropertyDAO(this->dbconn);
    MMSImportPropertyDAO myImportPropertyDAO(this->dbconn);
    vector <MMSPluginData *> pluginList = myPluginDAO.findAllPluginsByType(type, inactiveToo);

    for(unsigned i=0; i<pluginList.size(); i++) {
        if (pluginList.at(i)->getType()->getName() != PT_IMPORT_PLUGIN) {

            /* select the plugin properties*/
            vector <MMSPropertyData *> properties;
            properties = myPropertyDAO.findAllPluginPropertiesByPlugin(pluginList.at(i));
            pluginList.at(i)->setProperties(properties);
        }
        else {
            /* select the plugin import properties*/
            MMSImportPropertyData *i_properties;
            i_properties = myImportPropertyDAO.findImportPropertyByPlugin(pluginList.at(i));
            pluginList.at(i)->setImportProperties(i_properties);
        }
    }

    return pluginList;
}

MMSPluginCategoryData* MMSPluginService::getPluginCategoryByName(string name) {
    MMSPluginCategoryDAO categoryDAO(this->dbconn);
    return categoryDAO.findCategoryByName(name);
}

MMSPluginTypeData* MMSPluginService::getPluginTypeByName(string name) {
	MMSPluginTypeDAO typeDAO(this->dbconn);
    return typeDAO.findTypeByName(name);
}

vector<MMSPluginCategoryData*> MMSPluginService::getPluginCategories() {
    MMSPluginCategoryDAO categoryDAO(this->dbconn);
    return categoryDAO.findAllCategories();
}
